(() => {
    const console_log = window.console.log;
    window.console.log = function (...args) {
        console_log(...args);
        var textarea = document.getElementById('my_console');
        if (!textarea) return;
        args.forEach(arg => textarea.value += `${JSON.stringify(arg)}\n`);
    }
})();

if (window.Worker) {
    var myWorker = new Worker('worker.js');
    var rustWorker = new Worker('rust_worker.js', { type: "module" });
    myWorker.postMessage("initiate");
    rustWorker.postMessage("initiate");
    //console.log(`Last Updated: ${last_updated}`);
} else {
    console.warn('Web Workers not supported');
}

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
api_search = {
    postcode: "CV47AL", latitude: 52.3833, longitude: -1.5833, occupants: 2,
    temperature: 20.0, space_heating: 3000, floor_area: 60.0, tes_max: 0.5
};
//loadURLParams();

function updateURLParams() {
    search = Array();
    for (const [key, value] of Object.entries(api_search)) {
        search.push(`${key}=${value}`);
    }
    window.history.replaceState({}, '', `?${search.join('&')}`);
}

function loadURLParams() {
    //console.log('current url parms string:', window.location.search);
    const urlParams = new URLSearchParams(window.location.search);
    //console.log(urlParams.get('postcode'), urlParams.get('longitude'))

    postcode = urlParams.get('postcode');
    if (postcode != null) {
        document.getElementById('sim-postcode').value = postcode;
        api_search.postcode = postcode;
    }

    latitude = urlParams.get('latitude');
    if (latitude != null) {
        document.getElementById('sim-latitude').value = latitude;
        api_search.latitude = latitude;
    }

    longitude = urlParams.get('longitude');
    if (longitude != null) {
        document.getElementById('sim-longitude').value = longitude;
        api_search.longitude = longitude;
    }

    occupants = urlParams.get('occupants');
    if (occupants != null) {
        document.getElementById('sim-occupants').value = occupants;
        api_search.occupants = occupants;
    }

    temperature = urlParams.get('temperature');
    if (temperature != null) {
        document.getElementById('sim-temperature').value = temperature;
        api_search.temperature = temperature;
    }

    space_heating = urlParams.get('space_heating');
    if (space_heating != null) {
        document.getElementById('sim-space-heating').value = space_heating;
        api_search.space_heating = space_heating;
    }

    floor_area = urlParams.get('floor_area');
    if (floor_area != null) {
        document.getElementById('sim-floor-area').value = floor_area;
        api_search.floor_area = floor_area;
    }

    tes_max = urlParams.get('tes_max');
    if (tes_max != null) {
        document.getElementById('sim-tes-max').value = tes_max;
        api_search.tes_max = tes_max;
    }
    updateURLParams();
}

// simulation =============================================================================

document.getElementById('sim-postcode').addEventListener('change', (event) => {
    let postcode = document.getElementById('sim-postcode').value;
    postcode = postcode.toUpperCase().replace(' ', '');
    document.getElementById('sim-postcode').value = postcode;

    getJSON('https://api.postcodes.io/postcodes/' + postcode, function (err, data) {
        if (err != null) {
            console.error(err);
            document.getElementById('sim-postcode').style.textDecorationLine = 'line-through';
        } else {
            console.log(data);
            document.getElementById('sim-latitude').value = data.result.latitude;
            document.getElementById('sim-longitude').value = data.result.longitude;

            document.getElementById('sim-postcode').style.textDecorationLine = 'none';
        }
    });
});

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

    let groupbyopt = document.getElementById('groupby');
    groupby_index = Number(groupbyopt.value);

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
        console.log(sortname);

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
                        system_arr.push([heat_option, solar_option, system]);
                    }
                    break;
                case "hydrogen-boiler":
                case "hydrogen-fuel-cell":
                    for (const [h2_type, system] of Object.entries(heat_system)) {
                        npcs.push(system[sortname]);
                        system_arr.push([heat_option, h2_type, system]);
                    }
                    break;
                default:
                    npcs.push(heat_system[sortname]);
                    system_arr.push([heat_option, undefined, heat_system]);
                    break;
            }
        }

        console.log(npcs);
        console.log(system_arr);
        indexes = getSortIndices(npcs);
        console.log(indexes);

        for (let i of indexes) {
            //console.log(i);
            let tr = document.createElement('tr');
            make_cell(tr, heat_options_print[system_arr[i][0]]);
            make_cell(tr, solar_options_print[system_arr[i][1]]);
            make_cells(tr, system_arr[i][2]);
            simtable.appendChild(tr);
        }
    } else {
        draw_table(output_json);
    }


    return;

    let speci = 0;
    switch (groupby_index) {
        case 0:
            speci = Array(21).keys();
            output_json.sort(compare);
            break;
        case 1:
            speci = [0, 7, 14];
            //speci = Array(21).keys();
            a = output_json.slice(0, 7).sort(compare);
            b = output_json.slice(7, 14).sort(compare);
            c = output_json.slice(14, 21).sort(compare);
            output_json = a.concat(b, c);
            break;
        default: // 2
            b = output_json;
            output_json = [];
            for (let ii of Array(7).keys()) {
                let a = [b[ii], b[ii + 7], b[ii + 14]];
                a.sort(compare);
                //console.log(a);
                output_json.push(a[0]);
                output_json.push(a[1]);
                output_json.push(a[2]);
            }
            speci = [0, 3, 6, 9, 12, 15, 18];
        //part_json.sort(compare);
    }

    let collapseGroupsEle = document.getElementById('collapse-groups-input');
    let collapseGroups = collapseGroupsEle.checked;
    //console.log(collapseGroups);
    if (!collapseGroups) {
        speci = Array(21).keys();
    }
    // let part_json = [];
    // for (let ii of speci) {
    //     part_json.push(output_json[ii]);
    // };
    //console.log(output_json);

    for (let ii of speci) {
        //console.log(ii);
        let tr = document.createElement('tr');
        for (let value of output_json[ii]) {
            let td = document.createElement('td');
            td.innerHTML = value;
            tr.appendChild(td);
        }
        simtable.appendChild(tr);
    }
}

function updateGroupBy() {
    let groupbyopt = document.getElementById('groupby');
    groupby_index = Number(groupbyopt.value);
    if (groupby_index == 0) {
        document.getElementById('collapse-groups').classList.add("hide");;
    } else {
        document.getElementById('collapse-groups').classList.remove("hide");;
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
    console.log(t);
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
//clipboardJSON(j);
// SIMULATION FUNCTIONS ===========================================================

function clipboardJSON(j) {
    navigator.clipboard.writeText(JSON.stringify(j, null, 2));
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
    draw_table(j);
}
