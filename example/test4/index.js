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



function findAddress() {
    const postcode = document.getElementById('sim-postcode').value;
    getJSON(`http://localhost:3000/?postcode=${postcode}`, function (err, data) {
        if (err != null) {
            console.error(err);
        } else {
            let element = document.getElementById('sim-addresses');
            while (document.getElementsByTagName('option').length > 0) {
                element.removeChild(element.lastChild);
            }
            for (const [address, certificate] of data) {
                console.log(address, certificate);
                let option_ele = document.createElement('option');
                option_ele.value = certificate;
                option_ele.text = address;
                element.appendChild(option_ele);
            }

        }
    });
}

function getEpcData() {
    let select = document.getElementById('sim-addresses');
    let certificate = select.options[select.selectedIndex].value;
    console.log(certificate); // en
    getJSON(`http://localhost:3000/?certificate=${certificate}`, function (err, data) {
        if (err != null) {
            console.error(err);
        } else {
            console.log(data);
            const space_heating = data['space-heating'].match(/\d+/)[0];
            const floor_area = data['floor-area'].match(/\d+/)[0]
            document.getElementById('sim-epc-space-heating').value = space_heating;
            document.getElementById('sim-floor-area').value = floor_area;
        }
    });
}