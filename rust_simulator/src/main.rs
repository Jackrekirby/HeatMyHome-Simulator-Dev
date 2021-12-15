// wasm related includes
use wasm_bindgen::prelude::*;
extern crate web_sys;

use std::time::Instant;
mod heatninja;

fn main() {
    let now = Instant::now();
    heatninja::run_simulation(
        20.0,
        52.3833,
        -1.5833,
        2,
        60.0,
        String::from("CV4 7AL"),
        3000.0,
        0.5,
    );
    println!("{} ms", now.elapsed().as_millis());
}

// wasm related functions
#[wasm_bindgen]
pub fn run_simulation(thermostat_temperature: f32, latitude: f32, longitude: f32, num_occupants: u8, house_size: f32, postcode: String, epc_space_heating: f32, tes_volume_max: f32)
{
    heatninja::run_simulation( thermostat_temperature, latitude, longitude, num_occupants, house_size, postcode, epc_space_heating, tes_volume_max );
}