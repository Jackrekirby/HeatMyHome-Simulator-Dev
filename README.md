# HeatMyHome Simulator Development
This repository was used to develop the heating simulator for the [HeatMyHome.ninja](https://heatmyhome.ninja) website.

In summary, the website simulates a range of domestic heating systems alongside solar photovoltaic and solar thermal technologies to optimise the system configuration and estimate the operation costs, capital costs and operating emissions for each of the technologies simulated. It takes as input the user's location and home details to find the optimal heating system unique to the user's circumstances.


The GitHub repository for HeatMyHome can be found at [here](https://github.com/heatmyhome-ninja/HeatMyHome-Website). The repository used to develop the HeatMyHome user interface can be found [here](https://github.com/Jackrekirby/heat_ninja_ui). The final simulation code along with API, and user interface can be found at the HeatMyHome repository. However this repository contains additional raw files, and one can go back through the commit history to view its progression. 

A debug UI is used to interface with the simulator, which can be found [here](https://jackrekirby.github.io/Heating-Simulator/). If you wish to try the simulator with autofilled parameters click [here](https://jackrekirby.github.io/Heating-Simulator/?autofill=2).

Here is a guide to the folder structure:
* `active_tests` contains any tests that are currently ongoing. Once a test has been completed, either because it failed, or has been implemented into the design is transferred to the `archive` folder.
* `api` contains the NodeJS server, built using Express that enables simulations to be processes server-side and calls the Government website to fetch Energy Performance Certificates.
* `compiler` contains the python script used as a wrapper to interface with Emscripten which compiles the C++ simulator to Web Assembly (WASM) and merges JavaScript files into a single file.
* `docs` contains the HTML, CSS, JS, WASM  and other resources used by GitHub pages for the debug UI.
* `matlab` contains MATLAB files which were used to visualise the outputs of the simulator, in particular, visualisation of the liftime cost of each primary-heater solar-anciller combination as a surface, and the global optimisation algorithm developed to find the global minimum of each surface.
* `rust_simulator` contains the code and assets needed to run the simulator using Rust natively. It also contains the code required to compile the Rust simulator to WASM, either to NodeJS or for client-side use. Within the code is also means to generate output files in either CSV or JSON format, the ability to load input paramaters from file, and the ability to generate a performance analysis CSV file.
* `wasm_simulator_v2` contains the second version of the C++ simulator. The first was written in an OOP approach, and the second using a functional approach. A 'EMSCRIPTEN ' macro is used to disable features not supported by WASM if you wish to compiled the simulator to WASM.

Please contact me if you wish to use the simulator and need help navigating the repository.
