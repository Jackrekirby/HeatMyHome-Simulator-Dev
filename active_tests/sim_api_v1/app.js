import fetch from 'node-fetch';
import express from 'express';
import cors from 'cors';
import fs from 'fs';
import { run_simulation } from "./pkg/rust_simulator.js";
// npm init
// npm i cheerio
// npm i node-fetch
// npm i express
// npm i cors
// node app.js
// "type": "module",

const app = express();
app.use(cors());

app.get('/', async (req, res) => {
    //console.log(req, res);
    const postcode = req.query.postcode;
    let latitude = Number(req.query.latitude);
    let longitude = Number(req.query.longitude);
    console.log('postcode: ', postcode, latitude);
    if (postcode != undefined) {
        if (isNaN(latitude)) { latitude = 52.3833; };
        if (isNaN(longitude)) { longitude = -1.5833; };
        //const t0 = performance.now();

        //const result = '[1, 2, 3, 4]';
        const result = await submit_simulation(postcode, latitude, longitude, 2, 60, 20, 3000, 0.5);
        //const t1 = performance.now();
        //console.log(`Time: ${t1 - t0} milliseconds.`);
        res.send({ 'result': JSON.parse(result), 'inputs': { 'postcode': postcode, 'latitude': latitude, 'longitude': longitude } });
        //res.send('T4');
    }
    else {
        res.send('Simulator API: 0');
    }


    //console.log('DONE2');
    //res.send(JSON.stringify(result));
})

function build_file_path(latitude, longitude, datatype) {
    return `lat_${(Math.round(latitude * 2.0) / 2.0).toFixed(1)}_lon_${(Math.round(longitude * 2.0) / 2.0).toFixed(1)}.csv`;
}

async function read_array(filepath) {
    const resp = await fetch(filepath);
    const text = await resp.text();
    return text.split(/\r?\n/).map(Number);
}

async function submit_simulation(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max) {
    console.log(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max);
    const ASSETS_DIR = "./assets/";
    const agile_tariff_file_path = ASSETS_DIR + "agile_tariff.csv";
    const outside_temps_file_path = ASSETS_DIR + "outside_temps/" + build_file_path(latitude, longitude);
    const solar_irradiances_file_path = ASSETS_DIR + "solar_irradiances/" + build_file_path(latitude, longitude);
    console.log(agile_tariff_file_path);
    console.log(outside_temps_file_path);
    console.log(solar_irradiances_file_path);
    const agile_tariff = fs.readFileSync(agile_tariff_file_path, { encoding: 'utf8', flag: 'r' }).split(/\r?\n/).map(Number);
    const outside_temps = fs.readFileSync(outside_temps_file_path, { encoding: 'utf8', flag: 'r' }).split(/\r?\n/).map(Number);
    const solar_irradiances = fs.readFileSync(solar_irradiances_file_path, { encoding: 'utf8', flag: 'r' }).split(/\r?\n/).map(Number);
    const result = run_simulation(thermostat_temperature, latitude, longitude, num_occupants,
        house_size, postcode, epc_space_heating, tes_volume_max, agile_tariff, outside_temps, solar_irradiances);
    //const result = '[1, 2, 3, 4]';
    return result;
}

const port = process.env.PORT || 3000;
app.listen(port, () =>
    console.log('EPC API listening on port: ', port),
);

