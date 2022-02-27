// import init, { run_simulation } from "../../rust_simulator/assets";
import init, { run_simulation } from "../../rust_simulator/pkg/sim_lib.js";
// ..//rust_simulator/pkg/rust_simulator.js


// ..//rust_simulator/pkg/rust_simulator.js

onmessage = function (e) {
    let p = e.data;
    console.log(e.data.postcode);


    //submit_simulation(p.postcode, p.latitude, p.longitude, p.occupants, p['floor-area'], p.temperature, p['epc-space-heating'], p['tes-volume']);
    submit_simulation("CV4 7AL", 52.3833, -1.5833, 2, 60.0, 20.0, 3000.0, 0.5);
}

function build_file_path(latitude, longitude) {
    return `lat_${(Math.round(latitude * 2.0) / 2.0).toFixed(1)}_lon_${(Math.round(longitude * 2.0) / 2.0).toFixed(1)}.csv`;
}

async function read_array(filepath) {
    const resp = await fetch(filepath);
    const text = await resp.text();
    return text.split(/\r?\n/).map(Number);
}

async function submit_simulation(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max) {
    try {
        await init();
        console.log(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max);
        const ASSETS_DIR = "../../rust_simulator/assets/";
        const agile_tariff_file_path = ASSETS_DIR + "agile_tariff.csv";
        const outside_temps_file_path = ASSETS_DIR + "outside_temps/" + build_file_path(latitude, longitude);
        const solar_irradiances_file_path = ASSETS_DIR + "solar_irradiances/" + build_file_path(latitude, longitude);
        console.log(agile_tariff_file_path);
        console.log(outside_temps_file_path);
        console.log(solar_irradiances_file_path);
        const agile_tariff = await read_array(agile_tariff_file_path);
        const outside_temps = await read_array(outside_temps_file_path);
        const solar_irradiances = await read_array(solar_irradiances_file_path);
        const result = run_simulation(thermostat_temperature, latitude, longitude, num_occupants,
            house_size, postcode, epc_space_heating, tes_volume_max, agile_tariff, outside_temps, solar_irradiances);
        console.log(result);
        postMessage(result);
    } catch (error) {
        console.error('Rust-Sim-Error: ', error);
        postMessage(undefined);
    }
}

console.log('js');


// onmessage = function (e) {
//     console.log("MESSAGE");
//     let p = e.data;
//     console.log(e.data.postcode);
//     submit_simulation("CV4 7AL", 52.3833, -1.5833, 2, 60.0, 20.0, 3000.0, 0.5);
//     // let result = submit_simulation(p.postcode, p.latitude, p.longitude, p.occupants, p.floor_area, p.temperature, p.space_heating, p.tes_max);
// }

// function build_file_path(latitude, longitude) {
//     return `lat_${(Math.round(latitude * 2.0) / 2.0).toFixed(1)}_lon_${(Math.round(longitude * 2.0) / 2.0).toFixed(1)}.csv`;
// }

// async function read_array(filepath) {
//     const resp = await fetch(filepath);
//     const text = await resp.text();
//     return text.split(/\r?\n/).map(Number);
// }

// async function submit_simulation(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max) {
//     await init();
//     console.log(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max);
//     const ASSETS_DIR = "../../rust_simulator/assets/";
//     const agile_tariff_file_path = ASSETS_DIR + "agile_tariff.csv";
//     const outside_temps_file_path = ASSETS_DIR + "outside_temps/" + build_file_path(latitude, longitude);
//     const solar_irradiances_file_path = ASSETS_DIR + "solar_irradiances/" + build_file_path(latitude, longitude);
//     console.log(agile_tariff_file_path);
//     console.log(outside_temps_file_path);
//     console.log(solar_irradiances_file_path);
//     const agile_tariff = await read_array(agile_tariff_file_path);
//     const outside_temps = await read_array(outside_temps_file_path);
//     const solar_irradiances = await read_array(solar_irradiances_file_path);
//     const result = run_simulation(thermostat_temperature, latitude, longitude, num_occupants,
//         house_size, postcode, epc_space_heating, tes_volume_max, agile_tariff, outside_temps, solar_irradiances);
//     console.log(result);
//     postMessage(result);
// }

// console.log('js');
