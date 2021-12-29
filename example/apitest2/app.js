
import fetch from 'node-fetch';

//var requestPromise = require('request-promise');
var email = 'skurendhand@gmail.com'
var apikey = '37bef2fc4f0a1cd6daf270ed886658e7adfe4c39'
var postcode = 'TW7 5LA'
var base64encodedData = Buffer.from(email + ':' + apikey).toString('base64');
console.log(base64encodedData);

const response = await fetch('https://epc.opendatacommunities.org/api/v1/domestic/search?postcode=' + postcode, {
    headers: {
        'Authorization': 'Basic ' + base64encodedData,
        'Accept': 'application/json',
    },
    json: true
})

const json = await response.json()

console.log(json);

function archive() {
    requestPromise.get({
        uri: 'https://epc.opendatacommunities.org/api/v1/domestic/search?postcode=' + postcode,
        headers: {
            'Authorization': 'Basic ' + base64encodedData
        },
        json: true
    })
        .then(function ok(jsonData) {
            // console.log(jsonData)
            var jsonRows = (jsonData[Object.keys(jsonData)[1]]);
            let result2 = [];
            let resultKey = [];
            for (i in jsonRows) {
                result2.push(getNames(jsonRows[i], 'address'))
                resultKey.push(getNames(jsonRows[i], 'lmk-key'))
            }
            console.log(result2)
        })
        .catch(function fail(error) {
            // handle error
        });
    function getNames(obj, name) {
        let result = [];
        for (var key in obj) {
            if (obj.hasOwnProperty(key)) {
                if ("object" == typeof (obj[key])) {
                    getNames(obj[key], name);
                } else if (key == name) {
                    result.push(obj[key]);
                }
            }
        }
        return result
    }
}