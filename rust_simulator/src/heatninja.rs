use arrayvec::ArrayVec;
use std::fs::File;
use std::io::{prelude::*, BufReader};
use std::env;

pub fn run_simulation(
    thermostat_temperature: f32,
    latitude: f32,
    longitude: f32,
    num_occupants: u8,
    house_size: f32,
    postcode: String,
    epc_space_heating: f32,
    tes_volume_max: f32,
) {
    // input arguments:
    /*
        thermostat_temperature
        latitude
        longitude
        num_occupants
        house_size
        postcode
        tes_volume_max
    */

    // initiate the following variables (VARIABLE -> REQUIRES):
    /*
    hourly_erh_thermostat_temperature -> thermostat_temperature
    hourly_hp_thermostat_temperature -> thermostat_temperature
    monthly_solar_declinations
    monthly_solar_height_factors -> monthly_solar_declinations
    monthly_hot_water_factors
    monthly_cold_water_temperatures -> latitude
    monthly_solar_gain_ratios_north -> monthly_solar_height_factors
    monthly_solar_gain_ratios_south -> monthly_solar_height_factors
    hourly_hot_water_ratios
    daily_average_hot_water_volume -> num_occupants
    hot_water_temperature
    solar_gain_house_factor -> house_size
    epc_body_gain -> house_size
    region_identifier -> postcode
    monthly_epc_outside_temperatures -> region_identifier
    monthly_epc_solar_irradiances -> region_identifier
    monthly_solar_gains_north -> monthly_epc_solar_irradiances, monthly_solar_gain_ratios_north, solar_gain_house_factor
    monthly_solar_gains_south -> monthly_epc_solar_irradiances, monthly_solar_gain_ratios_south, solar_gain_house_factor
    heat_capacity -> house_size
    body_heat_gain -> num_occupants
    thermal_transmittance, optimised_epc_demand ->
        house_size,
        epc_body_gain,
        monthly_epc_outside_temperatures,
        monthly_solar_gains_south,
        monthly_solar_gains_north,
        heat_capacity,
        epc_space_heating,
    demand = >
        hourly_thermostat_temperatures,
        thermostat_temperature,
        monthly_hot_water_factors,
        monthly_cold_water_temperatures,
        monthly_solar_gain_ratios_north,
        monthly_solar_gain_ratios_south,
        hourly_hot_water_ratios,
        hourly_outside_temperatures_over_year,
        hourly_solar_irradiances_over_year,
        daily_average_hot_water_volume,
        hot_water_temperature,
        solar_gain_house_factor,
        thermal_transmittance,
        house_size,
        heat_capacity,
        body_heat_gain
    */
    let path = env::current_dir().expect("Could not locate current directory");
    println!("The current directory is {}", path.display());
    const ASSETS_DIR: &str = "assets/";

    let hourly_erh_thermostat_temperatures: [f32; 24] = {
        let a = thermostat_temperature;
        let b = a - 2.0;
        [
            b, b, b, b, b, b, b, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, b, b,
        ]
    };

    let hourly_hp_thermostat_temperatures: [f32; 24] = [thermostat_temperature; 24];

    let monthly_solar_declinations: [f32; 12] = [
        -20.7, -12.8, -1.8, 9.8, 18.8, 23.1, 21.2, 13.7, 2.9, -8.7, -18.4, -23.0,
    ];

    let monthly_solar_height_factors = monthly_solar_declinations.map(|solar_declination| {
        ((std::f32::consts::PI / 180.0) * (latitude - solar_declination)).cos()
    });

    let monthly_hot_water_factors: [f32; 12] = [
        1.10, 1.06, 1.02, 0.98, 0.94, 0.90, 0.90, 0.94, 0.98, 1.02, 1.06, 1.10,
    ];

    let monthly_cold_water_temperatures: [f32; 12] = {
        match latitude {
            x if x < 52.2 => [
                12.1, 11.4, 12.3, 15.2, 16.1, 19.3, 21.2, 20.1, 19.5, 16.8, 13.7, 12.4,
            ], // South of England
            x if x < 53.3 => [
                12.9, 13.3, 14.4, 16.3, 17.7, 19.7, 21.8, 20.1, 20.3, 17.8, 15.3, 14.0,
            ], // Middle of England and Wales
            x if x < 54.95 => [
                9.6, 9.3, 10.7, 13.7, 15.3, 17.3, 19.3, 18.6, 17.9, 15.5, 12.3, 10.5,
            ], // North of England and Northern Ireland
            _ => [
                9.6, 9.2, 9.8, 13.2, 14.5, 16.8, 19.4, 18.5, 17.5, 15.1, 13.7, 12.4,
            ], // Scotland
        }
    };

    fn ax3bx2cxd(a: f32, b: f32, c: f32, d: f32, x: f32) -> f32 {
        let x2 = x * x;
        let x3 = x2 * x;
        a * x3 + b * x2 + c * x + d
    }

    fn ax2bxc(a: f32, b: f32, c: f32, x: f32) -> f32 {
        a * x * x + b * x + c
    }

    let monthly_solar_gain_ratios_north: [f32; 12] = {
        // Assume windows are vertical, so no in roof windows
        let pf_sg = (std::f32::consts::PI / 180.0 * 90.0 / 2.0).sin();
        let a = ax3bx2cxd(26.3, -38.5, 14.8, 0.0, pf_sg);
        let b = ax3bx2cxd(-16.5, 27.3, -11.9, 0.0, pf_sg);
        let c = ax3bx2cxd(-1.06, -0.0872, -0.191, 1.0, pf_sg);

        monthly_solar_height_factors.map(|solar_height_factor| ax2bxc(a, b, c, solar_height_factor))
    };

    let monthly_solar_gain_ratios_south: [f32; 12] = {
        // Assume windows are vertical, so no in roof windows
        let pf_sg = (std::f32::consts::PI / 180.0 * 90.0 / 2.0).sin();
        let a = ax3bx2cxd(-0.66, -0.106, 2.93, 0.0, pf_sg);
        let b = ax3bx2cxd(3.63, -0.374, -7.4, 0.0, pf_sg);
        let c = ax3bx2cxd(-2.71, -0.991, 4.59, 1.0, pf_sg);

        monthly_solar_height_factors.map(|solar_height_factor| ax2bxc(a, b, c, solar_height_factor))
    };

    let hourly_hot_water_ratios: [f32; 24] = [
        0.025, 0.018, 0.011, 0.010, 0.008, 0.013, 0.017, 0.044, 0.088, 0.075, 0.060, 0.056, 0.050,
        0.043, 0.036, 0.029, 0.030, 0.036, 0.053, 0.074, 0.071, 0.059, 0.050, 0.041,
    ];

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
            ASSETS_DIR,
            datatype,
            (latitude * 2.0).round() / 2.0,
            (longitude * 2.0).round() / 2.0
        )
    };

    let hourly_outside_temperatures_over_year: [f32; 8760] =
        import_weather_data(build_weather_file_path("outside_temps"));
    let hourly_solar_irradiances_over_year: [f32; 8760] =
        import_weather_data(build_weather_file_path("solar_irradiances"));

    let daily_average_hot_water_volume = {
        let num_occupants: f32 = num_occupants as f32;
        let showers_volume: f32 = (0.45 * num_occupants + 0.65) * 28.8; // Litres, 28.8 equivalent of Mixer with TES
        let bath_volume: f32 = (0.13 * num_occupants + 0.19) * 50.8; // Assumes shower is present
        let other_volume: f32 = 9.8 * num_occupants + 14.0;
        showers_volume + bath_volume + other_volume
    };

    let hot_water_temperature: f32 = 51.0;

    let solar_gain_house_factor: f32 = (house_size * 0.15 / 2.0) * 0.77 * 0.7 * 0.76 * 0.9 / 1000.0;

    let epc_body_gain: f32 = {
        let epc_num_occupants = 1.0
            + 1.76 * (1.0 - (-0.000349 * (house_size - 13.9).powi(2)).exp())
            + 0.0013 * (house_size - 13.9);
        (epc_num_occupants * 60.0) / 1000.0
    };

    let region_identifier: u8 = {
        #[derive(Debug)]
        struct Region {
            postcode: String,
            minimum: u8,
            maximum: u8,
            id: u8,
        }

        impl Region {
            fn new(postcode: String, minimum: u8, maximum: u8, id: u8) -> Region {
                Region {
                    postcode,
                    minimum,
                    maximum,
                    id,
                }
            }
        }

        let regions: [Region; 171] = [
            Region::new(String::from("ZE"), 0, 0, 20),
            Region::new(String::from("YO25"), 0, 0, 11),
            Region::new(String::from("ZE"), 0, 0, 20),
            Region::new(String::from("YO25"), 0, 0, 11),
            Region::new(String::from("YO"), 15, 16, 11),
            Region::new(String::from("YO"), 0, 0, 10),
            Region::new(String::from("WV"), 0, 0, 6),
            Region::new(String::from("WS"), 0, 0, 6),
            Region::new(String::from("WR"), 0, 0, 6),
            Region::new(String::from("WN"), 0, 0, 7),
            Region::new(String::from("WF"), 0, 0, 11),
            Region::new(String::from("WD"), 0, 0, 1),
            Region::new(String::from("WC"), 0, 0, 1),
            Region::new(String::from("WA"), 0, 0, 7),
            Region::new(String::from("W"), 0, 0, 1),
            Region::new(String::from("UB"), 0, 0, 1),
            Region::new(String::from("TW"), 0, 0, 1),
            Region::new(String::from("TS"), 0, 0, 10),
            Region::new(String::from("TR"), 0, 0, 4),
            Region::new(String::from("TQ"), 0, 0, 4),
            Region::new(String::from("TN"), 0, 0, 2),
            Region::new(String::from("TF"), 0, 0, 6),
            Region::new(String::from("TD15"), 0, 0, 9),
            Region::new(String::from("TD12"), 0, 0, 9),
            Region::new(String::from("TD"), 0, 0, 9),
            Region::new(String::from("TA"), 0, 0, 5),
            Region::new(String::from("SY"), 15, 25, 13),
            Region::new(String::from("SY14"), 0, 0, 7),
            Region::new(String::from("SY"), 0, 0, 6),
            Region::new(String::from("SW"), 0, 0, 1),
            Region::new(String::from("ST"), 0, 0, 6),
            Region::new(String::from("SS"), 0, 0, 12),
            Region::new(String::from("SR"), 7, 8, 10),
            Region::new(String::from("SR"), 0, 0, 9),
            Region::new(String::from("SP"), 6, 11, 3),
            Region::new(String::from("SP"), 0, 0, 5),
            Region::new(String::from("SO"), 0, 0, 3),
            Region::new(String::from("SN7"), 0, 0, 1),
            Region::new(String::from("SN"), 0, 0, 5),
            Region::new(String::from("SM"), 0, 0, 1),
            Region::new(String::from("SL"), 0, 0, 1),
            Region::new(String::from("SK"), 22, 23, 6),
            Region::new(String::from("SK17"), 0, 0, 6),
            Region::new(String::from("SK13"), 0, 0, 6),
            Region::new(String::from("SK"), 0, 0, 7),
            Region::new(String::from("SG"), 0, 0, 1),
            Region::new(String::from("SE"), 0, 0, 1),
            Region::new(String::from("SA"), 61, 73, 13),
            Region::new(String::from("SA"), 31, 48, 13),
            Region::new(String::from("SA"), 14, 20, 13),
            Region::new(String::from("SA"), 0, 0, 5),
            Region::new(String::from("S"), 40, 45, 6),
            Region::new(String::from("S"), 32, 33, 6),
            Region::new(String::from("S18"), 0, 0, 6),
            Region::new(String::from("S"), 0, 0, 11),
            Region::new(String::from("RM"), 0, 0, 12),
            Region::new(String::from("RH"), 10, 20, 2),
            Region::new(String::from("RH"), 0, 0, 1),
            Region::new(String::from("RG"), 21, 29, 3),
            Region::new(String::from("RG"), 0, 0, 1),
            Region::new(String::from("PR"), 0, 0, 7),
            Region::new(String::from("PO"), 18, 22, 2),
            Region::new(String::from("PO"), 0, 0, 3),
            Region::new(String::from("PL"), 0, 0, 4),
            Region::new(String::from("PH50"), 0, 0, 14),
            Region::new(String::from("PH49"), 0, 0, 14),
            Region::new(String::from("PH"), 30, 44, 17),
            Region::new(String::from("PH26"), 0, 0, 16),
            Region::new(String::from("PH"), 19, 25, 17),
            Region::new(String::from("PH"), 0, 0, 15),
            Region::new(String::from("PE"), 20, 25, 11),
            Region::new(String::from("PE"), 9, 12, 11),
            Region::new(String::from("PE"), 0, 0, 12),
            Region::new(String::from("PA"), 0, 0, 14),
            Region::new(String::from("OX"), 0, 0, 1),
            Region::new(String::from("OL"), 0, 0, 7),
            Region::new(String::from("NW"), 0, 0, 1),
            Region::new(String::from("NR"), 0, 0, 12),
            Region::new(String::from("NP8"), 0, 0, 13),
            Region::new(String::from("NP"), 0, 0, 5),
            Region::new(String::from("NN"), 0, 0, 6),
            Region::new(String::from("NG"), 0, 0, 11),
            Region::new(String::from("NE"), 0, 0, 9),
            Region::new(String::from("N"), 0, 0, 1),
            Region::new(String::from("ML"), 0, 0, 14),
            Region::new(String::from("MK"), 0, 0, 1),
            Region::new(String::from("ME"), 0, 0, 2),
            Region::new(String::from("M"), 0, 0, 7),
            Region::new(String::from("LU"), 0, 0, 1),
            Region::new(String::from("LS24"), 0, 0, 10),
            Region::new(String::from("LS"), 0, 0, 11),
            Region::new(String::from("LN"), 0, 0, 11),
            Region::new(String::from("LL"), 30, 78, 13),
            Region::new(String::from("LL"), 23, 27, 13),
            Region::new(String::from("LL"), 0, 0, 7),
            Region::new(String::from("LE"), 0, 0, 6),
            Region::new(String::from("LD"), 0, 0, 13),
            Region::new(String::from("LA"), 7, 23, 8),
            Region::new(String::from("LA"), 0, 0, 7),
            Region::new(String::from("L"), 0, 0, 7),
            Region::new(String::from("KY"), 0, 0, 15),
            Region::new(String::from("KW"), 15, 17, 19),
            Region::new(String::from("KW"), 0, 0, 17),
            Region::new(String::from("KT"), 0, 0, 1),
            Region::new(String::from("KA"), 0, 0, 14),
            Region::new(String::from("IV36"), 0, 0, 16),
            Region::new(String::from("IV"), 30, 32, 16),
            Region::new(String::from("IV"), 0, 0, 17),
            Region::new(String::from("IP"), 0, 0, 12),
            Region::new(String::from("IG"), 0, 0, 12),
            Region::new(String::from("HX"), 0, 0, 11),
            Region::new(String::from("HU"), 0, 0, 11),
            Region::new(String::from("HS"), 0, 0, 18),
            Region::new(String::from("HR"), 0, 0, 6),
            Region::new(String::from("HP"), 0, 0, 1),
            Region::new(String::from("HG"), 0, 0, 10),
            Region::new(String::from("HD"), 0, 0, 11),
            Region::new(String::from("HA"), 0, 0, 1),
            Region::new(String::from("GU"), 51, 52, 3),
            Region::new(String::from("GU46"), 0, 0, 3),
            Region::new(String::from("GU"), 30, 35, 3),
            Region::new(String::from("GU"), 28, 29, 2),
            Region::new(String::from("GU14"), 0, 0, 3),
            Region::new(String::from("GU"), 11, 12, 3),
            Region::new(String::from("GU"), 0, 0, 1),
            Region::new(String::from("GL"), 0, 0, 5),
            Region::new(String::from("G"), 0, 0, 14),
            Region::new(String::from("FY"), 0, 0, 7),
            Region::new(String::from("FK"), 0, 0, 14),
            Region::new(String::from("EX"), 0, 0, 4),
            Region::new(String::from("EN9"), 0, 0, 12),
            Region::new(String::from("EN"), 0, 0, 1),
            Region::new(String::from("EH"), 43, 46, 9),
            Region::new(String::from("EH"), 0, 0, 15),
            Region::new(String::from("EC"), 0, 0, 1),
            Region::new(String::from("E"), 0, 0, 1),
            Region::new(String::from("DY"), 0, 0, 6),
            Region::new(String::from("DT"), 0, 0, 3),
            Region::new(String::from("DN"), 0, 0, 11),
            Region::new(String::from("DL"), 0, 0, 10),
            Region::new(String::from("DH"), 4, 5, 9),
            Region::new(String::from("DH"), 0, 0, 10),
            Region::new(String::from("DG"), 0, 0, 8),
            Region::new(String::from("DE"), 0, 0, 6),
            Region::new(String::from("DD"), 0, 0, 15),
            Region::new(String::from("DA"), 0, 0, 2),
            Region::new(String::from("CW"), 0, 0, 7),
            Region::new(String::from("CV"), 0, 0, 6),
            Region::new(String::from("CT"), 0, 0, 2),
            Region::new(String::from("CR"), 0, 0, 1),
            Region::new(String::from("CO"), 0, 0, 12),
            Region::new(String::from("CM"), 21, 23, 1),
            Region::new(String::from("CM"), 0, 0, 12),
            Region::new(String::from("CH"), 5, 8, 7),
            Region::new(String::from("CH"), 0, 0, 7),
            Region::new(String::from("CF"), 0, 0, 5),
            Region::new(String::from("CB"), 0, 0, 12),
            Region::new(String::from("CA"), 0, 0, 8),
            Region::new(String::from("BT"), 0, 0, 21),
            Region::new(String::from("BS"), 0, 0, 5),
            Region::new(String::from("BR"), 0, 0, 2),
            Region::new(String::from("BN"), 0, 0, 2),
            Region::new(String::from("BL"), 0, 0, 7),
            Region::new(String::from("BH"), 0, 0, 3),
            Region::new(String::from("BD"), 23, 24, 10),
            Region::new(String::from("BD"), 0, 0, 11),
            Region::new(String::from("BB"), 0, 0, 7),
            Region::new(String::from("BA"), 0, 0, 5),
            Region::new(String::from("B"), 0, 0, 6),
            Region::new(String::from("AL"), 0, 0, 1),
            Region::new(String::from("AB"), 0, 0, 16),
        ];

        let mut digits: String = String::from("");
        for c in postcode.chars() {
            if c.is_digit(10) {
                digits.push(c);
                if digits.len() > 1 {
                    break;
                };
            } else if digits.len() > 0 {
                break;
            };
        }

        let digits: u8 = digits.parse::<u8>().expect("Expected a number");

        //dbg!(digits);
        let mut region_id: u8 = 0;
        for region in regions {
            if region.postcode == &postcode[0..region.postcode.len()] {
                if region.maximum == 0 || (digits <= region.maximum && digits >= region.minimum) {
                    region_id = region.id;
                    break;
                }
            }
        }
        region_id
    };

    if region_identifier == 0 {
        panic!("postcode did not match any in database");
    }

    let monthly_epc_outside_temperatures: [f32; 12] = match region_identifier {
        1 => [
            5.1, 5.6, 7.4, 9.9, 13.0, 16.0, 17.9, 17.8, 15.2, 11.6, 8.0, 5.1,
        ],
        2 => [
            5.0, 5.4, 7.1, 9.5, 12.6, 15.4, 17.4, 17.5, 15.0, 11.7, 8.1, 5.2,
        ],
        3 => [
            5.4, 5.7, 7.3, 9.6, 12.6, 15.4, 17.3, 17.3, 15.0, 11.8, 8.4, 5.5,
        ],
        4 => [
            6.1, 6.4, 7.5, 9.3, 11.9, 14.5, 16.2, 16.3, 14.6, 11.8, 9.0, 6.4,
        ],
        5 => [
            4.9, 5.3, 7.0, 9.3, 12.2, 15.0, 16.7, 16.7, 14.4, 11.1, 7.8, 4.9,
        ],
        6 => [
            4.3, 4.8, 6.6, 9.0, 11.8, 14.8, 16.6, 16.5, 14.0, 10.5, 7.1, 4.2,
        ],
        7 => [
            4.7, 5.2, 6.7, 9.1, 12.0, 14.7, 16.4, 16.3, 14.1, 10.7, 7.5, 4.6,
        ],
        8 => [
            3.9, 4.3, 5.6, 7.9, 10.7, 13.2, 14.9, 14.8, 12.8, 9.7, 6.6, 3.7,
        ],
        9 => [
            4.0, 4.5, 5.8, 7.9, 10.4, 13.3, 15.2, 15.1, 13.1, 9.7, 6.6, 3.7,
        ],
        10 => [
            4.0, 4.6, 6.1, 8.3, 10.9, 13.8, 15.8, 15.6, 13.5, 10.1, 6.7, 3.8,
        ],
        11 => [
            4.3, 4.9, 6.5, 8.9, 11.7, 14.6, 16.6, 16.4, 14.1, 10.6, 7.1, 4.2,
        ],
        12 => [
            4.7, 5.2, 7.0, 9.5, 12.5, 15.4, 17.6, 17.6, 15.0, 11.4, 7.7, 4.7,
        ],
        13 => [
            5.0, 5.3, 6.5, 8.5, 11.2, 13.7, 15.3, 15.3, 13.5, 10.7, 7.8, 5.2,
        ],
        14 => [
            4.0, 4.4, 5.6, 7.9, 10.4, 13.0, 14.5, 14.4, 12.5, 9.3, 6.5, 3.8,
        ],
        15 => [
            3.6, 4.0, 5.4, 7.7, 10.1, 12.9, 14.6, 14.5, 12.5, 9.2, 6.1, 3.2,
        ],
        16 => [
            3.3, 3.6, 5.0, 7.1, 9.3, 12.2, 14.0, 13.9, 12.0, 8.8, 5.7, 2.9,
        ],
        17 => [
            3.1, 3.2, 4.4, 6.6, 8.9, 11.4, 13.2, 13.1, 11.3, 8.2, 5.4, 2.7,
        ],
        18 => [
            5.2, 5.0, 5.8, 7.6, 9.7, 11.8, 13.4, 13.6, 12.1, 9.6, 7.3, 5.2,
        ],
        19 => [
            4.4, 4.2, 5.0, 7.0, 8.9, 11.2, 13.1, 13.2, 11.7, 9.1, 6.6, 4.3,
        ],
        20 => [
            4.6, 4.1, 4.7, 6.5, 8.3, 10.5, 12.4, 12.8, 11.4, 8.8, 6.5, 4.6,
        ],
        21 => [
            4.8, 5.2, 6.4, 8.4, 10.9, 13.5, 15.0, 14.9, 13.1, 10.0, 7.2, 4.7,
        ],
        other => panic!(
            "no region_identifier ({}) matches for monthly_epc_outside_temperatures.",
            other
        ),
    };

    let monthly_epc_solar_irradiances: [u16; 12] = match region_identifier {
        1 => [30, 56, 98, 157, 195, 217, 203, 173, 127, 73, 39, 24],
        2 => [32, 59, 104, 170, 208, 231, 216, 182, 133, 77, 41, 25],
        3 => [35, 62, 109, 172, 209, 235, 217, 185, 138, 80, 44, 27],
        4 => [36, 63, 111, 174, 210, 233, 204, 182, 136, 78, 44, 28],
        5 => [32, 59, 105, 167, 201, 226, 206, 175, 130, 74, 40, 25],
        6 => [28, 55, 97, 153, 191, 208, 194, 163, 121, 69, 35, 23],
        7 => [24, 51, 95, 152, 191, 203, 186, 152, 115, 65, 31, 20],
        8 => [23, 51, 95, 157, 200, 203, 194, 156, 113, 62, 30, 19],
        9 => [23, 50, 92, 151, 200, 196, 187, 153, 11, 61, 30, 18],
        10 => [25, 51, 95, 152, 196, 198, 190, 156, 115, 64, 32, 20],
        11 => [26, 54, 96, 150, 192, 200, 189, 157, 115, 66, 33, 21],
        12 => [30, 58, 101, 165, 203, 220, 206, 173, 128, 74, 39, 24],
        13 => [29, 57, 104, 164, 205, 220, 199, 167, 120, 68, 35, 22],
        14 => [19, 46, 88, 148, 196, 193, 185, 150, 101, 55, 25, 15],
        15 => [21, 46, 89, 146, 198, 191, 183, 150, 106, 57, 27, 15],
        16 => [19, 45, 89, 143, 194, 188, 177, 144, 101, 54, 25, 14],
        17 => [17, 43, 85, 145, 189, 185, 170, 139, 98, 51, 22, 12],
        18 => [16, 41, 87, 155, 205, 206, 185, 148, 101, 51, 21, 11],
        19 => [14, 39, 84, 143, 205, 201, 178, 145, 100, 50, 19, 9],
        20 => [12, 34, 79, 135, 196, 190, 168, 144, 90, 46, 16, 7],
        21 => [24, 52, 96, 155, 201, 198, 183, 150, 107, 61, 30, 18],
        other => panic!(
            "no region_identifier ({}) matches for monthly_epc_solar_irradiances.",
            other
        ),
    };

    let monthly_solar_gains_north: ArrayVec<f32, 12> = {
        monthly_epc_solar_irradiances
            .iter()
            .zip(&monthly_solar_gain_ratios_north)
            .map(|(a, b)| (*a as f32) * b * solar_gain_house_factor)
            .collect()
    };

    let monthly_solar_gains_south: ArrayVec<f32, 12> = {
        monthly_epc_solar_irradiances
            .iter()
            .zip(&monthly_solar_gain_ratios_south)
            .map(|(a, b)| (*a as f32) * b * solar_gain_house_factor)
            .collect()
    };

    let heat_capacity: f32 = (250.0 * house_size) / 3600.0;
    let body_heat_gain: f32 = ((num_occupants as f32) * 60.0) / 1000.0;

    const DAYS_IN_MONTHS: [u8; 12] = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];

    let (thermal_transmittance, optimised_epc_demand) = {
        let mut optimised_thermal_transmittance: f32 = 0.5;
        let mut optimised_epc_demand: f32 = 0.0;

        let hourly_temperature_profile_summer: [f32; 24] = [
            7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0,
            7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0,
        ];
        let hourly_temperature_profile_weekend: [f32; 24] = [
            7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0,
            20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0,
        ];
        let hourly_temperature_profile_other: [f32; 24] = [
            7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 20.0, 20.0, 20.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0,
            20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0,
        ];

        let thermal_transmittance_min: f32 = 0.5;
        let thermal_transmittance_max: f32 = 3.0;
        let thermal_transmittance_step: f32 = 0.01;
        let thermal_transmittance_steps: u16 = ((thermal_transmittance_max
            - thermal_transmittance_min
            + (thermal_transmittance_step / 10.0))
            / thermal_transmittance_step) as u16; // add step/10 to avoid floating point error

        for i in 0..thermal_transmittance_steps {
            let thermal_transmittance: f32 =
                thermal_transmittance_min + thermal_transmittance_step * (i as f32);
            let mut inside_temperature: f32 = 20.0;
            let mut epc_demand: f32 = 0.0;
            for (month, days_in_month) in DAYS_IN_MONTHS.iter().enumerate() {
                let outside_temperature = monthly_epc_outside_temperatures[month];
                let solar_gain_south = monthly_solar_gains_south[month];
                let solar_gain_north = monthly_solar_gains_north[month];
                for day in 0..*days_in_month {
                    let hourly_temperature_profile: &[f32; 24] = {
                        if month >= 5 && month <= 8 {
                            // summer no heating
                            &hourly_temperature_profile_summer
                        } else if day % 7 >= 5 {
                            // weekend not summer
                            &hourly_temperature_profile_weekend
                        } else {
                            // weekday not summer
                            &hourly_temperature_profile_other
                        }
                    };
                    for hour in 0..24 {
                        let desired_temperature: f32 = hourly_temperature_profile[hour];
                        let heat_flow_out: f32 = (house_size
                            * thermal_transmittance
                            * (inside_temperature - outside_temperature))
                            / 1000.0;

                        // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
                        inside_temperature +=
                            (-heat_flow_out + solar_gain_south + solar_gain_north + epc_body_gain)
                                / heat_capacity;
                        //dbg!(inside_temperature, desired_temperature);
                        if inside_temperature < desired_temperature {
                            //  Requires heating
                            let space_hr_demand =
                                (desired_temperature - inside_temperature) * heat_capacity;
                            inside_temperature = desired_temperature;
                            epc_demand += space_hr_demand / 0.9;
                        }
                    }
                }
            }
            //dbg!(epc_space_heating, optimised_epc_demand, epc_demand);
            let epc_optimal_heating_demand_diff = (epc_space_heating - optimised_epc_demand).abs();
            let epc_heating_demand_diff = (epc_space_heating - epc_demand).abs();
            if epc_heating_demand_diff < epc_optimal_heating_demand_diff {
                optimised_epc_demand = epc_demand;
                optimised_thermal_transmittance = thermal_transmittance;
            } else {
                // if the epc heating demand difference is increasing the most optimal has already been found
                break;
            }
        }
        (optimised_thermal_transmittance, optimised_epc_demand)
    };

    let calculate_demand = |hourly_thermostat_temperatures: [f32; 24]| -> (f32, f32, f32, f32) {
        let mut hour_year_counter: usize = 0;
        let mut max_hourly_demand: f32 = 0.0;
        let mut total_demand: f32 = 0.0;
        let mut hot_water_demand: f32 = 0.0;
        let mut inside_temperature: f32 = thermostat_temperature;

        for (month, days_in_month) in DAYS_IN_MONTHS.iter().enumerate() {
            let monthly_hot_water_factor = monthly_hot_water_factors[month];
            let cold_water_temperature = monthly_cold_water_temperatures[month];
            let ratio_solar_gain_south = monthly_solar_gain_ratios_south[month];
            let ratio_solar_gain_north = monthly_solar_gain_ratios_north[month];
            for _day in 0..*days_in_month {
                for hour in 0..24 {
                    let current_thermostat_temperature: f32 = hourly_thermostat_temperatures[hour];
                    let hourly_hot_water_ratio: f32 = hourly_hot_water_ratios[hour];
                    let outside_temp_current: f32 =
                        hourly_outside_temperatures_over_year[hour_year_counter];
                    let solar_irradiance_current: f32 =
                        hourly_solar_irradiances_over_year[hour_year_counter];

                    let hot_water_hour_demand: f32 = (daily_average_hot_water_volume
                        * 4.18
                        * (hot_water_temperature - cold_water_temperature)
                        / 3600.0)
                        * monthly_hot_water_factor
                        * hourly_hot_water_ratio;

                    let heat_loss: f32 = (house_size
                        * thermal_transmittance
                        * (inside_temperature - outside_temp_current))
                        / 1000.0;

                    let solar_gain_north: f32 =
                        solar_irradiance_current * ratio_solar_gain_north * solar_gain_house_factor;
                    let solar_gain_south: f32 =
                        solar_irradiance_current * ratio_solar_gain_south * solar_gain_house_factor;
                    // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
                    inside_temperature +=
                        (-heat_loss + solar_gain_south + solar_gain_north + body_heat_gain)
                            / heat_capacity;
                    let space_hour_demand: f32 = {
                        if inside_temperature < current_thermostat_temperature {
                            //  Requires heating
                            let space_hour_demand: f32 = (current_thermostat_temperature
                                - inside_temperature)
                                * heat_capacity;
                            inside_temperature = current_thermostat_temperature;
                            space_hour_demand
                        } else {
                            0.0
                        }
                    };
                    //std::cout << hour << ' ' << space_hr_demand << ' ' << demand_total - dhw_hr_demand << '\n';
                    let hourly_demand: f32 = hot_water_hour_demand + space_hour_demand;
                    max_hourly_demand = if max_hourly_demand > hourly_demand {
                        max_hourly_demand
                    } else {
                        hourly_demand
                    };
                    total_demand += hourly_demand;
                    hot_water_demand += hot_water_hour_demand;
                    hour_year_counter += 1;
                }
            }
        }

        let space_demand: f32 = total_demand - hot_water_demand;
        (
            total_demand,
            space_demand,
            hot_water_demand,
            max_hourly_demand,
        )
    };

    let (erh_total_demand, erh_space_demand, erh_hot_water_demand, erh_max_hourly_demand) =
        calculate_demand(hourly_erh_thermostat_temperatures);

    let (hp_total_demand, hp_space_demand, hp_hot_water_demand, hp_max_hourly_demand) =
        calculate_demand(hourly_hp_thermostat_temperatures);

    let discount_rate: f32 = 1.035; // 3.5% standard for UK HMRC
    let npc_years: u8 = 20;
    let coldest_outside_temperature_of_year: f32 = {
        use std::collections::HashMap;
        let coldest_outside_temperatures_of_year_per_region: HashMap<String, f32> =
            HashMap::from([
                (String::from("50.0_-3.5"), 4.61),
                (String::from("50.0_-4.0"), 4.554),
                (String::from("50.0_-4.5"), 4.406),
                (String::from("50.0_-5.0"), 4.017),
                (String::from("50.0_-5.5"), 4.492),
                (String::from("50.5_-0.5"), 3.02),
                (String::from("50.5_-1.0"), 3.188),
                (String::from("50.5_-1.5"), 2.812),
                (String::from("50.5_-2.0"), 2.583),
                (String::from("50.5_-2.5"), 2.774),
                (String::from("50.5_-3.0"), 2.697),
                (String::from("50.5_-3.5"), 1.744),
                (String::from("50.5_-4.0"), 0.854),
                (String::from("50.5_-4.5"), 1.27),
                (String::from("50.5_-5.0"), 2.708),
                (String::from("50.5_0.0"), 2.886),
                (String::from("50.5_0.5"), 2.764),
                (String::from("51.0_-0.5"), -3.846),
                (String::from("51.0_-1.0"), -4.285),
                (String::from("51.0_-1.5"), -4.421),
                (String::from("51.0_-2.0"), -4.274),
                (String::from("51.0_-2.5"), -3.764),
                (String::from("51.0_-3.0"), -2.635),
                (String::from("51.0_-3.5"), -1.712),
                (String::from("51.0_-4.0"), -0.232),
                (String::from("51.0_-4.5"), 1.638),
                (String::from("51.0_0.0"), -3.344),
                (String::from("51.0_0.5"), -2.101),
                (String::from("51.0_1.0"), 0.307),
                (String::from("51.0_1.5"), 1.271),
                (String::from("51.5_-0.5"), -5.969),
                (String::from("51.5_-1.0"), -5.673),
                (String::from("51.5_-1.5"), -5.09),
                (String::from("51.5_-2.0"), -4.292),
                (String::from("51.5_-2.5"), -3.039),
                (String::from("51.5_-3.0"), -1.591),
                (String::from("51.5_-3.5"), 0.221),
                (String::from("51.5_-4.0"), 1.249),
                (String::from("51.5_-4.5"), 2.001),
                (String::from("51.5_-5.0"), 2.948),
                (String::from("51.5_0.0"), -5.628),
                (String::from("51.5_0.5"), -4.165),
                (String::from("51.5_1.0"), -1.369),
                (String::from("51.5_1.5"), 1.813),
                (String::from("52.0_-0.5"), -5.601),
                (String::from("52.0_-1.0"), -5.283),
                (String::from("52.0_-1.5"), -4.854),
                (String::from("52.0_-2.0"), -4.37),
                (String::from("52.0_-2.5"), -3.7),
                (String::from("52.0_-3.0"), -3.597),
                (String::from("52.0_-3.5"), -3.13),
                (String::from("52.0_-4.0"), -2.297),
                (String::from("52.0_-4.5"), -0.642),
                (String::from("52.0_-5.0"), 2.044),
                (String::from("52.0_-5.5"), 3.622),
                (String::from("52.0_0.0"), -5.439),
                (String::from("52.0_0.5"), -4.533),
                (String::from("52.0_1.0"), -2.836),
                (String::from("52.0_1.5"), 0.146),
                (String::from("52.5_-0.5"), -4.979),
                (String::from("52.5_-1.0"), -4.814),
                (String::from("52.5_-1.5"), -4.451),
                (String::from("52.5_-2.0"), -3.991),
                (String::from("52.5_-2.5"), -3.603),
                (String::from("52.5_-3.0"), -3.359),
                (String::from("52.5_-3.5"), -3.007),
                (String::from("52.5_-4.0"), -0.479),
                (String::from("52.5_-4.5"), 2.769),
                (String::from("52.5_0.0"), -4.845),
                (String::from("52.5_0.5"), -4.0),
                (String::from("52.5_1.0"), -3.96),
                (String::from("52.5_1.5"), -1.778),
                (String::from("52.5_2.0"), 1.576),
                (String::from("53.0_-0.5"), -4.434),
                (String::from("53.0_-1.0"), -4.51),
                (String::from("53.0_-1.5"), -4.234),
                (String::from("53.0_-2.0"), -3.806),
                (String::from("53.0_-2.5"), -3.409),
                (String::from("53.0_-3.0"), -2.964),
                (String::from("53.0_-3.5"), -2.419),
                (String::from("53.0_-4.0"), -0.304),
                (String::from("53.0_-4.5"), 1.987),
                (String::from("53.0_-5.0"), 3.827),
                (String::from("53.0_0.0"), -4.07),
                (String::from("53.0_0.5"), -1.754),
                (String::from("53.0_1.0"), 0.277),
                (String::from("53.0_1.5"), 1.709),
                (String::from("53.0_2.0"), 2.397),
                (String::from("53.5_-0.5"), -4.156),
                (String::from("53.5_-1.0"), -4.141),
                (String::from("53.5_-1.5"), -3.834),
                (String::from("53.5_-2.0"), -3.492),
                (String::from("53.5_-2.5"), -2.729),
                (String::from("53.5_-3.0"), -1.344),
                (String::from("53.5_-3.5"), 0.446),
                (String::from("53.5_-4.0"), 1.524),
                (String::from("53.5_-4.5"), 2.578),
                (String::from("53.5_0.0"), -2.173),
                (String::from("53.5_0.5"), 1.351),
                (String::from("54.0_-0.5"), -2.622),
                (String::from("54.0_-1.0"), -3.424),
                (String::from("54.0_-1.5"), -3.834),
                (String::from("54.0_-2.0"), -3.837),
                (String::from("54.0_-2.5"), -2.766),
                (String::from("54.0_-3.0"), -0.56),
                (String::from("54.0_-3.5"), 1.22),
                (String::from("54.0_-5.5"), 3.297),
                (String::from("54.0_-6.0"), 1.151),
                (String::from("54.0_-6.5"), -1.496),
                (String::from("54.0_-7.0"), -3.164),
                (String::from("54.0_-7.5"), -3.294),
                (String::from("54.0_-8.0"), -2.848),
                (String::from("54.0_0.0"), 0.231),
                (String::from("54.5_-0.5"), 0.579),
                (String::from("54.5_-1.0"), -1.903),
                (String::from("54.5_-1.5"), -4.414),
                (String::from("54.5_-2.0"), -5.579),
                (String::from("54.5_-2.5"), -5.161),
                (String::from("54.5_-3.0"), -2.187),
                (String::from("54.5_-3.5"), -0.424),
                (String::from("54.5_-4.0"), 1.047),
                (String::from("54.5_-4.5"), 2.244),
                (String::from("54.5_-5.0"), 2.994),
                (String::from("54.5_-5.5"), 1.337),
                (String::from("54.5_-6.0"), -0.575),
                (String::from("54.5_-6.5"), -2.338),
                (String::from("54.5_-7.0"), -3.041),
                (String::from("54.5_-7.5"), -2.662),
                (String::from("54.5_-8.0"), -1.808),
                (String::from("55.0_-1.5"), -0.996),
                (String::from("55.0_-2.0"), -4.155),
                (String::from("55.0_-2.5"), -6.204),
                (String::from("55.0_-3.0"), -4.514),
                (String::from("55.0_-3.5"), -2.703),
                (String::from("55.0_-4.0"), -1.58),
                (String::from("55.0_-4.5"), -0.407),
                (String::from("55.0_-5.0"), 0.806),
                (String::from("55.0_-5.5"), 2.081),
                (String::from("55.0_-6.0"), 0.887),
                (String::from("55.0_-6.5"), -0.469),
                (String::from("55.0_-7.0"), -0.993),
                (String::from("55.0_-7.5"), -0.77),
                (String::from("55.5_-1.5"), 0.873),
                (String::from("55.5_-2.0"), -2.474),
                (String::from("55.5_-2.5"), -5.702),
                (String::from("55.5_-3.0"), -5.566),
                (String::from("55.5_-3.5"), -4.895),
                (String::from("55.5_-4.0"), -4.132),
                (String::from("55.5_-4.5"), -2.358),
                (String::from("55.5_-5.0"), -0.579),
                (String::from("55.5_-5.5"), 1.338),
                (String::from("55.5_-6.0"), 2.057),
                (String::from("55.5_-6.5"), 2.505),
                (String::from("56.0_-2.0"), 1.815),
                (String::from("56.0_-2.5"), 0.195),
                (String::from("56.0_-3.0"), -2.189),
                (String::from("56.0_-3.5"), -4.626),
                (String::from("56.0_-4.0"), -5.49),
                (String::from("56.0_-4.5"), -4.919),
                (String::from("56.0_-5.0"), -3.499),
                (String::from("56.0_-5.5"), -1.181),
                (String::from("56.0_-6.0"), 1.063),
                (String::from("56.0_-6.5"), 2.977),
                (String::from("56.5_-2.5"), -0.305),
                (String::from("56.5_-3.0"), -3.11),
                (String::from("56.5_-3.5"), -5.41),
                (String::from("56.5_-4.0"), -6.757),
                (String::from("56.5_-4.5"), -7.005),
                (String::from("56.5_-5.0"), -5.879),
                (String::from("56.5_-5.5"), -3.253),
                (String::from("56.5_-6.0"), 0.046),
                (String::from("56.5_-6.5"), 2.699),
                (String::from("56.5_-7.0"), 4.242),
                (String::from("57.0_-2.0"), 1.061),
                (String::from("57.0_-2.5"), -4.347),
                (String::from("57.0_-3.0"), -6.774),
                (String::from("57.0_-3.5"), -8.256),
                (String::from("57.0_-4.0"), -8.531),
                (String::from("57.0_-4.5"), -8.952),
                (String::from("57.0_-5.0"), -7.613),
                (String::from("57.0_-5.5"), -4.211),
                (String::from("57.0_-6.0"), -0.368),
                (String::from("57.0_-6.5"), 2.421),
                (String::from("57.0_-7.0"), 3.249),
                (String::from("57.0_-7.5"), 4.066),
                (String::from("57.5_-2.0"), 0.562),
                (String::from("57.5_-2.5"), -2.636),
                (String::from("57.5_-3.0"), -3.24),
                (String::from("57.5_-3.5"), -3.825),
                (String::from("57.5_-4.0"), -4.351),
                (String::from("57.5_-4.5"), -5.412),
                (String::from("57.5_-5.0"), -7.049),
                (String::from("57.5_-5.5"), -3.771),
                (String::from("57.5_-6.0"), 0.002),
                (String::from("57.5_-6.5"), 2.105),
                (String::from("57.5_-7.0"), 2.649),
                (String::from("57.5_-7.5"), 3.287),
                (String::from("58.0_-3.5"), 1.614),
                (String::from("58.0_-4.0"), -0.872),
                (String::from("58.0_-4.5"), -2.392),
                (String::from("58.0_-5.0"), -2.029),
                (String::from("58.0_-5.5"), 0.609),
                (String::from("58.0_-6.0"), 2.139),
                (String::from("58.0_-6.5"), 2.056),
                (String::from("58.0_-7.0"), 1.757),
                (String::from("58.5_-3.0"), 1.924),
                (String::from("58.5_-3.5"), 1.382),
                (String::from("58.5_-4.0"), 0.97),
                (String::from("58.5_-4.5"), 0.903),
                (String::from("58.5_-5.0"), 1.605),
                (String::from("58.5_-5.5"), 2.935),
                (String::from("58.5_-6.0"), 2.901),
                (String::from("58.5_-6.5"), 2.723),
                (String::from("58.5_-7.0"), 2.661),
                (String::from("59.0_-2.5"), 2.975),
                (String::from("59.0_-3.0"), 2.525),
                (String::from("59.0_-3.5"), 3.066),
                (String::from("59.5_-1.5"), 3.281),
                (String::from("59.5_-2.5"), 3.684),
                (String::from("59.5_-3.0"), 3.79),
                (String::from("60.0_-1.0"), 2.361),
                (String::from("60.0_-1.5"), 2.383),
                (String::from("60.5_-1.0"), 1.794),
                (String::from("60.5_-1.5"), 1.783),
                (String::from("61.0_-1.0"), 1.721),
            ]);
        let key = format!(
            "{:.1}_{:.1}",
            (latitude * 2.0).round() / 2.0,
            (longitude * 2.0).round() / 2.0
        );
        dbg!(&key);
        *coldest_outside_temperatures_of_year_per_region.get(&key).expect("latitude and longitude position not found in hashmap of coldest_outside_temperatures_of_year_per_region")
    };
    let ground_temperature: f32 = 15.0 - (latitude - 50.0) * (4.0 / 9.0); // Linear regression ground temp across UK at 100m depth
    let tes_range: u8 = ((tes_volume_max + 0.01) / 0.1) as u8; // +0.01 avoids floating point error;
    let solar_maximum: u16 = ((house_size / 8.0) as u16) * 2; // Quarter of the roof for solar, even number
    let house_size_thermal_transmittance_product: f32 = house_size * thermal_transmittance / 1000.0;
    let cumulative_discount_rate: f32 = {
        let mut discount_rate_current: f32 = 1.0;
        let mut cumulative_discount_rate: f32 = 0.0;
        for _year in 0..npc_years {
            cumulative_discount_rate += 1.0 / discount_rate_current;
            discount_rate_current *= discount_rate;
        }
        cumulative_discount_rate
    };
    let monthly_roof_ratios_south: [f32; 12] = {
        let pf: f32 = (std::f32::consts::PI / 180.0 * 35.0 / 2.0).sin(); // Assume roof is 35° from horizontal
        let a: f32 = ax3bx2cxd(-0.66, -0.106, 2.93, 0.0, pf);
        let b: f32 = ax3bx2cxd(3.63, -0.374, -7.4, 0.0, pf);
        let c: f32 = ax3bx2cxd(-2.71, -0.991, 4.59, 1.0, pf);

        let mut monthly_ratios_roof_south: [f32; 12] = [0.0; 12];
        for (month, solar_declination) in monthly_solar_declinations.iter().enumerate() {
            let solar_height_factor: f32 =
                (std::f32::consts::PI / 180.0 * (latitude - solar_declination)).cos();
            monthly_ratios_roof_south[month] = ax2bxc(a, b, c, solar_height_factor);
        }
        monthly_ratios_roof_south
    };
    let u_value: f32 = 1.30 / 1000.0; // 0.00130 kW / m2K linearised from https://zenodo.org/record/4692649#.YQEbio5KjIV&
    #[allow(unused_variables)]
    let agile_tariff_per_hour_over_year: [f32; 8760] =
        import_weather_data(format!("{}agile_tariff.csv", ASSETS_DIR));
    let grid_emissions: f32 = 212.0;

    let print_vars = true;
    if print_vars {
        println!("latitude: {:?}", latitude);
        println!("thermostat_temperature: {:?}", thermostat_temperature);
        println!(
            "hourly_erh_thermostat_temperatures: {:?}",
            hourly_erh_thermostat_temperatures
        );
        println!(
            "hourly_hp_thermostat_temperatures: {:?}",
            hourly_hp_thermostat_temperatures
        );
        println!(
            "monthly_solar_declinations: {:?}",
            monthly_solar_declinations
        );
        println!(
            "monthly_solar_height_factors: {:?}",
            monthly_solar_height_factors
        );
        println!("monthly_hot_water_factors: {:?}", monthly_hot_water_factors);
        println!(
            "monthly_cold_water_temperatures: {:?}",
            monthly_cold_water_temperatures
        );
        println!(
            "monthly_solar_gain_ratios_north: {:?}",
            monthly_solar_gain_ratios_north
        );
        println!(
            "monthly_solar_gain_ratios_south: {:?}",
            monthly_solar_gain_ratios_south
        );
        println!("hourly_hot_water_ratios: {:?}", hourly_hot_water_ratios);
        println!(
            "daily_average_hot_water_volume: {:?}",
            daily_average_hot_water_volume
        );
        println!("hot_water_temperature: {:?}", hot_water_temperature);
        println!("solar_gain_house_factor: {:?}", solar_gain_house_factor);
        println!("epc_body_gain: {:?}", epc_body_gain);
        println!("region_identifier: {:?}", region_identifier);
        println!(
            "monthly_epc_outside_temperatures: {:?}",
            monthly_epc_outside_temperatures
        );
        println!(
            "monthly_epc_solar_irradiances: {:?}",
            monthly_epc_solar_irradiances
        );
        println!("monthly_solar_gains_north: {:?}", monthly_solar_gains_north);
        println!("monthly_solar_gains_south: {:?}", monthly_solar_gains_south);

        println!("heat_capacity: {:?}", heat_capacity);

        println!("body_heat_gain: {:?}", body_heat_gain);

        println!("thermal_transmittance: {:?}", thermal_transmittance);
        println!("optimised_epc_demand: {:?}", optimised_epc_demand);

        println!("erh_total_demand: {:?}", erh_total_demand);
        println!("erh_space_demand: {:?}", erh_space_demand);
        println!("erh_hot_water_demand: {:?}", erh_hot_water_demand);
        println!("erh_max_hourly_demand: {:?}", erh_max_hourly_demand);

        println!("hp_total_demand: {:?}", hp_total_demand);
        println!("hp_space_demand: {:?}", hp_space_demand);
        println!("hp_hot_water_demand: {:?}", hp_hot_water_demand);
        println!("hp_max_hourly_demand: {:?}", hp_max_hourly_demand);

        println!("discount_rate: {:?}", discount_rate);
        println!("npc_years: {:?}", npc_years);
        println!(
            "coldest_outside_temperature_of_year: {:?}",
            coldest_outside_temperature_of_year
        );
        println!("ground_temperature: {:?}", ground_temperature);
        println!("tes_range: {:?}", tes_range);
        println!("solar_maximum: {:?}", solar_maximum);
        println!(
            "house_size_thermal_transmittance_product: {:?}",
            house_size_thermal_transmittance_product
        );
        println!("cumulative_discount_rate: {:?}", cumulative_discount_rate);
        println!("monthly_roof_ratios_south: {:?}", monthly_roof_ratios_south);
        println!("u_value: {:?}", u_value);
        println!("grid_emissions: {:?}", grid_emissions);
    }

    #[derive(Debug, Clone, Copy)]
    enum HeatOption {
        ElectricResistanceHeating = 0,
        AirSourceHeatPump = 1,
        GroundSourceHeatPump = 2,
    }

    #[derive(Debug, Clone, Copy)]
    enum SolarOption {
        None = 0,
        PhotoVoltaics = 1,
        FlatPlate = 2,
        EvacuatedTube = 3,
        PhotoVoltaicsWithFlatPlate = 4,
        PhotoVoltaicsWithEvacuatedTube = 5,
        PhotoVoltaicThermalHybrid = 6,
    }

    #[derive(Debug, Clone, Copy)]
    enum Tariff {
        FlatRate = 0,
        Economy7 = 1,
        BulbSmart = 2,
        OctopusGo = 3,
        OctopusAgile = 4,
    }

    #[derive(Debug, Clone, Copy)]
    struct SystemSpecifications {
        heat_option: HeatOption,
        solar_option: SolarOption,
        pv_size: u16,
        solar_thermal_size: u16,
        tes_volume: f32,
        tariff: Tariff,
        operational_expenditure: f32,
        capital_expenditure: f32,
        net_present_cost: f32,
        operational_emissions: f32,
    }

    let heat_options: [HeatOption; 3] = [
        HeatOption::ElectricResistanceHeating,
        HeatOption::AirSourceHeatPump,
        HeatOption::GroundSourceHeatPump,
    ];

    let solar_options: [SolarOption; 7] = [
        SolarOption::None,
        SolarOption::PhotoVoltaics,
        SolarOption::FlatPlate,
        SolarOption::EvacuatedTube,
        SolarOption::PhotoVoltaicsWithFlatPlate,
        SolarOption::PhotoVoltaicsWithEvacuatedTube,
        SolarOption::PhotoVoltaicThermalHybrid,
    ];

    let tariffs: [Tariff; 5] = [
        Tariff::FlatRate,
        Tariff::Economy7,
        Tariff::BulbSmart,
        Tariff::OctopusGo,
        Tariff::OctopusAgile,
    ];

    let init_specification = SystemSpecifications {
        heat_option: HeatOption::ElectricResistanceHeating,
        solar_option: SolarOption::None,
        pv_size: 0,
        solar_thermal_size: 0,
        tes_volume: 0.0,
        tariff: Tariff::FlatRate,
        operational_expenditure: 0.0,
        capital_expenditure: 0.0,
        net_present_cost: 0.0,
        operational_emissions: 0.0,
    };

    let mut optimal_specifications: [SystemSpecifications; 21] = [init_specification; 21];

    for heat_option in heat_options {
        for solar_option in solar_options {
            //println!("{:?} {:?}", heat_option, solar_option);

            let hourly_thermostat_temperatures: &[f32; 24] = match heat_option {
                HeatOption::AirSourceHeatPump | HeatOption::GroundSourceHeatPump => {
                    &hourly_hp_thermostat_temperatures
                }
                HeatOption::ElectricResistanceHeating => &hourly_erh_thermostat_temperatures,
            };

            let cop_worst: f32 = match heat_option {
                HeatOption::ElectricResistanceHeating => 1.0,
                HeatOption::AirSourceHeatPump => ax2bxc(
                    0.000630,
                    -0.121,
                    6.81,
                    hot_water_temperature - coldest_outside_temperature_of_year,
                ),
                HeatOption::GroundSourceHeatPump => ax2bxc(
                    0.000734,
                    -0.150,
                    8.77,
                    hot_water_temperature - ground_temperature,
                ),
            };

            let hp_electrical_power: f32 = {
                // Mitsubishi have 4kWth ASHP, Kensa have 3kWth GSHP
                // 7kWth Typical maximum size for domestic power
                let mut hp_electrical_power = match heat_option {
                    HeatOption::ElectricResistanceHeating => erh_max_hourly_demand,
                    HeatOption::AirSourceHeatPump | HeatOption::GroundSourceHeatPump => {
                        hp_max_hourly_demand / cop_worst
                    }
                };

                if hp_electrical_power * cop_worst < 4.0 {
                    hp_electrical_power = 4.0 / cop_worst;
                }
                if hp_electrical_power > 7.0 {
                    hp_electrical_power = 7.0;
                }
                hp_electrical_power
            };

            let solar_size_range: u16 = match solar_option {
                SolarOption::None => 1,
                SolarOption::PhotoVoltaicsWithFlatPlate
                | SolarOption::PhotoVoltaicsWithEvacuatedTube => solar_maximum / 2 - 1,
                _ => solar_maximum / 2,
            };

            let mut min_net_present_cost: f32 = f32::MAX;

            let mut find_optimal_specification = |solar_size: u16, tes_option: u8| -> f32 {
                let solar_thermal_size: u16 = match solar_option {
                    SolarOption::None | SolarOption::PhotoVoltaics => 0,
                    _ => (solar_size * 2 + 2),
                };

                let pv_size: u16 = match solar_option {
                    SolarOption::PhotoVoltaics | SolarOption::PhotoVoltaicThermalHybrid => {
                        solar_size * 2 + 2
                    }
                    SolarOption::PhotoVoltaicsWithFlatPlate
                    | SolarOption::PhotoVoltaicsWithEvacuatedTube => {
                        solar_maximum - solar_thermal_size
                    }
                    _ => 0,
                };

                let tes_volume: f32 = 0.1 + (tes_option as f32) * 0.1; // m3
                let hp_electrical_power_worst: f32 = hp_electrical_power * cop_worst;
                let capital_expenditure: f32 = {
                    let capex_hp: f32 = match heat_option {
                        HeatOption::ElectricResistanceHeating => 100.0, // Small additional cost to a TES, https://zenodo.org/record/4692649#.YQEbio5KjIV
                        HeatOption::AirSourceHeatPump => {
                            (200.0 + 4750.0 / hp_electrical_power_worst.powf(1.25))
                                * hp_electrical_power_worst
                                + 1500.0
                        } // ASHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
                        HeatOption::GroundSourceHeatPump => {
                            (200.0 + 4750.0 / hp_electrical_power_worst.powf(1.25))
                                * hp_electrical_power_worst
                                + 800.0 * hp_electrical_power_worst
                        } // GSHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
                    };

                    let capex_pv: f32 = match solar_option {
                        SolarOption::PhotoVoltaics
                        | SolarOption::PhotoVoltaicsWithFlatPlate
                        | SolarOption::PhotoVoltaicsWithEvacuatedTube => {
                            // PV panels installed
                            let pv_size = pv_size as f32;
                            if pv_size * 0.2 < 4.0 {
                                // Less than 4kWp
                                pv_size * 0.2 * 1100.0 // m2 * 0.2kWp / m2 * £1100 / kWp = £
                            } else {
                                // Larger than 4kWp lower £ / kWp
                                pv_size * 0.2 * 900.0 // m2 * 0.2kWp / m2 * £900 / kWp = £
                            }
                        }
                        _ => 0.0,
                    };

                    let capex_solar_thermal: f32 = match solar_option {
                        // Flat plate solar thermal
                        // Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
                        // Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
                        SolarOption::FlatPlate | SolarOption::PhotoVoltaicsWithFlatPlate => {
                            (solar_thermal_size as f32) * (225.0 + 270.0 / (9.0 * 1.6))
                                + 490.0
                                + 800.0
                                + 800.0
                        }
                        // https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
                        SolarOption::PhotoVoltaicThermalHybrid => {
                            ((solar_thermal_size as f32) / 1.6) * (480.0 + 270.0 / 9.0)
                                + 640.0
                                + 490.0
                                + 800.0
                                + 1440.0
                        }
                        // Evacuated tube solar thermal
                        // Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
                        // Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
                        SolarOption::EvacuatedTube
                        | SolarOption::PhotoVoltaicsWithEvacuatedTube => {
                            (solar_thermal_size as f32) * (280.0 + 270.0 / (9.0 * 1.6))
                                + 490.0
                                + 800.0
                                + 800.0
                        }
                        _ => 0.0,
                    };

                    // Formula based on this data https://assets.publishing.service.gov.uk/government/uploads/system/uploads/attachment_data/file/545249/DELTA_EE_DECC_TES_Final__1_.pdf
                    let capex_tes = 2068.3 * tes_volume.powf(0.553);

                    capex_hp + capex_pv + capex_solar_thermal + capex_tes
                };

                let tes_radius: f32 = (tes_volume / (2.0 * std::f32::consts::PI)).powf(1.0 / 3.0); //For cylinder with height = 2x radius
                let tes_charge_full: f32 =
                    tes_volume * 1000.0 * 4.18 * (hot_water_temperature - 40.0) / 3600.0; // 40 min temp
                let tes_charge_boost: f32 = tes_volume * 1000.0 * 4.18 * (60.0 - 40.0) / 3600.0; //  # kWh, 60C HP with PV boost
                let tes_charge_max: f32 = tes_volume * 1000.0 * 4.18 * (95.0 - 40.0) / 3600.0;
                //  # kWh, 95C electric and solar

                let tes_charge_min: f32 = 10.0 * 4.18 * (hot_water_temperature - 10.0) / 3600.0;
                // 10litres hot min amount
                //CWT coming in from DHW re - fill, accounted for by DHW energy out, DHW min useful temperature 40°C
                //Space heating return temperature would also be ~40°C with flow at 51°C

                let pi_d: f32 = std::f32::consts::PI * tes_radius * 2.0;
                let pi_r2: f32 = std::f32::consts::PI * tes_radius * tes_radius;
                let pi_d2: f32 = pi_d * tes_radius * 2.0;

                let mut min_tariff_net_present_cost: f32 = f32::MAX;

                for tariff in tariffs {
                    let mut inside_temperature: f32 = thermostat_temperature;
                    //let mut solar_thermal_generation_total: f32 = 0.0;
                    let mut operational_costs_peak: f32 = 0.0;
                    let mut operational_costs_off_peak: f32 = 0.0;
                    let mut operational_emissions: f32 = 0.0;
                    let mut tes_state_of_charge: f32 = tes_charge_full;
                    // kWh, for H2O, starts full to prevent initial demand spike
                    // https ://www.sciencedirect.com/science/article/pii/S0306261916302045
                    let mut hour_year_counter: usize = 0;

                    for (month, days_in_month) in DAYS_IN_MONTHS.iter().enumerate() {
                        let monthly_solar_gain_ratio_south: f32 =
                            monthly_solar_gain_ratios_south[month];
                        let monthly_solar_gain_ratio_north: f32 =
                            monthly_solar_gain_ratios_north[month];
                        let monthly_cold_water_temperature: f32 =
                            monthly_cold_water_temperatures[month];
                        let monthly_hot_water_factor: f32 = monthly_hot_water_factors[month];
                        // let solar_declination: f32 = monthly_solar_declinations[month];
                        let ratio_roof_south: f32 = monthly_roof_ratios_south[month];
                        for _day in 0..*days_in_month {
                            for hour in 0..24 {
                                let outside_temperature =
                                    hourly_outside_temperatures_over_year[hour_year_counter];
                                let solar_irradiance =
                                    hourly_solar_irradiances_over_year[hour_year_counter];
                                {
                                    let solar_gain_south = solar_irradiance
                                        * monthly_solar_gain_ratio_south
                                        * solar_gain_house_factor;
                                    let solar_gain_north = solar_irradiance
                                        * monthly_solar_gain_ratio_north
                                        * solar_gain_house_factor;
                                    let heat_loss = house_size_thermal_transmittance_product
                                        * (inside_temperature - outside_temperature);
                                    // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
                                    inside_temperature += (-heat_loss
                                        + solar_gain_south
                                        + solar_gain_north
                                        + body_heat_gain)
                                        / heat_capacity;
                                }

                                let (
                                    tes_upper_temperature,
                                    tes_lower_temperature,
                                    tes_thermocline_height,
                                ): (f32, f32, f32) = match tes_state_of_charge {
                                    // Currently at nominal temperature ranges
                                    // tes_lower_temperature Bottom of the tank would still be at CWT,
                                    // tes_thermocline_height %, from top down, .25 is top 25 %
                                    x if x <= tes_charge_full => (
                                        51.0,
                                        monthly_cold_water_temperature,
                                        tes_state_of_charge / tes_charge_full,
                                    ),
                                    x if x <= tes_charge_boost => (
                                        60.0,
                                        51.0,
                                        (tes_state_of_charge - tes_charge_full)
                                            / (tes_charge_boost - tes_charge_full),
                                    ),
                                    // At max tes temperature
                                    _ => (
                                        95.0,
                                        60.0,
                                        (tes_state_of_charge - tes_charge_boost)
                                            / (tes_charge_max - tes_charge_boost),
                                    ),
                                };

                                let tes_upper_losses: f32 = (tes_upper_temperature
                                    - inside_temperature)
                                    * u_value
                                    * (pi_d2 * tes_thermocline_height + pi_r2); // losses in kWh
                                let tes_lower_losses: f32 = (tes_lower_temperature
                                    - inside_temperature)
                                    * u_value
                                    * (pi_d2 * (1.0 - tes_thermocline_height) + pi_r2);
                                let total_losses: f32 = tes_upper_losses + tes_lower_losses;
                                tes_state_of_charge -= total_losses;
                                inside_temperature += total_losses / heat_capacity;

                                let hourly_thermostat_temperature: f32 =
                                    hourly_thermostat_temperatures[hour];

                                let agile_tariff_current: f32 =
                                    agile_tariff_per_hour_over_year[hour_year_counter];
                                let hourly_hot_water_ratio: f32 = hourly_hot_water_ratios[hour];
                                let hourly_hot_water_demand: f32 = (daily_average_hot_water_volume
                                    * 4.18
                                    * (hot_water_temperature - monthly_cold_water_temperature)
                                    / 3600.0)
                                    * monthly_hot_water_factor
                                    * hourly_hot_water_ratio;

                                let (cop_current, cop_boost): (f32, f32) = match heat_option {
                                    HeatOption::ElectricResistanceHeating => (1.0, 1.0),
                                    // ASHP, source A review of domestic heat pumps
                                    HeatOption::AirSourceHeatPump => (
                                        ax2bxc(
                                            0.00063,
                                            -0.121,
                                            6.81,
                                            hot_water_temperature - outside_temperature,
                                        ),
                                        ax2bxc(0.00063, -0.121, 6.81, 60.0 - outside_temperature),
                                    ),
                                    // GSHP, source A review of domestic heat pumps
                                    HeatOption::GroundSourceHeatPump => (
                                        ax2bxc(
                                            0.000734,
                                            -0.150,
                                            8.77,
                                            hot_water_temperature - ground_temperature,
                                        ),
                                        ax2bxc(0.000734, -0.150, 8.77, 60.0 - ground_temperature),
                                    ),
                                };

                                let pv_efficiency: f32 = match solar_option {
                                    // https://www.sciencedirect.com/science/article/pii/S0306261919313443#b0175
                                    SolarOption::PhotoVoltaicThermalHybrid => {
                                        (14.7
                                            * (1.0
                                                - 0.0045
                                                    * ((tes_upper_temperature
                                                        + tes_lower_temperature)
                                                        / 2.0
                                                        - 25.0)))
                                            / 100.0
                                    }
                                    // Technology Library https://zenodo.org/record/4692649#.YQEbio5KjIV
                                    // monocrystalline used for domestic
                                    _ => 0.1928,
                                };

                                let incident_irradiance_roof_south: f32 =
                                    solar_irradiance * ratio_roof_south / 1000.0; // kW / m2
                                let pv_generation = (pv_size as f32)
                                    * pv_efficiency
                                    * incident_irradiance_roof_south
                                    * 0.8; // 80 % shading factor

                                let solar_thermal_generation: f32 = match solar_option {
                                    SolarOption::None | SolarOption::PhotoVoltaics => 0.0,
                                    _ => {
                                        if incident_irradiance_roof_south == 0.0 {
                                            0.0
                                        } else {
                                            let solar_thermal_collector_temperature: f32 =
                                                (tes_upper_temperature + tes_lower_temperature)
                                                    / 2.0;
                                            // Collector to heat from tes lower temperature to tes upper temperature, so use the average temperature

                                            let (a, b, c): (f32, f32, f32) = match solar_option {
                                                // https://www.sciencedirect.com/science/article/pii/B9781782422136000023
                                                SolarOption::FlatPlate
                                                | SolarOption::PhotoVoltaicsWithFlatPlate => {
                                                    (-0.000038, -0.0035, 0.78)
                                                }
                                                // https://www.sciencedirect.com/science/article/pii/S0306261919313443#b0175
                                                SolarOption::PhotoVoltaicThermalHybrid => {
                                                    (-0.0000176, -0.003325, 0.726)
                                                }
                                                // https://www.sciencedirect.com/science/article/pii/B9781782422136000023
                                                SolarOption::EvacuatedTube
                                                | SolarOption::PhotoVoltaicsWithEvacuatedTube => {
                                                    (-0.00002, -0.0009, 0.625)
                                                }
                                                _ => panic!(
                                                    "No other solar option should be possible"
                                                ),
                                            };
                                            let solar_thermal_generation = 0.8
                                                * (solar_thermal_size as f32)
                                                * ax2bxc(
                                                    a,
                                                    b,
                                                    c * incident_irradiance_roof_south,
                                                    solar_thermal_collector_temperature
                                                        - outside_temperature,
                                                );
                                            if solar_thermal_generation < 0.0 {
                                                0.0
                                            } else {
                                                solar_thermal_generation
                                            }
                                        }
                                    }
                                };

                                tes_state_of_charge += solar_thermal_generation;
                                //solar_thermal_generation_total += solar_thermal_generation;
                                // Dumps any excess solar generated heat to prevent boiling TES
                                tes_state_of_charge = if tes_state_of_charge < tes_charge_max {
                                    tes_state_of_charge
                                } else {
                                    tes_charge_max
                                };

                                let hourly_space_demand: f32 = {
                                    if inside_temperature > hourly_thermostat_temperature {
                                        0.0
                                    } else {
                                        let hourly_space_demand = (hourly_thermostat_temperature
                                            - inside_temperature)
                                            * heat_capacity;
                                        if (hourly_space_demand + hourly_hot_water_demand)
                                            < (tes_state_of_charge
                                                + hp_electrical_power * cop_current)
                                        {
                                            inside_temperature = hourly_thermostat_temperature;
                                            hourly_space_demand
                                        } else {
                                            if tes_state_of_charge > 0.0 {
                                                // Priority to space demand over TES charging
                                                let space_hr_demand = (tes_state_of_charge
                                                    + hp_electrical_power * cop_current)
                                                    - hourly_hot_water_demand;
                                                inside_temperature +=
                                                    space_hr_demand / heat_capacity;
                                                space_hr_demand
                                            } else {
                                                let space_hr_demand = (hp_electrical_power
                                                    * cop_current)
                                                    - hourly_hot_water_demand;
                                                inside_temperature +=
                                                    space_hr_demand / heat_capacity;
                                                space_hr_demand
                                            }
                                        }
                                    }
                                };

                                let mut electrical_demand: f32 = {
                                    let space_water_demand =
                                        hourly_space_demand + hourly_hot_water_demand;
                                    if space_water_demand < tes_state_of_charge {
                                        // TES can provide all demand
                                        tes_state_of_charge -= space_water_demand;
                                        0.0
                                    } else if space_water_demand
                                        < (tes_state_of_charge + hp_electrical_power * cop_current)
                                    {
                                        if tes_state_of_charge > 0.0 {
                                            let electrical_demand = (space_water_demand
                                                - tes_state_of_charge)
                                                / cop_current;
                                            tes_state_of_charge = 0.0; // TES needs support so taken to empty if it had any charge
                                            electrical_demand
                                        } else {
                                            space_water_demand / cop_current
                                        }
                                    } else {
                                        // TES and HP can't meet hour demand
                                        if tes_state_of_charge > 0.0 {
                                            tes_state_of_charge = 0.0;
                                        }
                                        hp_electrical_power
                                    }
                                };

                                // calculate_electrical_demand_for_tes_charging
                                if tes_state_of_charge < tes_charge_full
                                    && ((matches!(tariff, Tariff::FlatRate)
                                        && 12 < hour
                                        && hour < 16)
                                        || (matches!(tariff, Tariff::Economy7)
                                            && (hour == 23 || hour < 6))
                                        || (matches!(tariff, Tariff::BulbSmart)
                                            && 12 < hour
                                            && hour < 16)
                                        || (matches!(tariff, Tariff::OctopusGo) && hour < 5)
                                        || (matches!(tariff, Tariff::OctopusAgile)
                                            && agile_tariff_current < 9.0))
                                {
                                    // Flat rate and smart tariff charges TES at typical day peak air temperature times
                                    // GSHP is not affected so can keep to these times too
                                    if (tes_charge_full - tes_state_of_charge)
                                        < ((hp_electrical_power - electrical_demand) * cop_current)
                                    {
                                        // Small top up
                                        electrical_demand +=
                                            (tes_charge_full - tes_state_of_charge) / cop_current;
                                        tes_state_of_charge = tes_charge_full;
                                    } else {
                                        // HP can not fully top up in one hour
                                        tes_state_of_charge +=
                                            (hp_electrical_power - electrical_demand) * cop_current;
                                        electrical_demand = hp_electrical_power;
                                    }
                                }

                                {
                                    // Boost temperature if any spare PV generated electricity, as reduced cop, raises to nominal temp above first
                                    let pv_remaining: f32 = pv_generation - electrical_demand;
                                    let tes_boost_charge_difference: f32 =
                                        tes_charge_boost - tes_state_of_charge;
                                    if pv_remaining > 0.0 && tes_boost_charge_difference > 0.0 {
                                        if (tes_boost_charge_difference
                                            < (pv_remaining * cop_boost))
                                            && (tes_boost_charge_difference
                                                < ((hp_electrical_power - electrical_demand)
                                                    * cop_boost))
                                        {
                                            electrical_demand +=
                                                tes_boost_charge_difference / cop_boost;
                                            tes_state_of_charge = tes_charge_boost;
                                        } else if pv_remaining < hp_electrical_power {
                                            tes_state_of_charge += pv_remaining * cop_boost;
                                            electrical_demand += pv_remaining;
                                        } else {
                                            tes_state_of_charge += (hp_electrical_power
                                                - electrical_demand)
                                                * cop_boost;
                                            electrical_demand = hp_electrical_power;
                                        }
                                    }
                                }

                                // recharge tes to minimum
                                if tes_state_of_charge < tes_charge_min {
                                    // Take back up to 10L capacity if possible no matter what time
                                    if (tes_charge_min - tes_state_of_charge)
                                        < (hp_electrical_power - electrical_demand) * cop_current
                                    {
                                        electrical_demand +=
                                            (tes_charge_min - tes_state_of_charge) / cop_current;
                                        tes_state_of_charge = tes_charge_min;
                                    } else if electrical_demand < hp_electrical_power {
                                        // Can't take all the way back up to 10L charge
                                        tes_state_of_charge +=
                                            (hp_electrical_power - electrical_demand) * cop_current;
                                    }
                                }

                                let (pv_equivalent_revenue, electrical_import): (f32, f32) = {
                                    if pv_generation > electrical_demand {
                                        // Generating more electricity than using
                                        (pv_generation - electrical_demand, 0.0)
                                    } else {
                                        (0.0, electrical_demand - pv_generation)
                                    }
                                };

                                // add_electrical_import_cost_to_opex
                                if electrical_import > 0.0 {
                                    match tariff {
                                        // Flat rate tariff https://www.nimblefins.co.uk/average-cost-electricity-kwh-uk#:~:text=Unit%20Cost%20of%20Electricity%20per,more%20than%20the%20UK%20average
                                        // Average solar rate https://www.greenmatch.co.uk/solar-energy/solar-panels/solar-panel-grants
                                        Tariff::FlatRate => {
                                            operational_costs_peak += 0.163 * electrical_import
                                        }
                                        Tariff::Economy7 => {
                                            // Economy 7 tariff, same source as flat rate above
                                            if hour < 6 || hour == 23 {
                                                // Off Peak
                                                operational_costs_off_peak +=
                                                    0.095 * electrical_import;
                                            } else {
                                                // Peak
                                                operational_costs_peak += 0.199 * electrical_import;
                                            }
                                        }
                                        Tariff::BulbSmart => {
                                            // Bulb smart, for East Midlands values 2021
                                            // https://help.bulb.co.uk/hc/en-us/articles/360017795731-About-Bulb-s-smart-tariff
                                            if 15 < hour && hour < 19 {
                                                // Peak winter times throughout the year
                                                operational_costs_peak +=
                                                    0.2529 * electrical_import;
                                            } else {
                                                // Off peak
                                                operational_costs_off_peak +=
                                                    0.1279 * electrical_import;
                                            }
                                        }
                                        Tariff::OctopusGo => {
                                            // Octopus Go EV, LE10 0YE 2012, https://octopus.energy/go/rates/
                                            // https://www.octopusreferral.link/octopus-energy-go-tariff/
                                            if hour < 5 {
                                                // Off Peak
                                                operational_costs_off_peak +=
                                                    0.05 * electrical_import;
                                            } else {
                                                // Peak
                                                operational_costs_peak +=
                                                    0.1533 * electrical_import;
                                            }
                                        }
                                        Tariff::OctopusAgile => {
                                            // Octopus Agile file 2020
                                            // 2021 Octopus export rates https://octopus.energy/outgoing/
                                            if agile_tariff_current < 9.0 {
                                                // Off peak, lower range of variable costs
                                                operational_costs_off_peak += (agile_tariff_current
                                                    / 100.0)
                                                    * electrical_import;
                                            } else {
                                                // Peak, upper range of variable costs
                                                operational_costs_peak += (agile_tariff_current
                                                    / 100.0)
                                                    * electrical_import;
                                            }
                                        }
                                    }
                                }

                                // subtract_pv_revenue_from_opex
                                if pv_equivalent_revenue > 0.0 {
                                    match tariff {
                                        // Flat rate tariff https://www.nimblefins.co.uk/average-cost-electricity-kwh-uk#:~:text=Unit%20Cost%20of%20Electricity%20per,more%20than%20the%20UK%20average
                                        // Average solar rate https://www.greenmatch.co.uk/solar-energy/solar-panels/solar-panel-grants
                                        Tariff::FlatRate => {
                                            operational_costs_peak -=
                                                pv_equivalent_revenue * (0.163 + 0.035) / 2.0
                                        }
                                        Tariff::Economy7 => {
                                            // Economy 7 tariff, same source as flat rate above
                                            if hour < 6 || hour == 23 {
                                                // Off Peak
                                                operational_costs_off_peak -=
                                                    pv_equivalent_revenue * (0.095 + 0.035) / 2.0;
                                            } else {
                                                // Peak
                                                operational_costs_peak -=
                                                    pv_equivalent_revenue * (0.199 + 0.035) / 2.0;
                                            }
                                        }
                                        Tariff::BulbSmart => {
                                            // Bulb smart, for East Midlands values 2021
                                            // https://help.bulb.co.uk/hc/en-us/articles/360017795731-About-Bulb-s-smart-tariff
                                            if 15 < hour && hour < 19 {
                                                // Peak winter times throughout the year
                                                operational_costs_peak -=
                                                    pv_equivalent_revenue * (0.2529 + 0.035) / 2.0;
                                            } else {
                                                // Off peak
                                                operational_costs_off_peak -=
                                                    pv_equivalent_revenue * (0.1279 + 0.035) / 2.0;
                                            }
                                        }
                                        Tariff::OctopusGo => {
                                            // Octopus Go EV, LE10 0YE 2012, https://octopus.energy/go/rates/
                                            // https://www.octopusreferral.link/octopus-energy-go-tariff/
                                            if hour < 5 {
                                                // Off Peak
                                                operational_costs_off_peak -=
                                                    pv_equivalent_revenue * (0.05 + 0.03) / 2.0;
                                            } else {
                                                // Peak
                                                operational_costs_peak -=
                                                    pv_equivalent_revenue * (0.1533 + 0.03) / 2.0;
                                            }
                                        }
                                        Tariff::OctopusAgile => {
                                            // Octopus Agile file 2020
                                            // 2021 Octopus export rates https ://octopus.energy/outgoing/
                                            if agile_tariff_current < 9.0 {
                                                // Off peak, lower range of variable costs
                                                operational_costs_off_peak -= pv_equivalent_revenue
                                                    * ((agile_tariff_current / 100.0) + 0.055)
                                                    / 2.0;
                                            } else {
                                                // Peak, upper range of variable costs
                                                operational_costs_peak -= pv_equivalent_revenue
                                                    * ((agile_tariff_current / 100.0) + 0.055)
                                                    / 2.0;
                                            }
                                        }
                                    }
                                }

                                let hourly_operational_emissions: f32 = {
                                    // Operational emissions summation, 22.5 average ST
                                    // from https://post.parliament.uk/research-briefings/post-pn-0523/
                                    let emissions_solar_thermal: f32 =
                                        solar_thermal_generation * 22.5;

                                    let emissions_pv_generation: f32 = {
                                        // https://www.parliament.uk/globalassets/documents/post/postpn_383-carbon-footprint-electricity-generation.pdf
                                        // 75 for PV, 75 - Grid_Emissions show emissions saved for the grid or for reducing other electrical bills
                                        if pv_size > 0 {
                                            (pv_generation - pv_equivalent_revenue) * 75.0
                                                + pv_equivalent_revenue * (75.0 - grid_emissions)
                                        } else {
                                            0.0
                                        }
                                    };

                                    let emissions_grid_import: f32 =
                                        electrical_import * grid_emissions;

                                    emissions_solar_thermal
                                        + emissions_pv_generation
                                        + emissions_grid_import
                                };
                                operational_emissions += hourly_operational_emissions;

                                hour_year_counter += 1;
                            }
                        }
                    }
                    let operational_expenditure: f32 =
                            operational_costs_peak + operational_costs_off_peak; // tariff

                    let net_present_cost: f32 = capital_expenditure
                        + operational_expenditure * cumulative_discount_rate;

                    if net_present_cost < min_tariff_net_present_cost {
                        min_tariff_net_present_cost = net_present_cost;
                    } // for surface optimisation

                    if net_present_cost < min_net_present_cost {
                        // Lowest cost TES & tariff for heating tech. For OpEx vs CapEx plots, with optimised TES and tariff
                        min_net_present_cost = net_present_cost;

                        optimal_specifications
                            [((heat_option as u8) * 7 + (solar_option as u8)) as usize] =
                            SystemSpecifications {
                                heat_option,
                                solar_option,
                                pv_size,
                                solar_thermal_size,
                                tes_volume,
                                tariff,
                                operational_expenditure,
                                capital_expenditure,
                                net_present_cost,
                                operational_emissions,
                            };
                    }
                }
                min_tariff_net_present_cost
            };

            for solar_size in 0..solar_size_range {
                for tes_option in 0..tes_range {
                    let _min_tariff_net_present_cost: f32 =
                        find_optimal_specification(solar_size, tes_option);
                }
            }
        }
    }

    for s in optimal_specifications {
        //dbg!(optimal_specification);
        println!(
            "{:?}, {:?}, {}, {}, {}, {:?}, {}, {}, {}, {}",
            s.heat_option as u8,
            s.solar_option as u8,
            s.pv_size,
            s.solar_thermal_size,
            s.tes_volume,
            s.tariff,
            s.operational_expenditure,
            s.capital_expenditure,
            s.net_present_cost,
            s.operational_emissions
        );
    }
}
