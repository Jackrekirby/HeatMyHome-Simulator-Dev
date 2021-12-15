if (window.Worker) {
    var myWorker = new Worker('worker.js');
    var rustWorker = new Worker('rust_worker.js', { type: "module" });
    myWorker.postMessage("initiate");
    rustWorker.postMessage("initiate");
    //console.log(`Last Updated: ${last_updated}`);
} else {
    console.warn('Web Workers not supported');
}

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
        renderSimTable(e.data[1]);
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
        renderSimTable(e.data[1]);
    } else {
        console.warn('Message from worker is not linked to any event: ', e.data);
    }
}

// url parameters =====================================================================
api_search = {
    postcode: "CV47AL", latitude: 52.3833, longitude: -1.5833, occupants: 2,
    temperature: 20.0, space_heating: 3000, floor_area: 60.0, tes_max: 0.5
};
loadURLParams();

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


    simtable = document.getElementById('sim-table');
    simtable.classList.add("hide");
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

function renderSimTable(sim_string) {
    console.log(sim_string);
    specslist = JSON.parse(sim_string);
    simtable = document.getElementById('sim-table');

    for (let specs of specslist) {
        //console.log(specs);
        let tr = document.createElement('tr');
        for (let value of specs) {
            let td = document.createElement('td');
            td.innerHTML = value;
            tr.appendChild(td);
        }
        simtable.appendChild(tr);
    }
    simtable.classList.remove("hide");
}

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