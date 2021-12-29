
async function test() {
    const response = await fetch("https://epc.opendatacommunities.org/api/v1/domestic/search", {
        'method': "GET",
        'headers': {
            'Authorization': "Basic f2483d514d64e0e407e4ef1c7c62bcdd2e77625f",
            'Accept': "application/json"
        },
        'mode': 'cors',
    });
    console.log(response);
}

test();