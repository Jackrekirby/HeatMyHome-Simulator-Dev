const heat_options = {
    ERH: "Electric Boiler",
    ASHP: "ashp",
    GSHP: "gshp",
    HYDROGEN_BOILER: "Hydrogen Boiler",
    HYDROGEN_FUEL_CELL: "Hydrogen Fuel Cell",
    GAS: "gas",
    BIOMASS: "biomass",
}

const hydrogen_options = {
    GREY: "grey",
    BLUE: "blue",
    GREEN: "green",
}


let heat_solar_json = {
    "pv-size": 14,
    "solar-thermal-size": 2,
    "thermal-energy-storage-volume": 0.1,
    "operational-expenditure": 232,
    "capital-expenditure": 679,
    "net-present-cost": 4093,
    "operational-emissions": 214, // remove and calculate when called
}

let heat_json = {
    "operational-expenditure": 232,
    "capital-expenditure": 679,
    "net-present-cost": 4093,
    "operational-emissions": 214, // remove and calculate when called
}

let demand = {
    "hot-water": 2309,
    "space": 872,
    "total": 3109,
    "peak-hourly": 178,
}
// systems: heat option : solar option
// let j = {
//     "demand": { "boiler": demand, "heat-pump": demand },
//     "systems": {
//         "electric-boiler": {
//             "none": heat_solar_json,
//             "photovoltaic": heat_solar_json,
//             "flat-plate": heat_solar_json,
//             "evacuated-tube": heat_solar_json,
//             "flat-plate-and-photovoltaic": heat_solar_json,
//             "evacuated-tube-and-photovoltaic": heat_solar_json,
//             "photovoltaic-thermal-hybrid": heat_solar_json,
//         },
//         "air-source-heat-pump": {
//             "none": heat_solar_json,
//             "photovoltaic": heat_solar_json,
//             "flat-plate": heat_solar_json,
//             "evacuated-tube": heat_solar_json,
//             "flat-plate-and-photovoltaic": heat_solar_json,
//             "evacuated-tube-and-photovoltaic": heat_solar_json,
//             "photovoltaic-thermal-hybrid": heat_solar_json,
//         },
//         "ground-source-heat-pump": {
//             "none": heat_solar_json,
//             "photovoltaic": heat_solar_json,
//             "flat-plate": heat_solar_json,
//             "evacuated-tube": heat_solar_json,
//             "flat-plate-and-photovoltaic": heat_solar_json,
//             "evacuated-tube-and-photovoltaic": heat_solar_json,
//             "photovoltaic-thermal-hybrid": heat_solar_json,
//         },
//         "hydrogen-boiler": {
//             "grey": heat_json,
//             "blue": heat_json,
//             "green": heat_json,
//         },
//         "hydrogen-fuel-cell": {
//             "grey": heat_json,
//             "blue": heat_json,
//             "green": heat_json,
//         },
//         "biomass-boiler": heat_json,
//         "gas-boiler": heat_json,
//     }
// };

