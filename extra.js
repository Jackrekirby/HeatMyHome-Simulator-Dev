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
