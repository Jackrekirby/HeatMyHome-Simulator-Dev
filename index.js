// (() => {
//     const console_log = window.console.log;
//     window.console.log = function (...args) {
//         console_log(...args);
//         var textarea = document.getElementById('my_console');
//         if (!textarea) return;
//         args.forEach(arg => textarea.value += `${JSON.stringify(arg)}\n`);
//     }
// })();

//import { response } from "express";

if (window.Worker) {
    var myWorker = new Worker('worker.js');
    var rustWorker = new Worker('rust_worker.js', { type: "module" });
    myWorker.postMessage("initiate");
    rustWorker.postMessage("initiate");
    //console.log(`Last Updated: ${last_updated}`);
} else {
    console.warn('Web Workers not supported');
}

console.log('origin:', location.origin);
var output_json;

myWorker.onmessage = function (e) {
    //console.log('Message received from worker:', e);
    if (e.data[0] == "initiation complete") {
        console.log('C++ Worker Initiated');
        console.log('Last Updated:', e.data[1]);
    } else if (e.data[0] == "simulation complete") {
        let end = performance.now();
        let runtime = ((end - e.data[2]) / 1000.0).toPrecision(3);
        console.log(`C++ Simulation Runtime: ${runtime}s`);
        document.getElementById('sim-runtime').innerHTML = runtime;
        output_json = JSON.parse(e.data[1]);
        document.getElementById('results').classList.remove("hide");
        console.log(output_json);
        draw_table(output_json);
        createDownloadFile();
        //renderSimTable();
    } else {
        console.warn('Message from worker is not linked to any event: ', e.data);
    }
}

rustWorker.onmessage = function (e) {
    //console.log('Message received from worker:', e);
    if (e.data[0] == "initiation complete") {
        console.log('Rust Worker Initiated');
    } else if (e.data[0] == "simulation complete") {
        let end = performance.now();
        let runtime = ((end - e.data[2]) / 1000.0).toPrecision(3);
        console.log(`Rust Simulation Runtime: ${runtime}s`);
        document.getElementById('sim-runtime').innerHTML = runtime;
        sim_string = e.data[1];
        document.getElementById('results').classList.remove("hide");
        renderSimTable();
    } else if (e.data[0] == "msg") {
        console.log(`Rust MSG: ${e.data[1]}`);
    } else {
        console.warn('Message from worker is not linked to any event: ', e.data);
    }
}

// url parameters =====================================================================
// api_search = {
//     postcode: "CV47AL", latitude: 52.3833, longitude: -1.5833, occupants: 2,
//     temperature: 20.0, space_heating: 3000, floor_area: 60.0, tes_max: 0.5
// };
//loadURLParams();

function loadDefaultParams() {
    const urlParams = new URLSearchParams(window.location.search);
    let autofill = Number(urlParams.get('autofill'));
    console.log('autofill: ', autofill);
    if (autofill == 1) {
        document.getElementById('sim-postcode').value = 'CV47AL';
        //document.getElementById('sim-latitude').value = 52.3833;
        //document.getElementById('sim-longitude').value = -1.5833;
        document.getElementById('sim-occupants').value = 2;
        document.getElementById('sim-temperature').value = 20.0;
        //document.getElementById('sim-space-heating').value = 3000;
        //document.getElementById('sim-floor-area').value = 60.0;
        document.getElementById('sim-tes-max').value = 0.5;
        updatePostCodeFields();
    } else if (autofill == 2) {
        document.getElementById('sim-postcode').value = 'CV47AL';
        document.getElementById('sim-latitude').value = 52.3833;
        document.getElementById('sim-longitude').value = -1.5833;
        document.getElementById('sim-occupants').value = 2;
        document.getElementById('sim-temperature').value = 20.0;
        document.getElementById('sim-space-heating').value = 3000;
        document.getElementById('sim-floor-area').value = 60.0;
        document.getElementById('sim-tes-max').value = 0.5;
    }
}

loadDefaultParams();

