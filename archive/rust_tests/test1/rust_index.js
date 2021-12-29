import init, { run_simulation, test, pls_data } from "/rust_simulator/pkg/rust_simulator.js";

function build_simulation() {
    let postcode = document.getElementById('postcode').value;
    let latitude = document.getElementById('latitude').value;
    let longitude = document.getElementById('longitude').value;

    console.log(`postcode: ${postcode}, latitude: ${latitude}, longitude: ${longitude}`);
    const result = run_simulation(postcode, latitude, longitude);
    console.log(`run_simulation: ${result}`);
}

document.getElementById('run_button').addEventListener('click', build_simulation);

function readNumberArrayFile2(filepath) {
    let file = new XMLHttpRequest();
    file.open("GET", filepath, true);
    file.onreadystatechange = function () {
        if (file.readyState === 4) {
            console.log(file.responseText.split(/\r?\n/).map(Number));
            return file.responseText.split(/\r?\n/).map(Number);
        }
    }
    file.send(); // asks server for file
}

function readNumberArrayFile(filepath, callback) {
    let file = new XMLHttpRequest();
    file.open("GET", filepath, true);
    file.onreadystatechange = function () {
        if (file.readyState === 4) {
            //console.log(file.responseText.split(/\r?\n/).map(Number));
            callback(file.responseText.split(/\r?\n/).map(Number));
        }
    }
    file.send();
}

function build_file_path(latitude, longitude, datatype) {
    return `lat_${(Math.round(latitude * 2.0) / 2.0).toFixed(1)}_lon_${(Math.round(longitude * 2.0) / 2.0).toFixed(1)}.csv`;
}

function runme(data) {
    console.log(data);
}

async function read_array(filepath) {
    const resp = await fetch(filepath);
    const text = await resp.text();
    return text.split(/\r?\n/).map(Number);
}

async function run() {
    await init();

    const ASSETS_DIR = "/rust_simulator/assets/";
    let latitude = 52.3833;
    let longitude = -1.5833;

    const agile_tariff_file_path = ASSETS_DIR + "agile_tariff.csv";
    const outside_temps_file_path = ASSETS_DIR + "outside_temps/" + build_file_path(latitude, longitude);
    const solar_irradiances_file_path = ASSETS_DIR + "solar_irradiances/" + build_file_path(latitude, longitude);
    console.log(agile_tariff_file_path);
    console.log(outside_temps_file_path);
    console.log(solar_irradiances_file_path);
    const agile_tariff = await read_array(agile_tariff_file_path);
    const outside_temps = await read_array(outside_temps_file_path);
    const solar_irradiances = await read_array(solar_irradiances_file_path);
    //console.log(agile_tariff);
    //console.log(outside_temps);
    //console.log(solar_irradiances);

    //pls_data(outside_temps, solar_irradiances, agile_tariff);
    //const result2 = run_simulation(20.0, 52.3833, -1.5833, 2, 60.0, "CV4 7AL", 3000.0, 0.5);
    //console.log(result2);
    // thermostat_temperature, latitude, longitude, num_occupants, house_size, postcode, epc_space_heating, tes_volume_max
    const result2 = run_simulation(20.0, 52.3833, -1.5833, 2, 60.0, "CV4 7AL", 3000.0, 0.5, agile_tariff, outside_temps, solar_irradiances);
    console.log(`run_simulation: ${result2}`);
}



run();