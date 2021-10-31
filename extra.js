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

function submitLatLon() {
    let latlon = document.getElementById('latlon').value;
    console.log('input', latlon);
    var result = Module.ccall('print_outside_temps', // name of C function
        'number', // return type
        ['string'], // argument types
        [latlon]); // arguments
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

console.defaultLog = console.log.bind(console);
logs = '';
console.log = function () {
    // default &  console.log()
    console.defaultLog.apply(console, arguments);
    // new & array data
    logs += Array.from(arguments).join(' ') + '\n';
    console.defaultLog.apply(console, [logs]);
    document.getElementById('console-log').innerHTML = logs;
}

function clearConsole() {
    logs = '';
    document.getElementById('console-log').innerHTML = '';
}
