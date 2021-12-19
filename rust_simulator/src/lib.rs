use wasm_bindgen::prelude::*;
extern crate web_sys;

// to build call: wasm-pack build --target web
// wasm-pack build --debug --target web
// A macro to provide `println!(..)`-style syntax for `console.log` logging.
macro_rules! println {
    ( $( $t:tt )* ) => {
        web_sys::console::log_1(&format!( $( $t )* ).into());
    }
}

mod heat_ninja;

#[wasm_bindgen]
pub fn run_simulation(
    thermostat_temperature: f32,
    latitude: f32,
    longitude: f32,
    num_occupants: u8,
    house_size: f32,
    postcode: String,
    epc_space_heating: f32,
    tes_volume_max: f32,
    //agile_tariff_per_hour_over_year: &[f32],
    //hourly_outside_temperatures_over_year: &[f32],
    //hourly_solar_irradiances_over_year: &[f32],
) -> String
{
    println!("{}, {}, {}, {}, {}, {}, {}, {}", thermostat_temperature, latitude, longitude, num_occupants, house_size, postcode, epc_space_heating, tes_volume_max);
    // heatninja::run_simulation(
    //     20.0,
    //     52.3833,
    //     -1.5833,
    //     2,
    //     60.0,
    //     String::from("CV4 7AL"),
    //     3000.0,
    //     0.5,
    //     agile_tariff_per_hour_over_year,
    //     hourly_outside_temperatures_over_year,
    //     hourly_solar_irradiances_over_year,
    // )
    let agile_tariff_per_hour_over_year: [f32; 8760] = [3.33; 8760];
    let hourly_outside_temperatures_over_year: [f32; 8760] = [3.33; 8760];
    let hourly_solar_irradiances_over_year: [f32; 8760] = [3.33; 8760];

    heat_ninja::run_simulation(
        thermostat_temperature,
        latitude,
        longitude,
        num_occupants,
        house_size,
        postcode,
        epc_space_heating,
        tes_volume_max,
        &agile_tariff_per_hour_over_year,
        &hourly_outside_temperatures_over_year,
        &hourly_solar_irradiances_over_year
    )
}