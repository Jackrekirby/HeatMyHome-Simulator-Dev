use std::time::Instant;
use std::fs::File;
use std::io::{prelude::*, BufReader};
use std::env;
// use std::path::Path;

mod heat_ninja;

fn import_file_data(assets_dir: String, latitude: f32, longitude: f32) -> ([f32; 8760], [f32; 8760], [f32; 8760]) {
    // wasm does not support Rust file io,
    // -> file data must be gathered beforehand then passed into wasm functions

    let path = env::current_dir().expect("Could not locate current directory");
    println!("The current directory is {}", path.display());

    // let root = Path::new("C:/dev/wasm_website/rust_simulator");
    // assert!(env::set_current_dir(&root).is_ok());
    // println!("Successfully changed working directory to {}!", root.display());

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

#[allow(dead_code)]
fn run_simulation() {
    let now = Instant::now();
    let latitude: f32 = 52.3833;
    let longitude: f32 = -1.5833;
    let (agile_tariff_per_hour_over_year,
        hourly_outside_temperatures_over_year,
        hourly_solar_irradiances_over_year) = import_file_data(String::from("assets/"), latitude, longitude);

    heat_ninja::run_simulation(
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

fn main() {
    run_simulation();
    //build_profile();
}

// #[allow(dead_code)]
// fn build_profile() {
//     let filepath = "src/heatninja.rs";
//     let file = File::open(filepath).expect("cannot read file.");
//     let reader = BufReader::new(file);
//
//     let mut ofile = File::create("src/heatninja_profile.rs").expect("could not open file");
//     ofile.write_all(b"use std::time::Instant;\nuse std::fs::File;\nuse std::io::{prelude::*};\n").expect("cannot write file");
//
//     for (i, result) in reader.lines().enumerate() {
//         let line = result.expect("cannot read line");
//         let key_start = "// profile start";
//         let key = "// profile";
//
//         if line.contains(key_start) {
//             ofile.write_all(
//                 line.replace(key_start,
//                              "let mut profile_file = File::create(\"profile/heatninja.txt\").expect(\"could not open file\");\n").as_ref()).
//                 expect("cannot write file");
//             ofile.write_all(line.replace(key_start, "let now = Instant::now();\n").as_ref()).expect("cannot write file");
//         } else if line.contains(key) {
//             //println!("{:?}", line.replace(key, "// THIS IS THE NEW THING!"));
//             //println!("{:?}", line.find(key).expect("could not find position"));
//             let pstring = format!("profile_file.write_all(format!(\"{},{{}}\n\", now.elapsed().as_micros()).as_ref()).expect(\"cannot write file\");\n", i);
//             ofile.write_all(line.replace(key,&pstring).as_ref()).expect("cannot write file");
//         } else {
//             ofile.write_all(format!("{}\n", line).as_ref()).expect("cannot write file");
//         }
//         //println!("i: {}|{:?}", i, line.expect("cannot read line"));
//     }
// }

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