const j = JSON.parse('{"demand":{"boiler":{"hot-water":1460.07,"space":2737.49,"total":4197.57,"peak-hourly":9.73932},"heat-pump":{"hot-water":1460.07,"space":2844.84,"total":4304.91,"peak-hourly":1.67154}},"systems":{"electric-boiler":{"none":{"pv-size":0,"solar-thermal-size":0,"thermal-energy-storage-volume":0.2,"operational-expenditure":372.953,"capital-expenditure":949.343,"net-present-cost":6435.42,"operational-emissions":924093},"photovoltaic":{"pv-size":14,"solar-thermal-size":0,"thermal-energy-storage-volume":0.1,"operational-expenditure":173.281,"capital-expenditure":3758.91,"net-present-cost":6307.84,"operational-emissions":515562},"flat-plate":{"pv-size":0,"solar-thermal-size":2,"thermal-energy-storage-volume":0.2,"operational-expenditure":316.496,"capital-expenditure":3526.84,"net-present-cost":8182.45,"operational-emissions":774946},"evacuated-tube":{"pv-size":0,"solar-thermal-size":2,"thermal-energy-storage-volume":0.2,"operational-expenditure":305.393,"capital-expenditure":3636.84,"net-present-cost":8129.13,"operational-emissions":746851},"flat-plate-and-photovoltaic":{"pv-size":12,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":168.303,"capital-expenditure":5896.41,"net-present-cost":8372.12,"operational-emissions":477528},"evacuated-tube-and-photovoltaic":{"pv-size":12,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":147.247,"capital-expenditure":6006.41,"net-present-cost":8172.39,"operational-emissions":424266},"photovoltaic-thermal-hybrid":{"pv-size":2,"solar-thermal-size":2,"thermal-energy-storage-volume":0.2,"operational-expenditure":299.731,"capital-expenditure":4956.84,"net-present-cost":9365.84,"operational-emissions":747549}},"air-source-heat-pump":{"none":{"pv-size":0,"solar-thermal-size":0,"thermal-energy-storage-volume":0.1,"operational-expenditure":144.877,"capital-expenditure":6237.67,"net-present-cost":8368.78,"operational-emissions":355062},"photovoltaic":{"pv-size":14,"solar-thermal-size":0,"thermal-energy-storage-volume":0.1,"operational-expenditure":-106.519,"capital-expenditure":9317.67,"net-present-cost":7750.8,"operational-emissions":-23068.2},"flat-plate":{"pv-size":0,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":128.368,"capital-expenditure":8815.17,"net-present-cost":10703.4,"operational-emissions":324390},"evacuated-tube":{"pv-size":0,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":123.718,"capital-expenditure":8925.17,"net-present-cost":10745,"operational-emissions":315712},"flat-plate-and-photovoltaic":{"pv-size":12,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":-77.2658,"capital-expenditure":11455.2,"net-present-cost":10318.6,"operational-emissions":6971.75},"evacuated-tube-and-photovoltaic":{"pv-size":12,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":-90.3788,"capital-expenditure":11565.2,"net-present-cost":10235.7,"operational-emissions":-9912.1},"photovoltaic-thermal-hybrid":{"pv-size":2,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":111.131,"capital-expenditure":10245.2,"net-present-cost":11879.9,"operational-emissions":295010}},"ground-source-heat-pump":{"none":{"pv-size":0,"solar-thermal-size":0,"thermal-energy-storage-volume":0.1,"operational-expenditure":89.6801,"capital-expenditure":7937.67,"net-present-cost":9256.85,"operational-emissions":220896},"photovoltaic":{"pv-size":14,"solar-thermal-size":0,"thermal-energy-storage-volume":0.1,"operational-expenditure":-198.088,"capital-expenditure":11017.7,"net-present-cost":8103.82,"operational-emissions":-163122},"flat-plate":{"pv-size":0,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":77.3645,"capital-expenditure":10515.2,"net-present-cost":11653.2,"operational-emissions":202183},"evacuated-tube":{"pv-size":0,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":74.2573,"capital-expenditure":10625.2,"net-present-cost":11717.5,"operational-emissions":197829},"flat-plate-and-photovoltaic":{"pv-size":12,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":-163.783,"capital-expenditure":13155.2,"net-present-cost":10745.9,"operational-emissions":-123340},"evacuated-tube-and-photovoltaic":{"pv-size":12,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":-173.046,"capital-expenditure":13265.2,"net-present-cost":10719.7,"operational-emissions":-133570},"photovoltaic-thermal-hybrid":{"pv-size":2,"solar-thermal-size":2,"thermal-energy-storage-volume":0.1,"operational-expenditure":60.3794,"capital-expenditure":11945.2,"net-present-cost":12833.3,"operational-emissions":172818}},"hydrogen-boiler":{"grey":{"operational-expenditure":228.534,"capital-expenditure":2120,"net-present-cost":5481.7,"operational-emissions":1.78163e+06},"blue":{"operational-expenditure":433.748,"capital-expenditure":2120,"net-present-cost":8500.37,"operational-emissions":279838},"green":{"operational-expenditure":858.169,"capital-expenditure":2120,"net-present-cost":14743.5,"operational-emissions":1.85392e+06}},"hydrogen-fuel-cell":{"grey":{"operational-expenditure":218.809,"capital-expenditure":25157.8,"net-present-cost":28376.5,"operational-emissions":1.70582e+06},"blue":{"operational-expenditure":415.291,"capital-expenditure":25157.8,"net-present-cost":31266.7,"operational-emissions":267930},"green":{"operational-expenditure":821.651,"capital-expenditure":25157.8,"net-present-cost":37244.2,"operational-emissions":1.77503e+06}},"gas-boiler":{"operational-expenditure":186.558,"capital-expenditure":1620,"net-present-cost":4364.25,"operational-emissions":853505},"biomass-boiler":{"operational-expenditure":191.689,"capital-expenditure":9750,"net-present-cost":12569.7,"operational-emissions":419757}}}');

