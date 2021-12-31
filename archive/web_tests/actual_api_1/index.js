console.log("hello");

async function test1() {
    // insert email and apikey
    var postcode = 'TW7 5LA'
    var base64encodedData = (email + ':' + apikey).toString('base64');
    const response = await fetch('https://epc.opendatacommunities.org/api/v1/domestic/search?postcode=' + postcode, {
        headers: {
            'Authorization': 'Basic ' + base64encodedData,
            'Accept': 'application/json',
        },
        json: true
    })
    console.log(response);
    const json = await response.json()

    console.log(json);
}

test1();
