async function call_postcode_api() {
    const postcode_url = 'http://localhost:3000/?postcode=HP160LU';
    fetch(postcode_url).then(response => response.json())
        .then(data => {
            console.log('data:', data);
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

call_postcode_api();