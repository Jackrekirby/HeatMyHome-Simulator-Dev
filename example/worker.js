var t = 0;

onmessage = function (e) {
    console.log('Message received from main script. Message: ', e);
    console.log('Posting message back to main script');
    t++;
    postMessage(["Hello main!", t]);
}