function generateParamUrl() {
    search = Array();
    const parameters = {
        postcode: document.getElementById('sim-postcode').value,
        'latitude': document.getElementById('sim-latitude').value,
        'longitude': document.getElementById('sim-longitude').value,
        'occupants': document.getElementById('sim-occupants').value,
        'temperature': document.getElementById('sim-temperature').value,
        'space_heating': document.getElementById('sim-space-heating').value,
        'floor_area': document.getElementById('sim-floor-area').value,
        'tes_max': document.getElementById('sim-tes-max').value,
    };
    for (const [key, value] of Object.entries(parameters)) {
        search.push(`${key}=${value}`);
    }
    //console.log(parameters);
    const url = location.protocol + '//' + location.host + location.pathname + `?${search.join('&')}`;
    console.log('save parameters url: ', url);
    navigator.clipboard.writeText(url);
    //window.history.replaceState({}, '', `?${search.join('&')}`);
}

function loadURLParams() {
    //console.log('current url parms string:', window.location.search);
    const urlParams = new URLSearchParams(window.location.search);
    //console.log(urlParams.get('postcode'), urlParams.get('longitude'))

    postcode = urlParams.get('postcode');
    if (postcode != null) {
        document.getElementById('sim-postcode').value = postcode;
        //api_search.postcode = postcode;
    }

    latitude = urlParams.get('latitude');
    if (latitude != null) {
        document.getElementById('sim-latitude').value = latitude;
        //api_search.latitude = latitude;
    }

    longitude = urlParams.get('longitude');
    if (longitude != null) {
        document.getElementById('sim-longitude').value = longitude;
        //api_search.longitude = longitude;
    }

    occupants = urlParams.get('occupants');
    if (occupants != null) {
        document.getElementById('sim-occupants').value = occupants;
        //api_search.occupants = occupants;
    }

    temperature = urlParams.get('temperature');
    if (temperature != null) {
        document.getElementById('sim-temperature').value = temperature;
        //api_search.temperature = temperature;
    }

    space_heating = urlParams.get('space_heating');
    if (space_heating != null) {
        document.getElementById('sim-space-heating').value = space_heating;
        //api_search.space_heating = space_heating;
    }

    floor_area = urlParams.get('floor_area');
    if (floor_area != null) {
        document.getElementById('sim-floor-area').value = floor_area;
        //api_search.floor_area = floor_area;
    }

    tes_max = urlParams.get('tes_max');
    if (tes_max != null) {
        document.getElementById('sim-tes-max').value = tes_max;
        //api_search.tes_max = tes_max;
    }
    // /?postcode=CV47AL&latitude=52.3833&longitude=-1.5833&occupants=2&temperature=20&space_heating=3000&floor_area=60&tes_max=0.5
    window.history.replaceState({}, document.title, "/");
    //updateURLParams();
}

loadURLParams();

// simulation =============================================================================

async function updatePostCodeFields() {
    let postcode = document.getElementById('sim-postcode').value;
    postcode = postcode.toUpperCase().replace(' ', '');
    document.getElementById('sim-postcode').value = postcode;

    //const data = await fetchJSON('https://api.postcodes.io/postcodes/' + 'HP169374');
    //console.log("DATA2: ", data);
    const postcode_url = 'https://api.postcodes.io/postcodes/' + postcode;
    fetch(postcode_url).then(response => response.json())
        .then(data => {
            if (data['status'] == 200) {
                console.log('Postcode Data: ', data);
                document.getElementById('sim-latitude').value = data.result.latitude;
                document.getElementById('sim-longitude').value = data.result.longitude;
                document.getElementById('sim-postcode').style.textDecorationLine = 'none';
                findAddress();
            } else {
                console.error('Error:', data['error']);
                document.getElementById('sim-postcode').style.textDecorationLine = 'line-through';
            }
        })
        .catch((error) => {
            console.error('Error:', error);
            document.getElementById('sim-postcode').style.textDecorationLine = 'line-through';
        });;

    // getJSON(postcode_url, function (err, data) {
    //     if (err != null) {
    //         console.error(err);
    //         document.getElementById('sim-postcode').style.textDecorationLine = 'line-through';
    //     } else {
    //         console.log('postcode: ', data);
    //         document.getElementById('sim-latitude').value = data.result.latitude;
    //         document.getElementById('sim-longitude').value = data.result.longitude;
    //         document.getElementById('sim-postcode').style.textDecorationLine = 'none';
    //         findAddress();
    //     }
    // });
}

