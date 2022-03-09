import fetch from 'node-fetch';
import express from 'express';
import cheerio from 'cheerio'
import cors from 'cors';
import Worker from 'web-worker';

const API_VERSION = 1.0;
// setup
// npm init
// npm i cheerio
// npm i node-fetch
// npm i express
// npm i cors
// npm i web-worker
// node app.js
// "type": "module",

const app = express();
// app.use(cors());

app.use(cors({
    origin: ['https://heatmyhome.ninja', 'https://jackrekirby.github.io']
}));

// ==============================================================================================================================
// SIMULATE API

app.get('/simulate', async (req, res) => {
    try {
        //console.log(req, res);
        const p = {
            'postcode': req.query.postcode,
            'latitude': req.query.latitude,
            'longitude': req.query.longitude,
            'occupants': req.query.occupants,
            'temperature': req.query.temperature,
            'space_heating': req.query.space_heating,
            'floor_area': req.query.floor_area,
            'tes_max': req.query.tes_max,
        };

        let undefined_parameter = false;
        for (const [parameter, value] of Object.entries(p)) {
            if (value == undefined) {
                undefined_parameter = true;
            }
            console.log(parameter, value);
        }

        if (!undefined_parameter) {
            console.log('parameters: ', p);

            if (isNaN(p.floor_area) || p.floor_area < 25 || p.floor_area > 1500) {
                res.send({
                    'status': 404,
                    'error': `The floor area is set to: ${p.floor_area}. This is either not a number, less than 25 m^2, or greater than 1500m^2`,
                    'inputs': p
                });
            } else if (isNaN(p.tes_max) || p.tes_max < 0.1 || p.tes_max > 3.0) {
                res.send({
                    'status': 404,
                    'error': `The tes-max is set to: ${p.tes_max}. This is either not a number, less than 0.1 m^3, or greater than 3.0m^3`,
                    'inputs': p
                });
            } else {
                let max_run_time = 30000;
                let sim_complete = false;

                const url = new URL('./webworker.cjs', import.meta.url);
                const worker = new Worker(url);

                worker.addEventListener('message', e => {
                    // res.send({ 'worker': e.data });
                    sim_complete = true;
                    res.send({ 'status': 200, 'inputs': p, 'result': JSON.parse(e.data) });
                    worker.terminate();
                    clearTimeout(simulation_timeout);
                    console.log("simulation complete");
                });

                let simulation_timeout = setTimeout(() => {
                    console.log("simulation timeout");
                    if (!sim_complete) {
                        worker.terminate();
                        res.send({
                            'status': 404,
                            'error': `simulation exceeded allowed runtime: ${max_run_time} ms. Server may be busy.`,
                            'inputs': p
                        });
                    }
                }, max_run_time);

                worker.postMessage(p);
            }
        } else {
            let url = req.get('host') + req.baseUrl + req.path;
            res.send({
                'status': 404,
                'error': `not all parameters defined. Example parameters: ${url}?postcode=CV47AL&latitude=52.3833&longitude=-1.5833&occupants=2&temperature=20&space_heating=3000&floor_area=60&tes_max=0.5`,
                'inputs': p
            });
        }
    } catch (error) {
        res.send({
            'status': 404,
            'error': `An unhandled error occured. ${error}`,
        });
    }
});

// =================================================================================================================================
// EPC API

app.get('/epc', async (req, res) => {
    try {
        //console.log(req, res);
        const postcode = req.query.postcode;
        console.log('postcode: ', postcode);
        const certificate = req.query.certificate;
        console.log('certificate: ', certificate);
        if (postcode) {
            const address_link_list = await get_addresses(postcode);
            //console.log(address_link_list);
            if (address_link_list.length === 0) {
                res.send({ 'status': 404, 'error': `postcode <${postcode}> is not valid` });
            } else {
                res.send({ 'status': 200, 'result': address_link_list });
            }
        } else if (certificate) {
            const url_cert = 'https://find-energy-certificate.service.gov.uk/energy-certificate/' + certificate
            const data = await get_data_from_certificate(url_cert);

            if (Object.keys(data).length === 0) {
                res.send({ 'status': 404, 'error': `certificate <${certificate}> is not valid` });
            } else {
                res.send({ 'status': 200, 'result': data });
            }
        } else {
            let url = req.get('host') + req.baseUrl + req.path;
            res.send({ 'status': 404, 'error': `must provide url parameter (either ${url}?postcode=CV47AL or ${url}?certificate=2808-3055-6321-9909-2974)` });
        }
    } catch (error) {
        res.send({
            'status': 404,
            'error': `An unhandled error occured. ${error}`,
        });
    }
});

async function get_addresses(postcode) {
    const url = `https://find-energy-certificate.service.gov.uk/find-a-certificate/search-by-postcode?postcode=${postcode.replace(' ', '+')}`;
    let address_link_list = [];
    const response = await fetch(url);
    console.log('url: ', url);
    const body = await response.text();
    let $ = cheerio.load(body);
    const gov_partial_url = 'https://find-energy-certificate.service.gov.uk';
    let links = $("a.govuk-link");
    links.each(function (i, link) {
        let href = $(link).attr("href");
        if (href.startsWith('/energy-certificate')) {
            let address = $(link).text().trim();
            const full_link = `${href.split('e/')[1]}`;
            address_link_list.push([address, full_link]);
            console.log(address, full_link);
        }
    });
    return address_link_list;
}

async function get_data_from_certificate(url) {
    const response = await fetch(url);
    console.log('url: ', url);
    if (response.ok) {
        const body = await response.text();
        let $ = cheerio.load(body);
        let links = $("dl.govuk-summary-list");

        let floor_area_node = $("#main-content > div > div.govuk-grid-column-two-thirds.epc-domestic-sections > div.govuk-body.epc-blue-bottom.printable-area.epc-box-container > dl > div:nth-child(2) > dd")
        let floor_area_txt = floor_area_node.text().trim();
        console.log('Floor Area: ', floor_area_txt);

        let epc_space_heating_node = $("#main-content > div > div.govuk-grid-column-two-thirds.epc-domestic-sections > div.govuk-body.epc-blue-bottom.printable-area.epc-estimated-energy-use > dl:nth-child(10) > div:nth-child(1) > dd")
        let epc_space_heating_txt = epc_space_heating_node.text().trim();
        console.log('Space heating: ', epc_space_heating_txt);

        let valid_until_node = $("#main-content > div > div.govuk-grid-column-two-thirds.epc-domestic-sections > div.govuk-body.epc-blue-bottom.printable-area.epc-box-container > div > div.epc-extra-boxes > p:nth-child(1) > b");
        let valid_until_txt = valid_until_node.text().trim();
        console.log('Valid Until: ', valid_until_txt);
        return { 'floor-area': floor_area_txt, 'space-heating': epc_space_heating_txt, 'valid-until': valid_until_txt };
    } else {
        return {};
    }
}

// ====================================================================================================================================
// OTHER

app.get('/', async (req, res) => {
    let url = req.get('host') + req.baseUrl + req.path;
    console.log();
    res.send({ 'status': 404, 'error': `must call ${url}epc or ${url}simulate with their respective url parameters`, 'info': `API Version ${API_VERSION}` });
});

const port = process.env.PORT || 3000;
app.listen(port, () =>
    console.log('EPC API listening on port: ', port),
);