
function call_rust_simulator() {
    if (window.Worker) {
        let worker = new Worker('worker.js', { type: "module" });
        worker.onmessage = function (e) {
            if (!e.data) {
                console.error("Rust simulator failed");
            } else {
                worker.terminate();
                clearTimeout(simulation_timeout);
                console.log(JSON.parse(e.data));
            }
        }

        let simulation_timeout = setTimeout(() => {
            console.error("Rust simulator exceeded time limit");
        }, 120000);

        worker.postMessage("");
    }


}

call_rust_simulator();