function submitSimulation() {
    let postcode = document.getElementById('sim-postcode').value;

    postcode = postcode.toUpperCase().replace(' ', '');
    document.getElementById('sim-postcode').value = postcode;

    document.getElementById('results').classList.add("hide");

    simtable = document.getElementById('sim-table');
    while (document.getElementsByTagName('tr').length > 1) {
        simtable.removeChild(simtable.lastChild);
    }

    let latitude = document.getElementById('sim-latitude').value;
    let longitude = document.getElementById('sim-longitude').value;
    let occupants = document.getElementById('sim-occupants').value;
    let floor_area = document.getElementById('sim-floor-area').value;
    let temperature = document.getElementById('sim-temperature').value;
    let space_heating = document.getElementById('sim-space-heating').value;
    let tes_max = document.getElementById('sim-tes-max').value;
    let use_surface_optimisation = document.getElementById('sim-surface-optimisation').checked;
    let use_rust = document.getElementById('sim-use-rust').checked;

    document.getElementById('sim-runtime').innerHTML = '...';
    //console.log('asking worker to run simulation');
    if (use_rust) {
        rustWorker.postMessage(["run simulation", postcode, latitude, longitude, occupants, floor_area, temperature, space_heating, tes_max]);
    } else {
        myWorker.postMessage(["run simulation", [postcode, latitude, longitude, occupants, floor_area, temperature, space_heating, tes_max, use_surface_optimisation]]);
    }
    //console.log('sent message to worker');
}

document.getElementById('sim-use-rust').addEventListener('change', (event) => {
    let use_rust = document.getElementById('sim-use-rust').checked;
    if (use_rust) {
        document.getElementById('sim-surface-optimisation').checked = false;
        document.getElementById('sim-surface-optimisation').disabled = true;
    } else {
        document.getElementById('sim-surface-optimisation').disabled = false;
    }
});

// https://zetcode.com/javascript/jsonurl/
function getJSON(url, callback) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'json';

    xhr.onload = function () {

        var status = xhr.status;

        if (status == 200) {
            callback(null, xhr.response);
        } else {
            callback(status);
        }
    };
    xhr.send();
};

async function fetchJSON(url) {
    const response = await fetch(url);
    if (response.status >= 400 && response.status < 600) {
        throw new Error("Bad response from server");
    }

    if (!response.ok) {
        const message = `An error has occured: ${response.status}`;
        throw new Error(message);
    }
    const json = await response.json();
    return json;
};

// ================================================================================================================

let sortby_index = 7;
function compare(a, b) {
    const i = sortby_index;
    if (sortby_index == 4) {
        return 0;
    }

    if (a[i] < b[i]) {
        return -1;
    }
    if (a[i] > b[i]) {
        return 1;
    }
    return 0;
}

