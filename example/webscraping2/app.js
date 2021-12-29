// const http = require('http')
// const fs = require('fs')
// const rp = require("request-promise");
// const cheerio = require("cheerio");
//const fetch = require('node-fetch');

import fetch from 'node-fetch';
import cheerio from 'cheerio'

async function getAddresses(postcode) {
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

async function getDataFromAddress(url) {
    const response = await fetch(url);
    console.log('url: ', url);
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
}

function archive() {
    rp(url)
        .then(function (html) {
            let $ = cheerio.load(html);
            const gov_partial_url = 'https://find-energy-certificate.service.gov.uk';
            var links = $("a.govuk-link");
            links.each(function (i, link) {
                let href = $(link).attr("href");
                if (href.startsWith('/energy-certificate')) {
                    let address = $(link).text().trim();
                    const full_link = `${src_url}${href}`;
                    address_link_list.push([address, full_link]);
                    //console.log(address, full_link);
                }
            });
        })
        .catch(function (err) {
            console.log(err);
        });

    console.log();
    let url2 = address_link_list[0][1];

    rp(url2)
        .then(function (html) {
            let $ = cheerio.load(html);
            var links = $("dl.govuk-summary-list");

            let floor_area_node = $("#main-content > div > div.govuk-grid-column-two-thirds.epc-domestic-sections > div.govuk-body.epc-blue-bottom.printable-area.epc-box-container > dl > div:nth-child(2) > dd")
            let floor_area_txt = floor_area_node.text().trim();
            console.log('Floor Area: ', floor_area_txt);

            let epc_space_heating_node = $("#main-content > div > div.govuk-grid-column-two-thirds.epc-domestic-sections > div.govuk-body.epc-blue-bottom.printable-area.epc-estimated-energy-use > dl:nth-child(10) > div:nth-child(1) > dd")
            let epc_space_heating_txt = epc_space_heating_node.text().trim();
            console.log('Space heating: ', epc_space_heating_txt);


            let valid_until_node = $("#main-content > div > div.govuk-grid-column-two-thirds.epc-domestic-sections > div.govuk-body.epc-blue-bottom.printable-area.epc-box-container > div > div.epc-extra-boxes > p:nth-child(1) > b");
            let valid_until_txt = valid_until_node.text().trim();
            console.log('Valid Until: ', valid_until_txt);
        })
        .catch(function (err) {
            console.log(err);
        });
}

import express from 'express';
import cors from 'cors';

const app = express();
app.use(cors());
app.options('*', cors());

app.get('/', async (req, res) => {
    //console.log(req, res);

    const postcode = req.query.postcode;
    console.log('postcode: ', postcode);
    const certificate = req.query.certificate;
    console.log('certificate: ', certificate);
    if (postcode) {
        const address_link_list = await getAddresses(postcode);
        //console.log(address_link_list);
        res.send(address_link_list);
    } else if (certificate) {
        const url_cert = 'https://find-energy-certificate.service.gov.uk/energy-certificate/' + certificate
        const data = await getDataFromAddress(url_cert);
        res.send(data);
    } else {
        res.send('No Request. ?postcode=CV57AA ?certificate=0774-2804-7261-2620-7941');
    }
})

async function test() {
    const response = await fetch("https://epc.opendatacommunities.org/api/v1/display/search?postcode=HP16+0LU", {
        method: "GET",
        headers: {
            Authorization: "Basic amFja3Jla2lyYnlAZ21haWwuY29tOmYyNDgzZDUxNGQ2NGUwZTQwN2U0ZWYxYzdjNjJiY2RkMmU3NzYyNWY=",
        },
    })
    const txt = await response.text();
    console.log(txt);
}

//test();

app.listen(3000, () =>
    console.log('Example app listening on port 3000!'),
);

// const server = http.createServer((req, res) => {
//     res.writeHead(200, { 'content-type': 'text/html' })
//     fs.createReadStream('index.html').pipe(res)
// })

// server.listen(process.env.PORT || 3000);