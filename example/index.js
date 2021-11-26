// // https://zetcode.com/javascript/jsonurl/
// function getJSON(url, callback) {
//     var xhr = new XMLHttpRequest();
//     xhr.open('GET', url, true);
//     xhr.responseType = 'json';

//     xhr.onload = function () {

//         var status = xhr.status;

//         if (status == 200) {
//             callback(null, xhr.response);
//         } else {
//             callback(status);
//         }
//     };
//     xhr.send();
// };

// // 'https://api.postcodes.io/postcodes/'
// let postcode = 'HP160LU'
// let url = 'https://find-energy-certificate.digital.communities.gov.uk/find-a-certificate/search-by-postcode'
// getJSON(url, function (err, data) {
//     if (err != null) {
//         console.error(err);
//     } else {
//         console.log(data);
//     }
// });

// $.get('https://api.postcodes.io/postcodes/HP160LU', function (response) { console.log(response); });
// console.log('HERE');
// let suburl = "https://api.postcodes.io/postcodes/";
// let search = "HP160LU";
// let url = "http://anyorigin.com/go?url=" + encodeURIComponent(suburl) + search + "&callback=?";

// console.log(url);
// $.get(url, function (response) { console.log('A'); console.log(`<${response}>`); });
// console.log('HERE');

// let url = 'https://en.wikipedia.org/wiki/Jack';
// $.getJSON('http://www.whateverorigin.org/get?url=' + encodeURIComponent(url) + '&callback=?', function (data) {
//     console.log(data);
// });