console.log(j);
console.log(JSON.stringify(j));
document.getElementById("json-output").innerHTML = JSON.stringify(j, null, 2);

function draw_table(j) {
    const heat_options_json = ["electric-boiler", "air-source-heat-pump",
        "ground-source-heat-pump", "hydrogen-boiler", "hydrogen-fuel-cell",
        "biomass-boiler", "gas-boiler"];

    const heat_options_print = {
        "electric-boiler": "EleBo", "air-source-heat-pump": "ASHP",
        "ground-source-heat-pump": "GSHP", "hydrogen-boiler": "H2Bo", "hydrogen-fuel-cell": "H2FC",
        "biomass-boiler": "BioBo", "gas-boiler": "GasBo"
    };

    const solar_options_json = ["none", "photovoltaic",
        "flat-plate", "evacuated-tube", "flat-plate-and-photovoltaic",
        "evacuated-tube-and-photovoltaic", "photovoltaic-thermal-hybrid"];

    const solar_options_print = {
        "none": "None", "photovoltaic": "PV",
        "flat-plate": "FP", "evacuated-tube": "ET", "flat-plate-and-photovoltaic": "FP+PV",
        "evacuated-tube-and-photovoltaic": "ET+PV", "photovoltaic-thermal-hybrid": "PVT"
    };

    const hydrogen_options_json = ["grey", "blue", "green"];

    let table = document.getElementById('systems-table');
    while (document.getElementsByTagName('tr').length > 1) {
        table.removeChild(table.lastChild);
    }

    //console.log(table);
    systems = j["systems"];
    for (let heat_option of heat_options_json) {
        let system = systems[heat_option];
        //console.log(system, heat_option);
        switch (heat_option) {
            case "electric-boiler":
            case "air-source-heat-pump":
            case "ground-source-heat-pump":
                for (const [solar_option, value] of Object.entries(system)) {
                    let tr = document.createElement('tr');
                    make_cell(tr, heat_options_print[heat_option]);
                    make_cell(tr, solar_options_print[solar_option]);
                    make_cells(tr, value);
                    table.appendChild(tr);
                }
                break;
            case "hydrogen-boiler":
            case "hydrogen-fuel-cell":
                for (const [h2_type, value] of Object.entries(system)) {
                    let tr = document.createElement('tr');
                    make_cell(tr, heat_options_print[heat_option]);
                    make_cell(tr, h2_type[0].toUpperCase() + h2_type.substring(1));
                    make_cells(tr, value);
                    table.appendChild(tr);
                }
                break;
            default:
                let tr = document.createElement('tr');
                make_cell(tr, heat_options_print[heat_option]);
                make_cell(tr, undefined);
                make_cells(tr, system);
                table.appendChild(tr);
                break;
        }
    }
}

