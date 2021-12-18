onmessage = function (e) {
    //console.log('Message received from main script. Message: ', e);
    if (e.data[0] == "run simulation") {
        let result = submitSimulation(e.data[1]);
        postMessage(["simulation complete", result]);
    } else if (e.data == "initiate") {
        console.log('Worker Initiated');
    }
    else {
        console.warn('Message to worker is not linked to any event: ', e.data);
    }
}

function submitSimulation(arguments) {
    //console.log("sim args: ", arguments);
    let result = Module.ccall('run_simulation', // name of C function
        'string', // return type
        ['string', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'bool'], // argument types
        arguments); // arguments
    return result;
}