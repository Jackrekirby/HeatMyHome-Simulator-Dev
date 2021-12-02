if (window.Worker) {
    var myWorker = new Worker('worker.js');

    myWorker.postMessage("hello worker!");
    console.log('sent message to worker');

    myWorker.onmessage = function (e) {
        console.log('Message received from worker:', e);
    }

    myWorker.postMessage("hello worker again!");
} else {
    console.log('Web Workers not supported');
}