function draw_table2(j) {
    const heat_options_json = ["electric-boiler", "air-source-heat-pump",
        "ground-source-heat-pump", "hydrogen-boiler", "hydrogen-fuel-cell",
        "biomass-boiler", "gas-boiler"];

    const solar_options_json = ["none", "photovoltaic",
        "flat-plate", "evacuated-tube", "flat-plate-and-photovoltaic",
        "evacuated-tube-and-photovoltaic", "photovoltaic-thermal-hybrid"];

    const hydrogen_options_json = ["grey", "blue", "green"];

    let table = document.getElementById('systems-table');
    console.log(table);
    systems = j["systems"];
    for (const [heat_option, heat_system] of Object.entries(systems)) {
        for (const [solar_option, system] of Object.entries(heat_system)) {
            // And then create our markup:
            console.log(solar_option, getPosition(solar_option, '-', 3), solar_option.length);
            const markup = `
                <h2>${heat_option}</h2>
                <div class="flex-row">
                    <h1>${getPosition(solar_option, '-', 3) != solar_option.length ?
                    solar_option.substr(getPosition(solar_option, '-', 3) + 1) :
                    solar_option
                }</h1>
                    <p>${system["pv-size"]}</p>
                    <h2>m<sup>2</sup></h2>
                </div>
                <div class="flex-row">
                    <h1>${solar_option.substr(0, getPosition(solar_option, '-', 2))}</h1>
                    <p>${system["solar_thermal_size"]}</p>
                    <h2>m<sup>2</sup></h2>
                </div>
                <div class="flex-row">
                    <h1>Thermal Energy Storage</h1>
                    <p>${system["thermal-energy-storage-volume"]}</p>
                    <h2>m<sup>3</sup></h2>
                </div>
                <div class="flex-row">
                    <h1>CAPEX</h1>
                    <p>£${system["capital-expenditure"]}</p>
                </div>
                <div class="flex-row">
                    <h1>OPEX</h1>
                    <p>£${system["operational-expenditure"]}</p>
                    <h2>/yr</h2>
                </div>
                <div class="flex-row">
                    <h1>NPC</h1>
                    <p>£${system["net-present-cost"]}</p>
                </div>
                <div class="flex-row">
                    <h1>Emissions</h1>
                    <p>${system["operational-emissions"]}</p>
                    <h2>kgCO<sub>2</sub>e/yr</h2>
                </div>
            `;
            let div = document.createElement('div');
            div.classList.add("system-box");
            div.innerHTML = markup;
            document.body.append(div);
        }
    }
}

function getPosition(string, subString, index) {
    return string.split(subString, index).join(subString).length;
}

function make_cell(tr, value) {
    //console.log("row", tr);
    let td = document.createElement('td');
    td.innerHTML = value == undefined ? '-' : value;
    tr.appendChild(td);
}

function make_cells(tr, system) {
    const headers = ["pv-size", "solar-thermal-size",
        "thermal-energy-storage-volume", "operational-expenditure", "capital-expenditure",
        "net-present-cost", "operational-emissions"];

    for (let key of headers) {
        switch (key) {
            case "net-present-cost":
                const net_present_cost = calculate_net_present_cost(
                    system["operational-expenditure"],
                    system["capital-expenditure"],
                    cumulative_discount_rate);
                make_cell(tr, Math.round(net_present_cost));
                break;
            case "operational-expenditure":
                make_cell(tr, Math.round(system[key]));
                break;
            case "capital-expenditure":
                make_cell(tr, Math.round(system[key]));
                break;
            case "operational-emissions":
                make_cell(tr, Math.round(system[key] / 1000));
                break;
            default:
                make_cell(tr, system[key]);
        }
    }
    return tr;
}

const discount_rate = 1.035; // 3.5% standard for UK HMRC
let npc_years = 20;
let cumulative_discount_rate = calculate_cumulative_discount_rate(discount_rate, npc_years);

console.log("cumulative_discount_rate:", cumulative_discount_rate);
draw_table(j);
//clipboardJSON(j);
// SIMULATION FUNCTIONS ===========================================================

function clipboardJSON(j) {
    navigator.clipboard.writeText(JSON.stringify(j, null, 2));
}

function calculate_systems() {

}

function calculate_net_present_cost(opex, capex, cumulative_discount_rate) {
    return capex + opex * cumulative_discount_rate;
}

function calculate_cumulative_discount_rate(discount_rate, npc_years) {
    let discount_rate_current = 1;
    let cumulative_discount_rate = 0;
    for (let year = 0; year < npc_years; year++) {
        cumulative_discount_rate += 1.0 / discount_rate_current;
        discount_rate_current *= discount_rate;
    }
    return cumulative_discount_rate;
}

function updateLifetime() {
    npc_years = document.getElementById('sim-lifetime').value;
    cumulative_discount_rate = calculate_cumulative_discount_rate(discount_rate, npc_years);
    draw_table(j);
}