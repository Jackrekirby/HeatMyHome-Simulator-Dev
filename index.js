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

var sim_string;

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
        sim_string = e.data[1];
        document.getElementById('results').classList.remove("hide");
        renderSimTable();
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
    output_json = JSON.parse(sim_string);
    simtable = document.getElementById('sim-table');

    let groupbyopt = document.getElementById('groupby');
    groupby_index = Number(groupbyopt.value);

    let sortbyopt = document.getElementById('sortby');
    sortby_index = Number(sortbyopt.value) + 4;

    simtable = document.getElementById('sim-table');
    while (document.getElementsByTagName('tr').length > 1) {
        simtable.removeChild(simtable.lastChild);
    }

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