function renderSimTable() {
    //console.log(sim_string);
    simtable = document.getElementById('sim-table');

    simtable = document.getElementById('sim-table');
    while (document.getElementsByTagName('tr').length > 1) {
        simtable.removeChild(simtable.lastChild);
    }

    let group_systems = document.getElementById('group-systems-input').checked;

    let sortbyopt = document.getElementById('sortby');
    sortby_index = Number(sortbyopt.value);

    if (sortby_index > 0) {
        let sorts = [
            "none",
            "operational-expenditure",
            "capital-expenditure",
            "net-present-cost",
            "operational-emissions"];

        let sortname = sorts[sortby_index];
        //console.log(sortname);

        systems = output_json["systems"];
        let npcs = [];
        let system_arr = [];
        for (let heat_option of heat_options_json) {
            let heat_system = systems[heat_option];
            //console.log(system, heat_option);
            switch (heat_option) {
                case "electric-boiler":
                case "air-source-heat-pump":
                case "ground-source-heat-pump":
                    for (const [solar_option, system] of Object.entries(heat_system)) {
                        npcs.push(system[sortname]);
                        system_arr.push([heat_options_print[heat_option], solar_options_print[solar_option], system]);
                    }
                    break;
                case "hydrogen-boiler":
                case "hydrogen-fuel-cell":
                    for (const [h2_type, system] of Object.entries(heat_system)) {
                        npcs.push(system[sortname]);
                        system_arr.push([heat_options_print[heat_option], h2_type[0].toUpperCase() + h2_type.substring(1), system]);
                    }
                    break;
                default:
                    npcs.push(heat_system[sortname]);
                    system_arr.push([heat_options_print[heat_option], undefined, heat_system]);
                    break;
            }
        }

        //console.log(npcs);
        //console.log(system_arr);
        indexes = getSortIndices(npcs);
        //console.log("grouped_indexes", indexes);

        if (group_systems) {
            let grouped_indexes = [[], [], [], [], [], [], []];
            let first_indexes = [];
            let firsts = [true, true, true, true, true, true];
            for (let i of indexes) {
                if (i < 7) {
                    if (firsts[0]) { first_indexes.push(0); firsts[0] = false; }
                    grouped_indexes[0].push(i);
                } else if (i >= 7 && i < 14) {
                    if (firsts[1]) { first_indexes.push(1); firsts[1] = false; }
                    grouped_indexes[1].push(i);
                } else if (i >= 14 && i < 21) {
                    if (firsts[2]) { first_indexes.push(2); firsts[2] = false; }
                    grouped_indexes[2].push(i);
                } else if (i >= 21 && i < 24) {
                    if (firsts[3]) { first_indexes.push(3); firsts[3] = false; }
                    grouped_indexes[3].push(i);
                } else if (i >= 24 && i < 27) {
                    if (firsts[4]) { first_indexes.push(4); firsts[4] = false; }
                    grouped_indexes[4].push(i);
                } else if (i == 27) {
                    if (firsts[5]) { first_indexes.push(5); firsts[5] = false; }
                    grouped_indexes[5].push(27);
                } else if (i == 28) {
                    if (firsts[6]) { first_indexes.push(6); firsts[6] = false; }
                    grouped_indexes[6].push(27);
                }
            }
            //console.log("grouped_indexes: ", grouped_indexes);
            //console.log("first_indexes: ", first_indexes);

            let collapseGroupsEle = document.getElementById('collapse-groups-input');
            let collapseGroups = collapseGroupsEle.checked;

            if (collapseGroups) {
                for (let j of first_indexes) {
                    let i = grouped_indexes[j][0];
                    let tr = document.createElement('tr');
                    make_cell(tr, system_arr[i][0]);
                    make_cell(tr, system_arr[i][1]);
                    make_cells(tr, system_arr[i][2]);
                    simtable.appendChild(tr);
                };
            } else {
                for (let j of first_indexes) {
                    for (let i of grouped_indexes[j]) {
                        let tr = document.createElement('tr');
                        make_cell(tr, system_arr[i][0]);
                        make_cell(tr, system_arr[i][1]);
                        make_cells(tr, system_arr[i][2]);
                        simtable.appendChild(tr);
                    }
                };
            }

        } else {
            for (let i of indexes) {
                //console.log(i);
                let tr = document.createElement('tr');
                make_cell(tr, system_arr[i][0]);
                make_cell(tr, system_arr[i][1]);
                make_cells(tr, system_arr[i][2]);
                simtable.appendChild(tr);
            };
        }

    } else {
        draw_table(output_json);
    }
}

function updateGroupSystems() {
    let group_systems = document.getElementById('group-systems-input').checked;

    if (!group_systems) {
        document.getElementById('collapse-groups-input').checked = false;
        document.getElementById('collapse-groups-input').disabled = true;
    } else {
        document.getElementById('collapse-groups-input').disabled = false;
    }
    renderSimTable();
}

function updateSortBy() {
    renderSimTable();
}

function updateCollapseGroups() {
    renderSimTable();
}

// JSON OUTPUT =========================================================================================================================

function getSortIndices(array) {
    var array_with_index = [];
    for (let i in array) {
        array_with_index.push([array[i], i]);
    }
    array_with_index.sort(function (left, right) {
        return left[0] < right[0] ? -1 : 1;
    });
    let indexes = [];
    let t = [];
    for (let i in array_with_index) {
        t.push(Number(array_with_index[i][0]));
        indexes.push(Number(array_with_index[i][1]));
    }
    //console.log(t);
    return indexes;
}

const heat_options_json = ["electric-boiler", "air-source-heat-pump",
    "ground-source-heat-pump", "hydrogen-boiler", "hydrogen-fuel-cell",
    "biomass-boiler", "gas-boiler"];

const heat_options_print = {
    "electric-boiler": "EleBo", "air-source-heat-pump": "ASHP",
    "ground-source-heat-pump": "GSHP", "hydrogen-boiler": "H2Bo", "hydrogen-fuel-cell": "H2FC",
    "biomass-boiler": "BioBo", "gas-boiler": "GasBo"
};

