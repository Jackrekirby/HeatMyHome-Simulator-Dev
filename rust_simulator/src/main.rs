use std::env;
use std::fs::File;
use std::io::{prelude::*, BufReader};
use std::time::Instant;

mod heat_ninja;

fn import_file_data(
    assets_dir: String,
    latitude: f32,
    longitude: f32,
) -> ([f32; 8760], [f32; 8760], [f32; 8760]) {
    // wasm does not support Rust file io,
    // -> file data must be gathered beforehand then passed into wasm functions

    let import_weather_data = |filepath: String| -> [f32; 8760] {
        //println!("filepath: {}", filepath);
        let file = File::open(filepath).expect("cannot read file.");
        let reader = BufReader::new(file);
        let mut data: [f32; 8760] = [0.0; 8760];
        for (i, line) in reader.lines().enumerate() {
            data[i] = line.unwrap().parse::<f32>().expect("string invalid float.");
        }
        data
    };

    let format_coordinate = |coordinate: f32| -> f32 {
        let rounded: i16 = (coordinate * 2.0).round() as i16;
        if rounded == -0 {
            0.0
        } else {
            (rounded as f32) / 2.0
        }
    };

    let build_weather_file_path = |datatype: &str| -> String {
        format!(
            "{}{}/lat_{:.1}_lon_{:.1}.csv",
            assets_dir,
            datatype,
            format_coordinate(latitude),
            format_coordinate(longitude)
        )
    };

    let agile_tariff_per_hour_over_year: [f32; 8760] =
        import_weather_data(format!("{}agile_tariff.csv", assets_dir));

    let hourly_outside_temperatures_over_year: [f32; 8760] =
        import_weather_data(build_weather_file_path("outside_temps"));

    let hourly_solar_irradiances_over_year: [f32; 8760] =
        import_weather_data(build_weather_file_path("solar_irradiances"));

    (
        agile_tariff_per_hour_over_year,
        hourly_outside_temperatures_over_year,
        hourly_solar_irradiances_over_year,
    )
}

#[allow(dead_code)]
fn run_simulation() {
    let now = Instant::now();
    let latitude: f32 = 52.3833;
    let longitude: f32 = -1.5833;
    let (
        agile_tariff_per_hour_over_year,
        hourly_outside_temperatures_over_year,
        hourly_solar_irradiances_over_year,
    ) = import_file_data(String::from("assets/"), latitude, longitude);

    const CONFIG: heat_ninja::Config = heat_ninja::Config {
        print_intermediates: false,
        print_results: false,
        use_surface_optimisation: true,
        use_multithreading: true,
        file_index: 0,
        save_results_as_csv: true,
        save_results_as_json: false,
    };

    let json_results = heat_ninja::run_simulation(
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
        CONFIG,
    );

    println!("{}", json_results);
    println!("{} ms", now.elapsed().as_millis());
}

#[allow(dead_code)]
fn run_simulation_using_input_file() {
    let mut config: heat_ninja::Config = heat_ninja::Config {
        print_intermediates: false,
        print_results: false,
        use_surface_optimisation: false,
        use_multithreading: true,
        file_index: 0,
        save_results_as_csv: true,
        save_results_as_json: false,
    };

    println!("config: {:?}", config);
    // import input list file
    let filepath = "assets/input_list.csv";
    println!("filepath: {}", filepath);
    let file = File::open(filepath).expect("cannot read file.");
    let reader = BufReader::new(file);

    let all_now = Instant::now();

    for (i, line) in reader.lines().enumerate() {
        let just_now = Instant::now();
        if i == 0 {
            println!("{:?}", line.expect("could not get line"));
            continue;
        };
        let unwrapped_line = line.unwrap();
        let parts: Vec<&str> = unwrapped_line.split(',').collect();
        println!("{:?}", parts);

        let postcode = parts[0].parse().unwrap();
        let latitude: f32 = parts[1].parse().unwrap();
        let longitude: f32 = parts[2].parse().unwrap();
        let num_occupants: u8 = parts[3].parse().unwrap();
        let house_size: f32 = parts[4].parse().unwrap();
        let tes_volume_max: f32 = parts[6].parse().unwrap();
        let thermostat_temperature: f32 = parts[5].parse().unwrap();
        let epc_space_heating: f32 = parts[7].parse().unwrap();

        // import assets
        let (
            agile_tariff_per_hour_over_year,
            hourly_outside_temperatures_over_year,
            hourly_solar_irradiances_over_year,
        ) = import_file_data(String::from("assets/"), latitude, longitude);

        config.file_index = i;
        let _json_results = heat_ninja::run_simulation(
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
            &hourly_solar_irradiances_over_year,
            config,
        );
        println!(
            "i:{}, elapsed: {} ms, total {} s",
            i,
            just_now.elapsed().as_millis(),
            all_now.elapsed().as_secs()
        );
    }
}

fn main() {
    let path = env::current_dir().expect("Could not locate current directory");
    println!("The current directory is {}", path.display());
    run_simulation_using_input_file();
    //run_simulation();
}
