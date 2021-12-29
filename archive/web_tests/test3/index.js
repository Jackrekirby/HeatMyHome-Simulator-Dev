function httpGet(theUrl) {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", theUrl); // false for synchronous request
    xmlHttp.send(null);
    return xmlHttp.responseText;
}

console.log(httpGet('https://www.gov.uk/find-energy-certificate'));