const solar_options_json = ["none", "photovoltaic",
    "flat-plate", "evacuated-tube", "flat-plate-and-photovoltaic",
    "evacuated-tube-and-photovoltaic", "photovoltaic-thermal-hybrid"];

const solar_options_print = {
    "none": "None", "photovoltaic": "PV",
    "flat-plate": "FP", "evacuated-tube": "ET", "flat-plate-and-photovoltaic": "FP+PV",
    "evacuated-tube-and-photovoltaic": "ET+PV", "photovoltaic-thermal-hybrid": "PVT"
};

//console.log(JSON.stringify(j));
//document.getElementById("json-output").innerHTML = JSON.stringify(j, null, 2);

function draw_table(j) {
    const hydrogen_options_json = ["grey", "blue", "green"];

    let table = document.getElementById('sim-table');
    while (document.getElementsByTagName('tr').length > 1) {
        table.removeChild(table.lastChild);
    }

    //console.log(table);
    systems = j["systems"];
    for (let heat_option of heat_options_json) {
        let system = systems[heat_option];
        //console.log(system, heat_option);
        switch (heat_option) {
            case "electric-boiler":
            case "air-source-heat-pump":
            case "ground-source-heat-pump":
                for (const [solar_option, value] of Object.entries(system)) {
                    let tr = document.createElement('tr');
                    make_cell(tr, heat_options_print[heat_option]);
                    make_cell(tr, solar_options_print[solar_option]);
                    make_cells(tr, value);
                    table.appendChild(tr);
                }
                break;
            case "hydrogen-boiler":
            case "hydrogen-fuel-cell":
                for (const [h2_type, value] of Object.entries(system)) {
                    let tr = document.createElement('tr');
                    make_cell(tr, heat_options_print[heat_option]);
                    make_cell(tr, h2_type[0].toUpperCase() + h2_type.substring(1));
                    make_cells(tr, value);
                    table.appendChild(tr);
                }
                break;
            default:
                let tr = document.createElement('tr');
                make_cell(tr, heat_options_print[heat_option]);
                make_cell(tr, undefined);
                make_cells(tr, system);
                table.appendChild(tr);
                break;
        }
    }
}

function getPosition(string, subString, index) {
    return string.split(subString, index).join(subString).length;
}

function make_cell(tr, value) {
    //console.log("row", tr);
    let td = document.createElement('td');
    td.innerHTML = value == undefined ? '-' : value;
    tr.appendChild(td);
}

function make_cells(tr, system) {
    const headers = ["pv-size", "solar-thermal-size",
        "thermal-energy-storage-volume", "operational-expenditure", "capital-expenditure",
        "net-present-cost", "operational-emissions"];

    for (let key of headers) {
        switch (key) {
            case "net-present-cost":
                const net_present_cost = calculate_net_present_cost(
                    system["operational-expenditure"],
                    system["capital-expenditure"],
                    cumulative_discount_rate);
                make_cell(tr, Math.round(net_present_cost));
                break;
            case "operational-expenditure":
                make_cell(tr, Math.round(system[key]));
                break;
            case "capital-expenditure":
                make_cell(tr, Math.round(system[key]));
                break;
            case "operational-emissions":
                make_cell(tr, Math.round(system[key] / 1000));
                break;
            default:
                make_cell(tr, system[key]);
        }
    }
    return tr;
}

const discount_rate = 1.035; // 3.5% standard for UK HMRC
let npc_years = 20;
let cumulative_discount_rate = calculate_cumulative_discount_rate(discount_rate, npc_years);
//console.log("cumulative_discount_rate:", cumulative_discount_rate);
//draw_table(j);
// SIMULATION FUNCTIONS ===========================================================
function createDownloadFile() {
    let txt = JSON.stringify(output_json, null, 2);
    var myBlob = new Blob([txt], { type: "text/plain" });

    var dlink = document.getElementById("download-link");
    // (B) CREATE DOWNLOAD LINK
    var url = window.URL.createObjectURL(myBlob);
    dlink.href = url;
    dlink.download = "heatninja.json";
}

function clipboardJSON() {
    let txt = JSON.stringify(output_json, null, 2);
    navigator.clipboard.writeText(txt);
}

function calculate_systems() {

}

function calculate_net_present_cost(opex, capex, cumulative_discount_rate) {
    return capex + opex * cumulative_discount_rate;
}

