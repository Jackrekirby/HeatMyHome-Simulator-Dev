import init, { run_simulation } from "./rust_simulator/pkg/rust_simulator.js";

(() => {
    console.log = function (...args) {
        args.forEach(arg => postMessage(["msg", `${JSON.stringify(arg)}\n`]));
    }
})();

onmessage = async function (e) {
    //console.log('Message received from main script. Message: ', e);
    let start = performance.now();
    if (e.data[0] == "run simulation") {
        const result = await submit_simulation(e.data[1], e.data[2], Number(e.data[3]), Number(e.data[4]),
            Number(e.data[5]), Number(e.data[6]), Number(e.data[7]), Number(e.data[8]), Number(e.data[9]));
        postMessage(["simulation complete", result, start]);
    } else if (e.data == "initiate") {
        await init();
        postMessage(["initiation complete"]);
    }
    else {
        console.warn('Message to worker is not linked to any event: ', e.data);
    }
}


function build_file_path(latitude, longitude, datatype) {
    return `lat_${(Math.round(latitude * 2.0) / 2.0).toFixed(1)}_lon_${(Math.round(longitude * 2.0) / 2.0).toFixed(1)}.csv`;
}

async function read_array(filepath) {
    const resp = await fetch(filepath);
    const text = await resp.text();
    return text.split(/\r?\n/).map(Number);
}

async function submit_simulation(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max) {
    //console.log(postcode, latitude, longitude, num_occupants, house_size, thermostat_temperature, epc_space_heating, tes_volume_max);
    const ASSETS_DIR = "./rust_simulator/assets/";
    const agile_tariff_file_path = ASSETS_DIR + "agile_tariff.csv";
    const outside_temps_file_path = ASSETS_DIR + "outside_temps/" + build_file_path(latitude, longitude);
    const solar_irradiances_file_path = ASSETS_DIR + "solar_irradiances/" + build_file_path(latitude, longitude);
    //console.log(agile_tariff_file_path);
    //console.log(outside_temps_file_path);
    //console.log(solar_irradiances_file_path);
    const agile_tariff = await read_array(agile_tariff_file_path);
    const outside_temps = await read_array(outside_temps_file_path);
    const solar_irradiances = await read_array(solar_irradiances_file_path);
    //console.log(agile_tariff);
    //console.log(outside_temps);
    //console.log(solar_irradiances);

    const result = run_simulation(thermostat_temperature, latitude, longitude, num_occupants,
        house_size, postcode, epc_space_heating, tes_volume_max, agile_tariff, outside_temps, solar_irradiances);
    return result;
}