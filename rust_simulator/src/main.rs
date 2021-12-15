use std::time::Instant;
use std::fs::File;
use std::io::{prelude::*, BufReader};
use std::env;

mod heatninja;

fn import_file_data(assets_dir: String, latitude: f32, longitude: f32) -> ([f32; 8760], [f32; 8760], [f32; 8760]) {
    // wasm does not support Rust file io,
    // -> file data must be gathered beforehand then passed into wasm functions

    let path = env::current_dir().expect("Could not locate current directory");
    println!("The current directory is {}", path.display());

    let import_weather_data = |filepath: String| -> [f32; 8760] {
        println!("filepath: {}", filepath);
        let file = File::open(filepath).expect("cannot read file.");
        let reader = BufReader::new(file);
        let mut data: [f32; 8760] = [0.0; 8760];
        for (i, line) in reader.lines().enumerate() {
            data[i] = line.unwrap().parse::<f32>().expect("string invalid float.");
        }
        data
    };

    let build_weather_file_path = |datatype: &str| -> String {
        format!(
            "{}{}/lat_{:.1}_lon_{:.1}.csv",
            assets_dir,
            datatype,
            (latitude * 2.0).round() / 2.0,
            (longitude * 2.0).round() / 2.0
        )
    };

    let agile_tariff_per_hour_over_year: [f32; 8760] =
        import_weather_data(format!("{}agile_tariff.csv", assets_dir));

    let hourly_outside_temperatures_over_year: [f32; 8760] =
        import_weather_data(build_weather_file_path("outside_temps"));

    let hourly_solar_irradiances_over_year: [f32; 8760] =
        import_weather_data(build_weather_file_path("solar_irradiances"));

    (agile_tariff_per_hour_over_year, hourly_outside_temperatures_over_year, hourly_solar_irradiances_over_year)
}

fn main() {
    let now = Instant::now();
    let latitude: f32 = 52.3833;
    let longitude: f32 = -1.5833;
    let (agile_tariff_per_hour_over_year,
        hourly_outside_temperatures_over_year,
        hourly_solar_irradiances_over_year) = import_file_data(String::from("assets/"), latitude, longitude);

    heatninja::run_simulation(
        20.0,
        latitude,
        longitude,
        2,
        60.0,
        String::from("CV4 7AL"),
        3000.0,
        0.5,
        &agile_tariff_per_hour_over_year,
        &hourly_outside_temperatures_over_year,
        &hourly_solar_irradiances_over_year,
    );
    println!("{} ms", now.elapsed().as_millis());
}

// archive
// #[allow(dead_code)]
// fn create_binary_file_for_weather_data() {
//     const ASSETS_DIR: &str = "assets/";
//     let import_weather_data = |filepath: String| -> [f32; 8760] {
//         println!("filepath: {}", filepath);
//         let file = File::open(filepath).expect("cannot read file.");
//         let reader = BufReader::new(file);
//         let mut data: [f32; 8760] = [0.0; 8760];
//         for (i, line) in reader.lines().enumerate() {
//             data[i] = line.unwrap().parse::<f32>().expect("string invalid float.");
//         }
//         data
//     };
//
//     let data: [f32; 8760] = import_weather_data(format!("{}agile_tariff.csv", ASSETS_DIR));
//
//     let mut file = File::create("test.data").expect("could not create file");
//     for float in data {
//         file.write_all( & f32::to_le_bytes(float)).expect("could not write file");
//     }
// }