function calculate_cumulative_discount_rate(discount_rate, npc_years) {
    let discount_rate_current = 1;
    let cumulative_discount_rate = 0;
    for (let year = 0; year < npc_years; year++) {
        cumulative_discount_rate += 1.0 / discount_rate_current;
        discount_rate_current *= discount_rate;
    }
    return cumulative_discount_rate;
}

function updateLifetime() {
    npc_years = document.getElementById('sim-lifetime').value;
    cumulative_discount_rate = calculate_cumulative_discount_rate(discount_rate, npc_years);
    draw_table(output_json);
}

// WEBSCRAPING
//const epc_api_url = 'http://heatmyhomeninja-env.eba-w2gamium.us-east-2.elasticbeanstalk.com/';
const epc_api_url = 'http://localhost:3000/';

function findAddress() {
    const postcode = document.getElementById('sim-postcode').value;
    fetch(`${epc_api_url}?postcode=${postcode}`).then(response => response.json())
        .then(data => {
            if (data['status'] == 200) {
                console.log('addresses: ', data);
                let element = document.getElementById('sim-addresses');
                //console.log(document.getElementsByTagName('option').length);
                while (element.getElementsByTagName('option').length > 0) {
                    element.removeChild(element.lastChild);
                }
                for (const [address, certificate] of data.result) {
                    //console.log(address, certificate);
                    let option_ele = document.createElement('option');
                    option_ele.value = certificate;
                    option_ele.text = address;
                    element.appendChild(option_ele);
                }
                getEpcData();
            } else {
                console.error('Error:', data['error']);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });;


    // getJSON(`${epc_api_url}?postcode=${postcode}`, function (err, response) {
    //     if (err != null) {
    //         console.error(err);
    //     } else {
    //         console.log('addresses: ', response);
    //         if (response['status'] == 200) {
    //             let data = response['result'];
    //             let element = document.getElementById('sim-addresses');
    //             //console.log(document.getElementsByTagName('option').length);
    //             while (element.getElementsByTagName('option').length > 0) {
    //                 element.removeChild(element.lastChild);
    //             }
    //             for (const [address, certificate] of data) {
    //                 //console.log(address, certificate);
    //                 let option_ele = document.createElement('option');
    //                 option_ele.value = certificate;
    //                 option_ele.text = address;
    //                 element.appendChild(option_ele);
    //             }
    //             getEpcData();
    //         }
    //     }
    // });
}

function getEpcData() { // comment
    let select = document.getElementById('sim-addresses');
    let certificate = select.options[select.selectedIndex].value;
    //console.log(certificate);

    fetch(`${epc_api_url}?certificate=${certificate}`).then(response => response.json())
        .then(data => {
            if (data['status'] == 200) {
                console.log('epc certificate: ', data);
                if (data['status'] == 200) {
                    const datar = data['result'];
                    if (datar['space-heating']) {
                        const space_heating = datar['space-heating'].match(/\d+/)[0];
                        document.getElementById('sim-space-heating').value = space_heating;
                    } else {
                        document.getElementById('sim-space-heating').value = null;
                    }
                    if (datar['floor-area']) {
                        const floor_area = datar['floor-area'].match(/\d+/)[0];
                        document.getElementById('sim-floor-area').value = floor_area;
                    } else {
                        document.getElementById('sim-floor-area').value = null;
                    }
                }
            } else {
                console.error('Error:', data['error']);
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });;

    // getJSON(`${epc_api_url}?certificate=${certificate}`, function (err, response) {
    //     if (err != null) {
    //         console.error(err);
    //     } else {
    //         console.log('epc certificate: ', response);
    //         if (response['status'] == 200) {
    //             const data = response['result'];
    //             if (data['space-heating']) {
    //                 const space_heating = data['space-heating'].match(/\d+/)[0];
    //                 document.getElementById('sim-space-heating').value = space_heating;
    //             } else {
    //                 document.getElementById('sim-space-heating').value = null;
    //             }
    //             if (data['floor-area']) {
    //                 const floor_area = data['floor-area'].match(/\d+/)[0];
    //                 document.getElementById('sim-floor-area').value = floor_area;
    //             } else {
    //                 document.getElementById('sim-floor-area').value = null;
    //             }
    //         }
    //     }
    // });
}