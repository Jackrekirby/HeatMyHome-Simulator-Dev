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
let j = {
    "demand": { "boiler": demand, "heat-pump": demand },
    "systems": {
        "electric-boiler": {
            "none": heat_solar_json,
            "photovoltaic": heat_solar_json,
            "flat-plate": heat_solar_json,
            "evacuated-tube": heat_solar_json,
            "flat-plate-and-photovoltaic": heat_solar_json,
            "evacuated-tube-and-photovoltaic": heat_solar_json,
            "photovoltaic-thermal-hybrid": heat_solar_json,
        },
        "air-source-heat-pump": {
            "none": heat_solar_json,
            "photovoltaic": heat_solar_json,
            "flat-plate": heat_solar_json,
            "evacuated-tube": heat_solar_json,
            "flat-plate-and-photovoltaic": heat_solar_json,
            "evacuated-tube-and-photovoltaic": heat_solar_json,
            "photovoltaic-thermal-hybrid": heat_solar_json,
        },
        "ground-source-heat-pump": {
            "none": heat_solar_json,
            "photovoltaic": heat_solar_json,
            "flat-plate": heat_solar_json,
            "evacuated-tube": heat_solar_json,
            "flat-plate-and-photovoltaic": heat_solar_json,
            "evacuated-tube-and-photovoltaic": heat_solar_json,
            "photovoltaic-thermal-hybrid": heat_solar_json,
        },
        "hydrogen-boiler": {
            "grey": heat_json,
            "blue": heat_json,
            "green": heat_json,
        },
        "hydrogen-fuel-cell": {
            "grey": heat_json,
            "blue": heat_json,
            "green": heat_json,
        },
        "biomass-boiler": heat_json,
        "gas-boiler": heat_json,
    }
};

console.log(j);
//console.log(JSON.stringify(j, null, 2));
document.getElementById("json-output").innerHTML = JSON.stringify(j, null, 2);

function draw_table(j) {
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
    for (let heat_option of heat_options_json) {
        let system = systems[heat_option];
        //console.log(system, heat_option);
        switch (heat_option) {
            case "electric-boiler":
            case "air-source-heat-pump":
            case "ground-source-heat-pump":
                for (const [solar_option, value] of Object.entries(system)) {
                    let tr = document.createElement('tr');
                    make_cell(tr, heat_option);
                    make_cell(tr, solar_option);
                    make_cells(tr, value);
                    table.appendChild(tr);
                }
                break;
            case "hydrogen-boiler":
            case "hydrogen-fuel-cell":
                for (const [h2_type, value] of Object.entries(system)) {
                    let tr = document.createElement('tr');
                    make_cell(tr, heat_option);
                    make_cell(tr, h2_type);
                    make_cells(tr, value);
                    table.appendChild(tr);
                }
                break;
            default:
                let tr = document.createElement('tr');
                make_cell(tr, heat_option);
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
        make_cell(tr, system[key]);
    }
    return tr;
}

draw_table(j);