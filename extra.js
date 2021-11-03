console.defaultLog = console.log.bind(console);
logs = '';
console.log = function () {
    // default &  console.log()
    console.defaultLog.apply(console, arguments);
    // new & array data
    logs += Array.from(arguments).join(' ') + '\n';
    //console.defaultLog.apply(console, [logs]);
    document.getElementById('console-log').innerHTML = logs;
}

function clearConsole() {
    logs = '';
    document.getElementById('console-log').innerHTML = '';
}

console.log(`Last Updated: ${last_updated}`);

api_search = {
    postcode: "CV47AL", latitude: 52.3833, longitude: -1.5833, occupants: 2,
    temperature: 20.0, space_heating: 3000, floor_area: 60.0, tes_max: 0.5
};
loadURLParams();

function updateURLParams() {
    search = Array();
    //console.log('new url params:', api_search);
    for (const [key, value] of Object.entries(api_search)) {
        search.push(`${key}=${value}`);
    }
    //console.log('new url params string:', search.join('&'));

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

function submitCallClass() {
    let n = document.getElementById('callClass').value;
    console.log('input', n);

    var result = Module.ccall('call_class', // name of C function
        'number', // return type
        ['number'], // argument types
        [n]); // arguments

    console.log('result', result);
}

function submitPrintTxt() {
    var result = Module.ccall('print_example_file', // name of C function
        'number', // return type
        ['number'], // argument types
        [1]); // arguments]
}

// https://stackoverflow.com/questions/16396124/how-to-convert-comma-separated-string-into-numeric-array-in-javascript/33890740
function submitReturnVector() {
    var result = Module.ccall('return_vector', // name of C function
        'string', // return type
        ['number'], // argument types
        [1]); // arguments]
    console.log('result', result.split(",").map(Number));
}

function submitSqrtNum() {
    let n = document.getElementById('sqrtnum').value;
    console.log('input', n);
    var result = Module.ccall('int_sqrt', // name of C function
        'number', // return type
        ['number'], // argument types
        [n]); // arguments
    console.log('result', result);
}

function submitSimTest() {
    console.log('sim test');
    let start = performance.now();
    var result = Module.ccall('sim_test', // name of C function
        'number', // return type
        [], // argument types
        []); // arguments
    console.log('result', result);
    let end = performance.now();
    console.log(result, (end - start) / 1000.0);
}

function submitSimulation() {
    console.log('simulation started');

    let postcode = document.getElementById('sim-postcode').value;

    postcode = postcode.toUpperCase().replace(' ', '');
    document.getElementById('sim-postcode').value = postcode;

    getJSON('https://api.postcodes.io/postcodes/' + postcode, function (err, data) {
        if (err != null) {
            console.error(err);
        } else {
            console.log(data);
            document.getElementById('sim-latitude').value = data.result.latitude;
            document.getElementById('sim-longitude').value = data.result.longitude;
        }

        let latitude = document.getElementById('sim-latitude').value;
        let longitude = document.getElementById('sim-longitude').value;
        let occupants = document.getElementById('sim-occupants').value;
        let floor_area = document.getElementById('sim-floor-area').value;
        let temperature = document.getElementById('sim-temperature').value;
        let space_heating = document.getElementById('sim-space-heating').value;
        let tes_max = document.getElementById('sim-tes-max').value;

        setTimeout(function () {
            let start = performance.now();
            var result = Module.ccall('sim_test_args', // name of C function
                'number', // return type
                ['string', 'number', 'number', 'number', 'number', 'number', 'number', 'number'], // argument types
                [postcode, latitude, longitude, occupants, floor_area, temperature, space_heating, tes_max]); // arguments
            let end = performance.now();
            console.log(`Simulation Runtime: ${(end - start) / 1000.0}s`);
        }, 10);
    });


    // console.log('MARKER 1');
    // Module.ccall('sim_test_args', // name of C function
    //     'number', // return type
    //     ['string', 'number', 'number', 'number', 'number', 'number', 'number', 'number'], // argument types
    //     [postcode, latitude, longitude, occupants, floor_area, temperature, space_heating, tes_max],
    //     { async: true }).then(result => {
    //         console.log('MARKER 3');
    //         console.log('result', result);
    //         let end = performance.now();
    //         console.log(result, (end - start) / 1000.0);
    //     });
    // console.log('MARKER 2');
}

function submitLatLon() {
    let latlon = document.getElementById('latlon').value;
    console.log('input', latlon);
    var result = Module.ccall('print_outside_temps', // name of C function
        'number', // return type
        ['string'], // argument types
        [latlon]); // arguments
}

function getPostcodeInfo() {
    let postcode = document.getElementById('postcode').value;
    console.log(postcode);

    getJSON('https://api.postcodes.io/postcodes/' + postcode, function (err, data) {
        if (err != null) {
            console.error(err);
        } else {
            console.log(data);
            var text =
                `Postcode: ${data.result.postcode}
            Longitude: ${data.result.longitude}
            Latitude: ${data.result.latitude}`

            console.log(text);
        }
    });
    return null;
}

function submitPostcode() {
    let postcode = document.getElementById('postcode').value;
    console.log(postcode);

    getJSON('https://api.postcodes.io/postcodes/' + postcode, function (err, data) {
        if (err != null) {
            console.error(err);
        } else {
            console.log(data);
            var text =
                `Postcode: ${data.result.postcode}
            Longitude: ${data.result.longitude}
            Latitude: ${data.result.latitude}`

            console.log(text);
        }
    });
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

function speedTestWasm() {
    let n = document.getElementById('speedNum').value
    let start = performance.now();
    var result = Module.ccall('speed_test', // name of C function
        'number', // return type
        ['number'], // argument types
        [n]); // arguments
    let end = performance.now();
    console.log(result, (end - start) / 1000.0);
}

function speedTest() {
    //let n = document.getElementById('speedNumJS').value
    hours = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24];
    total = 0;
    let start = performance.now();
    for (let i = 0; i < 100000; ++i) {
        for (let day = 0; day < 365; ++day) {
            for (let hour = 0; hour < 24; ++hour) {
                total += hours[hour];
            }
        }
        //std::cout << total << '\n';
    }
    let end = performance.now();
    console.log(total, (end - start) / 1000.0);
}
// speedTest();
// speedTestWasm();
