function submitNumber() {
    let n = document.getElementById('number').value;
    console.log(n);

    let address = document.getElementById('address').value;
    console.log(address);

    var result = Module.ccall('int_sqrt', // name of C function
        'number', // return type
        ['number'], // argument types
        [n]); // arguments

    var result = Module.ccall('print_file', // name of C function
        'number', // return type
        ['string'], // argument types
        [address]); // arguments

    var result = Module.ccall('get_message', // name of C function
        'string', // return type
        ['number'], // argument types
        [1]); // arguments
    console.log(result.split(",").map(Number));
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
