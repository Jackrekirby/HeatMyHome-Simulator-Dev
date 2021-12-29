console.log("hello");

async function testpls() {
    var email = 'skurendhand@gmail.com'
    var apikey = '37bef2fc4f0a1cd6daf270ed886658e7adfe4c39'
    var postcode = 'TW7 5LA'
    var base64encodedData = (email + ':' + apikey).toString('base64');
    const response = await fetch('https://epc.opendatacommunities.org/api/v1/domestic/search?postcode=' + postcode, {
        headers: {
            'Authorization': 'Basic ' + base64encodedData,
            'Accept': 'application/json',
        },
        json: true
    })

    const json = await response.json()

    console.log(json);
}

testpls();