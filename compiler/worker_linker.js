onmessage = function (e) {
    //console.log('Message received from main script. Message: ', e);
    let start = performance.now();
    if (e.data[0] == "run simulation") {
        let result = submit_simulation(e.data[1]);
        postMessage(["simulation complete", result, start]);
    } else if (e.data == "initiate") {
        postMessage(["initiation complete"]);
    }
    else {
        console.warn('Message to worker is not linked to any event: ', e.data);
    }
}

const parameter_name_list = [
    'postcode',
    'latitude',
    'longitude',
    'occupants',
    'floor_area',
    'temperature',
    'space_heating',
    'tes_max',
    'surface_optimisation'
];

function submit_simulation(inputs) {
    let arguments = [];
    // arguments:  [postcode, latitude, longitude, occupants, floor_area, temperature, space_heating, tes_max, use_surface_optimisation]
    for (const parameter of parameter_name_list) {
        arguments.push(inputs[parameter]);
    }

    console.log("simulation arguments: ", arguments);
    let result = Module.ccall('run_simulation', // name of C function
        'string', // return type
        ['string', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'bool'], // argument types
        arguments); // arguments
    return result;
}