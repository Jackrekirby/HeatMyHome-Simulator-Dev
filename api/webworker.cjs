let fs = require('fs');

const SERVER = true;
let sim = undefined;
if (SERVER) {
    sim = require("./pkg/sim_lib.js");
} else {
    sim = require("../rust_simulator/pkg/sim_lib.js");
}

addEventListener('message', e => {
    let p = e.data;
    console.log(e.data.postcode);
    let result = submit_simulation(p.postcode, p.latitude, p.longitude, p.occupants, p.floor_area, p.temperature, p.space_heating, p.tes_max);
    postMessage(result);
});

function build_file_path(latitude, longitude) {
    return `lat_${(Math.round(latitude * 2.0) / 2.0).toFixed(1)}_lon_${(Math.round(longitude * 2.0) / 2.0).toFixed(1)}.csv`;
}

function submit_simulation(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max) {
    console.log(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max);
    let t0 = undefined;
    if (!SERVER) {
        t0 = performance.now();
    }
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
    const result = sim.run_simulation(thermostat_temperature, latitude, longitude, num_occupants,
        house_size, postcode, epc_space_heating, tes_volume_max, agile_tariff, outside_temps, solar_irradiances);
    if (!SERVER) {
        const t1 = performance.now();
        console.log(`Time: ${t1 - t0} milliseconds.`);
    }
    return result;
}