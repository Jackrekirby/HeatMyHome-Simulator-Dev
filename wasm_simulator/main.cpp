#include <iostream>
#include <sstream>
#include <cmath>
#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>

#include "animal.h"

#include <array>

#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_WARNINGS

#define USING_EMSCRIPTEN_MACRO

#ifndef USING_EMSCRIPTEN_MACRO
#include <chrono>
#include <thread>
#include <algorithm>
#include <execution>
#endif

///////////////////////////////////////////////////////////////////

// HeatNinjaCpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <cmath>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <map>

std::string printArray(const auto& arr) {
    std::stringstream ss;
    for (const auto& element : arr) {
        ss << element << ", ";
    }
    return ss.str();
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 3)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

class HeatNinja {
public:
    // inputs
    const int num_occupants;
    const std::string location;
    const int epc_space_heating;
    const float house_size;
    const float tes_volume_max;
    const float temp;
    const float latitude;
    const float longitude;

    // compile time constant
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr std::array<float, 24> dhw_hourly_ratios = { 0.025f, 0.018f, 0.011f, 0.010f, 0.008f, 0.013f, 0.017f, 0.044f, 0.088f, 0.075f, 0.060f, 0.056f, 0.050f, 0.043f, 0.036f, 0.029f, 0.030f, 0.036f, 0.053f, 0.074f, 0.071f, 0.059f, 0.050f, 0.041f };

    static constexpr int hot_water_temp = 51;
    static constexpr int grid_emissions = 212;
    static constexpr std::array<float, 24> hw_hourly_ratios = { 0.025f, 0.018f, 0.011f, 0.010f, 0.008f, 0.013f, 0.017f, 0.044f, 0.088f, 0.075f, 0.060f, 0.056f,
            0.050f, 0.043f, 0.036f, 0.029f, 0.030f, 0.036f, 0.053f, 0.074f, 0.071f, 0.059f, 0.050f, 0.041f };
    static constexpr std::array<float, 12> dhw_monthly_factor = { 1.10f, 1.06f, 1.02f, 0.98f, 0.94f, 0.90f, 0.90f, 0.94f, 0.98f, 1.02f, 1.06f, 1.10f };

    static constexpr float asg_s = 1.7854776310850238f; // DONT PRECALCULATE
    static constexpr float bsg_s = -4.136191372926868f;
    static constexpr float csg_s = 2.791990437138481f;
    static constexpr float asg_n = 0.5136345341640034f;
    static constexpr float bsg_n = -0.5982016409089308f;
    static constexpr float csg_n = 0.44657601076449904f;

    static constexpr std::array<float, 12> solar_declination = { -20.7f, -12.8f, -1.8f, 9.8f, 18.8f, 23.1f, 21.2f, 13.7f, 2.9f, -8.7f, -18.4f, -23.0f };  // Monthly values

    static constexpr std::array<int, 24> epc_temp_profile_summer = { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
    static constexpr std::array<int, 24> epc_temp_profile_weekend = { 7, 7, 7, 7, 7, 7, 7, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 };
    static constexpr std::array<int, 24> epc_temp_profile_other = { 7, 7, 7, 7, 7, 7, 7, 20, 20, 20, 7, 7, 7, 7, 7, 7, 20, 20, 20, 20, 20, 20, 20, 20 };
    static constexpr float u_value = 1.30f / 1000; // 0.00130 kW / m2K linearised from https ://zenodo.org/record/4692649#.YQEbio5KjIV &

    // run time constants
    const std::array<float, 24> temp_profile;
    const std::array<float, 24> hp_temp_profile;
    const std::array<float, 12> cold_water_temp;
    const float dhw_avg_daily_vol;

    const float heat_capacity; // kWh / K, 250 kJ / m2K average UK dwelling specific heat capacity in SAP
    const float body_heat_gain; // kWh
    const float epc_body_gain; // kWh

    const int postcode_id;
    const std::array<float, 12> epc_outside_temp;
    const std::array<int, 12> epc_solar_irradiance;

    const float solar_gain_house_factor;
    const std::array<float, 12> solar_height_factors;
    const std::array<float, 12> ratios_sg_south;
    const std::array<float, 12> ratios_sg_north;
    const std::array<float, 12> incident_irradiances_sg_south;
    const std::array<float, 12> incident_irradiances_sg_north;
    const std::array<float, 12> solar_gains_south;
    const std::array<float, 12> solar_gains_north;
    const std::array<float, 12> ratios_roof_south;

    const float coldest_outside_temp;
    const std::vector<float> outside_temps;
    const std::vector<float> solar_irradiances;
    const std::vector<float> agile_tariff;

    const float discount_rate = 1.035f; // 3.5% standard for UK HMRC
    // https://www.finance-ni.gov.uk/articles/step-eight-calculate-net-present-values-and-assess-uncertainties

    const int npc_years = 20;

    // non constants
    float thermal_transmittance = 0;
    float house_size_thermal_transmittance_product = 0;
    //std::ifstream outside_temps_file;
    //std::ifstream solar_irradiances_file;
    //std::ifstream agile_tariff_file;

    struct Demand {
        float total;
        float max_hourly;

        Demand(float total = 0, float max_hourly = 0)
            : total(total), max_hourly(max_hourly) {

        }
    };

    Demand hp_demand;
    Demand boiler_demand;

    HeatNinja(int num_occupants, const std::string& location, int epc_space_heating,
        float house_size, float tes_volume_max, float temp, float latitude, float longitude)
        : num_occupants(num_occupants),
        location(location),
        epc_space_heating(epc_space_heating),
        house_size(house_size),
        tes_volume_max(std::max(std::min(tes_volume_max, 0.1f), 3.0f)),
        temp(temp),
        latitude(latitude),
        longitude(longitude),
        temp_profile(initTempProfile(temp)),
        hp_temp_profile(initHpTempProfile(temp)),
        cold_water_temp(initColdWaterTemp(latitude)),
        dhw_avg_daily_vol(initDhwAvgDailyVol(num_occupants)),
        heat_capacity(initHeatCapacity(house_size)),
        body_heat_gain(initBodyHeatGain(num_occupants)),
        epc_body_gain(initEpcBodyGain(house_size)),
        postcode_id(initPostcodeId(location)),
        epc_outside_temp(initEpcOutsideTemp()),
        epc_solar_irradiance(initEpcSolarIrradiance()),
        solar_height_factors(initSolarHeightFactors(latitude)),
        solar_gain_house_factor(initSolarGainHouseFactor(house_size)),
        ratios_sg_south(initRatiosSgSouth()),
        ratios_sg_north(initRatiosSgNorth()),
        incident_irradiances_sg_south(initIncidentIrradiancesSgSouth()),
        incident_irradiances_sg_north(initIncidentIrradiancesSgNorth()),
        solar_gains_south(initSolarGainsSouth()),
        solar_gains_north(initSolarGainsNorth()),
        coldest_outside_temp(initColdestOutsideTemp()),
        outside_temps(importWeatherData("outside_temps")),
        solar_irradiances(importWeatherData("solar_irradiances")),
        agile_tariff(importAgileTariff("solar_irradiances")),
        ratios_roof_south(initRatiosRoofSouth())
    {
        //initOutsideTempFile();
        //initSolarIrradiancesFile();
        //initAgileTariffFile();

        //std::cout << "outside temps: " << printArray(outside_temps) << '\n';

        //fmt::print("Num occupants: {}\n", num_occupants);
        //fmt::print("EPC Space Heating : {}\n", epc_space_heating);
        //fmt::print("House Size: {}\n", house_size);
        //fmt::print("TES Volume Maximum: {}\n", tes_volume_max);
        //fmt::print("Thermostat Temp: {}\n", temp);
        //fmt::print("Latitude: {}\n", latitude);
        //fmt::print("Longitude: {}\n", longitude);

        //std::cout << "Longitude: " << longitude << '\n';

        //fmt::print("Temp Profile: {}\n", printArray(temp_profile));
        //fmt::print("HP Temp Profile: {}\n", printArray(hp_temp_profile));
        //fmt::print("Cold Water Temp: {}\n", printArray(cold_water_temp));

        //fmt::print("DHW Avg Daily Vol: {:.2f}\n", dhw_avg_daily_vol);
        //fmt::print("Heat Capacity: {:.2f}\n", heat_capacity);
        //fmt::print("Body Heat Gain: {:.2f}\n", body_heat_gain);
        //fmt::print("EPC Body Heat Gain: {:.2f}\n", body_heat_gain);

        //fmt::print("Postcode ID: {}\n", postcode_id);
        //fmt::print("EPC Outside Temp: {}\n", printArray(epc_outside_temp));
        //fmt::print("EPC Solar Irridance: {}\n", printArray(epc_solar_irradiance));

        //fmt::print("solar_height_factors: {}\n", printArray(solar_height_factors));
        //fmt::print("solar_gain_house_factor: {}\n", solar_gain_house_factor);
        //fmt::print("ratios_sg_south: {}\n", printArray(ratios_sg_south));
        //fmt::print("incident_irradiances_sg_south: {}\n", printArray(incident_irradiances_sg_south));
        //fmt::print("solar_gains_south: {}\n", printArray(solar_gains_south));

        //fmt::print("ratios_sg_north: {}\n", printArray(ratios_sg_north));
        //fmt::print("incident_irradiances_sg_north: {}\n", printArray(incident_irradiances_sg_north));
        //fmt::print("solar_gains_north: {}\n", printArray(solar_gains_north));

        //std::cout << "epc_outside_temp: " << printArray(epc_outside_temp) << '\n';
        //std::cout << "epc_solar_irradiance: " << printArray(epc_solar_irradiance) << '\n';

        //std::cout << "solar_gains_north: " << printArray(solar_gains_north) << '\n';

        //fmt::print("\n\n");
        std::cout << "\n--- Energy Performance Certicate Demand ---" << '\n';
        calcEpcYear();

        //fmt::print("\n\n");
        //fmt::print("Heat Pump Demand\n");
        std::cout << "\n--- Heat Pump Yearly Demand ---" << '\n';
        hp_demand = calcDemandYear(hp_temp_profile);

        //fmt::print("\n\n");
        //fmt::print("Boiler Demand\n");
        std::cout << "\n--- Boiler Yearly Demand ---" << '\n';
        boiler_demand = calcDemandYear(temp_profile);
        
        //std::cout << "initHeaterTesSettings" << '\n';
        //initHeaterTesSettings();
        //std::cout << "finished sim" << '\n';
    }

    std::array<float, 24> initTempProfile(float temp) {
        const float temp2 = temp - 2;
        return { temp2, temp2, temp2, temp2, temp2, temp2, temp2, temp, temp, temp, temp, temp,
            temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp2, temp2 };
    }

    std::array<float, 24> initHpTempProfile(float temp) {
        return { temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp,
            temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp };
    }

    std::array<float, 12> initColdWaterTemp(float latitude) {
        if (latitude < 52.2f) { // South of England
            return { 12.1f, 11.4f, 12.3f, 15.2f, 16.1f, 19.3f, 21.2f, 20.1f, 19.5f, 16.8f, 13.7f, 12.4f };
        }
        else if (latitude < 53.3f) { // Middle of England and Wales
            return { 12.9f, 13.3f, 14.4f, 16.3f, 17.7f, 19.7f, 21.8f, 20.1f, 20.3f, 17.8f, 15.3f, 14.0f };
        }
        else if (latitude < 53.3f) { // North of England and Northern Ireland
            return { 9.6f, 9.3f, 10.7f, 13.7f, 15.3f, 17.3f, 19.3f, 18.6f, 17.9f, 15.5f, 12.3f, 10.5f };
        }
        else { // Scotland
            return { 9.6f, 9.2f, 9.8f, 13.2f, 14.5f, 16.8f, 19.4f, 18.5f, 17.5f, 15.1f, 13.7f, 12.4f };
        }
    }

    float initDhwAvgDailyVol(int num_occupants) {
        const float showers_vol = (0.45f * num_occupants + 0.65f) * 28.8f;  // Litres, 28.8 equivalent of Mixer with TES
        const float bath_vol = (0.13f * num_occupants + 0.19f) * 50.8f;  // Assumes shower is present
        const float other_vol = 9.8f * num_occupants + 14;
        //fmt::print("Volumes: Shower: {}, Bath: {}, Other: {}\n", showers_vol, bath_vol, other_vol);
        return showers_vol + bath_vol + other_vol;
    }

    float initHeatCapacity(float house_size) {
        return (250 * house_size) / 3600;
    }

    float initBodyHeatGain(int num_occupants) {
        return (num_occupants * 60) / 1000.0f;
    }

    float initEpcBodyGain(float house_size) {
        const float epc_num_occupants = 1 + 1.76f * (1 - std::exp(-0.000349f *
            std::powf((house_size - 13.9f), 2))) + 0.0013f * (house_size - 13.9f);
        return (epc_num_occupants * 60) / 1000;
    }

    template<typename T>
    int inPostalRegion(const T& postcode_region, const std::array<std::string, 4>& locations) {
        for (const auto& option : postcode_region) {
            // fmt::print("{}, {}, \n", option, locations.at(option.length() - 2));
            if (option == locations.at(option.length() - 1)) {
                return true;
            }
        }
        return false;
    }

    int initPostcodeId(const std::string& location) {
        const std::array<std::string, 4> locations = { location.substr(0, 1), location.substr(0, 2), location.substr(0, 3), location.substr(0, 4) };

        // POSTCODES MUST BE CHECK AGAINST IN THE ORDER OF DEFINITION
        constexpr std::array<std::string_view, 23> postcode_region1 = { "AL", "CM21", "CM22", "CM23", "CR", "EC", "HA", "HP", "KT", "LU", "MK", "NW", "OX", "SE", "SG", "SL", "SM", "SN7", "SW", "TW", "UB", "WC", "WD" };
        constexpr std::array<std::string_view, 24> postcode_region2 = { "BN", "BR", "CT", "DA", "GU28", "GU29", "ME", "PO18", "PO19", "PO20", "PO21", "PO22", "RH10", "RH11", "RH12", "RH13", "RH14", "RH15", "RH16", "RH17", "RH18", "RH19", "RH20", "TN" };
        constexpr std::array<std::string_view, 31> postcode_region3 = { "BH", "DT", "GU11", "GU12", "GU14", "GU30", "GU31", "GU32", "GU33", "GU34", "GU35", "GU46", "GU51", "GU52", "PO", "RG21", "RG22", "RG23", "RG24", "RG25", "RG26", "RG27", "RG28", "RG29", "SO", "SP6", "SP7", "SP8", "SP9", "SP10", "SP11" };
        constexpr std::array<std::string_view, 4> postcode_region4 = { "EX", "PL", "TQ", "TR" };
        constexpr std::array<std::string_view, 5> postcode_region5 = { "BA", "BS", "CF", "GL", "TA" };
        constexpr std::array<std::string_view, 24> postcode_region6 = { "CV", "DE", "DY", "HR", "LE", "NN", "S18", "S32", "S33", "S40", "S41", "S42", "S43", "S44", "S45", "SK13", "SK17", "SK22", "SK23", "ST", "TF", "WR", "WS", "WV" };
        constexpr std::array<std::string_view, 10> postcode_region7 = { "BB", "BL", "CH", "CW", "FY", "OL", "PR", "SY14", "WA", "WN" };
        constexpr std::array<std::string_view, 19> postcode_region8 = { "CA", "DG", "LA7", "LA8", "LA9", "LA10", "LA11", "LA12", "LA13", "LA14", "LA15", "LA16", "LA17", "LA18", "LA19", "LA20", "LA21", "LA22", "LA23" };
        constexpr std::array<std::string_view, 8> postcode_region9 = { "DH4", "DH5", "EH43", "EH44", "EH45", "EH46", "NE", "TD" };
        constexpr std::array<std::string_view, 9> postcode_region10 = { "BD23", "BD24", "DH", "DL", "HG", "LS24", "SR7", "SR8", "TS" };
        constexpr std::array<std::string_view, 22> postcode_region11 = { "BD", "DN", "HD", "HU", "HX", "LN", "LS", "NG", "PE9", "PE10", "PE11", "PE12", "PE20", "PE21", "PE22", "PE23", "PE24", "PE25", "WF", "YO15", "YO16", "YO25" };
        constexpr std::array<std::string_view, 10> postcode_region12 = { "CB", "CM", "CO", "EN9", "IG", "IP", "NR", "PE", "RM", "SS" };
        constexpr std::array<std::string_view, 105> postcode_region13 = { "LD", "LL23", "LL24", "LL25", "LL26", "LL27", "LL30", "LL31", "LL32", "LL33", "LL34", "LL35", "LL36", "LL37", "LL38", "LL39", "LL40", "LL41", "LL42", "LL43", "LL44", "LL45", "LL46", "LL47", "LL48", "LL49", "LL50", "LL51", "LL52", "LL53", "LL54", "LL55", "LL56", "LL57", "LL58", "LL59", "LL60", "LL61", "LL62", "LL63", "LL64", "LL65", "LL66", "LL67", "LL68", "LL69", "LL70", "LL71", "LL72", "LL73", "LL74", "LL75", "LL76", "LL77", "LL78", "NP8", "SA14", "SA15", "SA16", "SA17", "SA18", "SA19", "SA20", "SA31", "SA32", "SA33", "SA34", "SA35", "SA36", "SA37", "SA38", "SA39", "SA40", "SA41", "SA42", "SA43", "SA44", "SA45", "SA46", "SA47", "SA48", "SA61", "SA62", "SA63", "SA64", "SA65", "SA66", "SA67", "SA68", "SA69", "SA70", "SA71", "SA72", "SA73", "SY15", "SY16", "SY17", "SY18", "SY19", "SY20", "SY21", "SY22", "SY23", "SY24", "SY25" };
        constexpr std::array<std::string_view, 6> postcode_region14 = { "FK", "KA", "ML", "PA", "PH49", "PH50" };
        constexpr std::array<std::string_view, 3> postcode_region15 = { "DD", "EH", "KY" };
        constexpr std::array<std::string_view, 6> postcode_region16 = { "AB", "IV30", "IV31", "IV32", "IV36", "PH26" };
        constexpr std::array<std::string_view, 23> postcode_region17 = { "IV", "PH19", "PH20", "PH21", "PH22", "PH23", "PH24", "PH25", "PH30", "PH31", "PH32", "PH33", "PH34", "PH35", "PH36", "PH37", "PH38", "PH39", "PH40", "PH41", "PH42", "PH43", "PH44" };
        constexpr std::array<std::string_view, 1> postcode_region18 = { "HS" };
        constexpr std::array<std::string_view, 3> postcode_region19 = { "KW15", "KW16", "KW17" };
        constexpr std::array<std::string_view, 1> postcode_region20 = { "ZE" };
        constexpr std::array<std::string_view, 1> postcode_region21 = { "BT" };

        constexpr std::array<std::string_view, 4> postcode_region5b = { "NP", "SA", "SN", "SP" };
        constexpr std::array<std::string_view, 7> postcode_region1b = { "E", "EN", "GU", "N", "RG", "RH", "W" };
        constexpr std::array<std::string_view, 2> postcode_region6b = { "B", "SY" };
        constexpr std::array<std::string_view, 5> postcode_region7b = { "L", "LA", "LL", "M", "SK" };
        constexpr std::array<std::string_view, 1> postcode_region9b = { "SR" };
        constexpr std::array<std::string_view, 1> postcode_region10b = { "YO" };
        constexpr std::array<std::string_view, 1> postcode_region11b = { "S" };
        constexpr std::array<std::string_view, 1> postcode_region14b = { "G" };
        constexpr std::array<std::string_view, 1> postcode_region15b = { "PH" };
        constexpr std::array<std::string_view, 1> postcode_region17b = { "KW" };

        // POSTAL REGIONS MUST BE CHECKED IN THIS ORDER
        if (inPostalRegion(postcode_region1, locations)) return 0;
        if (inPostalRegion(postcode_region2, locations)) return 1;
        if (inPostalRegion(postcode_region3, locations)) return 2;
        if (inPostalRegion(postcode_region4, locations)) return 3;
        if (inPostalRegion(postcode_region5, locations)) return 4;
        if (inPostalRegion(postcode_region6, locations)) return 5;
        if (inPostalRegion(postcode_region7, locations)) return 6;
        if (inPostalRegion(postcode_region8, locations)) return 7;
        if (inPostalRegion(postcode_region9, locations)) return 8;
        if (inPostalRegion(postcode_region10, locations)) return 9;
        if (inPostalRegion(postcode_region11, locations)) return 10;
        if (inPostalRegion(postcode_region12, locations)) return 11;
        if (inPostalRegion(postcode_region13, locations)) return 12;
        if (inPostalRegion(postcode_region14, locations)) return 13;
        if (inPostalRegion(postcode_region15, locations)) return 14;
        if (inPostalRegion(postcode_region16, locations)) return 15;
        if (inPostalRegion(postcode_region17, locations)) return 16;
        if (inPostalRegion(postcode_region18, locations)) return 17;
        if (inPostalRegion(postcode_region19, locations)) return 18;
        if (inPostalRegion(postcode_region20, locations)) return 19;
        if (inPostalRegion(postcode_region21, locations)) return 20;

        if (inPostalRegion(postcode_region5b, locations)) return 4;
        if (inPostalRegion(postcode_region1b, locations)) return 0;
        if (inPostalRegion(postcode_region6b, locations)) return 5;
        if (inPostalRegion(postcode_region7b, locations)) return 6;
        if (inPostalRegion(postcode_region9b, locations)) return 8;
        if (inPostalRegion(postcode_region10b, locations)) return 9;
        if (inPostalRegion(postcode_region11b, locations)) return 10;
        if (inPostalRegion(postcode_region14b, locations)) return 13;
        if (inPostalRegion(postcode_region15b, locations)) return 14;
        if (inPostalRegion(postcode_region17b, locations)) return 16;
        return -1;
    }

    std::array<float, 12> initEpcOutsideTemp() {
        constexpr std::array<std::array<float, 12>, 21> epc_outside_temps = { {
                { 5.1f, 5.6f, 7.4f, 9.9f, 13.0f, 16.0f, 17.9f, 17.8f, 15.2f, 11.6f, 8.0f, 5.1f },
                { 5.0f, 5.4f, 7.1f, 9.5f, 12.6f, 15.4f, 17.4f, 17.5f, 15.0f, 11.7f, 8.1f, 5.2f },
                { 5.4f, 5.7f, 7.3f, 9.6f, 12.6f, 15.4f, 17.3f, 17.3f, 15.0f, 11.8f, 8.4f, 5.5f },
                { 6.1f, 6.4f, 7.5f, 9.3f, 11.9f, 14.5f, 16.2f, 16.3f, 14.6f, 11.8f, 9.0f, 6.4f },
                { 4.9f, 5.3f, 7.0f, 9.3f, 12.2f, 15.0f, 16.7f, 16.7f, 14.4f, 11.1f, 7.8f, 4.9f },
                { 4.3f, 4.8f, 6.6f, 9.0f, 11.8f, 14.8f, 16.6f, 16.5f, 14.0f, 10.5f, 7.1f, 4.2f },
                { 4.7f, 5.2f, 6.7f, 9.1f, 12.0f, 14.7f, 16.4f, 16.3f, 14.1f, 10.7f, 7.5f, 4.6f },
                { 3.9f, 4.3f, 5.6f, 7.9f, 10.7f, 13.2f, 14.9f, 14.8f, 12.8f, 9.7f, 6.6f, 3.7f },
                { 4.0f, 4.5f, 5.8f, 7.9f, 10.4f, 13.3f, 15.2f, 15.1f, 13.1f, 9.7f, 6.6f, 3.7f },
                { 4.0f, 4.6f, 6.1f, 8.3f, 10.9f, 13.8f, 15.8f, 15.6f, 13.5f, 10.1f, 6.7f, 3.8f },
                { 4.3f, 4.9f, 6.5f, 8.9f, 11.7f, 14.6f, 16.6f, 16.4f, 14.1f, 10.6f, 7.1f, 4.2f },
                { 4.7f, 5.2f, 7.0f, 9.5f, 12.5f, 15.4f, 17.6f, 17.6f, 15.0f, 11.4f, 7.7f, 4.7f },
                { 5.0f, 5.3f, 6.5f, 8.5f, 11.2f, 13.7f, 15.3f, 15.3f, 13.5f, 10.7f, 7.8f, 5.2f },
                { 4.0f, 4.4f, 5.6f, 7.9f, 10.4f, 13.0f, 14.5f, 14.4f, 12.5f, 9.3f, 6.5f, 3.8f },
                { 3.6f, 4.0f, 5.4f, 7.7f, 10.1f, 12.9f, 14.6f, 14.5f, 12.5f, 9.2f, 6.1f, 3.2f },
                { 3.3f, 3.6f, 5.0f, 7.1f, 9.3f, 12.2f, 14.0f, 13.9f, 12.0f, 8.8f, 5.7f, 2.9f },
                { 3.1f, 3.2f, 4.4f, 6.6f, 8.9f, 11.4f, 13.2f, 13.1f, 11.3f, 8.2f, 5.4f, 2.7f },
                { 5.2f, 5.0f, 5.8f, 7.6f, 9.7f, 11.8f, 13.4f, 13.6f, 12.1f, 9.6f, 7.3f, 5.2f },
                { 4.4f, 4.2f, 5.0f, 7.0f, 8.9f, 11.2f, 13.1f, 13.2f, 11.7f, 9.1f, 6.6f, 4.3f },
                { 4.6f, 4.1f, 4.7f, 6.5f, 8.3f, 10.5f, 12.4f, 12.8f, 11.4f, 8.8f, 6.5f, 4.6f },
                { 4.8f, 5.2f, 6.4f, 10.9f, 13.5f, 15.0f, 14.9f, 13.1f, 10.0f, 7.2f, 4.7f }
        } };
        return epc_outside_temps.at(postcode_id);
    }

    std::array<int, 12> initEpcSolarIrradiance() {
        constexpr std::array<std::array<int, 12>, 21> epc_solar_irradiances = { {
            { 30, 56, 98, 157, 195, 217, 203, 173, 127, 73, 39, 24 },
            { 32, 59, 104, 170, 208, 231, 216, 182, 133, 77, 41, 25 },
            { 35, 62, 109, 172, 209, 235, 217, 185, 138, 80, 44, 27 },
            { 36, 63, 111, 174, 210, 233, 204, 182, 136, 78, 44, 28 },
            { 32, 59, 105, 167, 201, 226, 206, 175, 130, 74, 40, 25 },
            { 28, 55, 97, 153, 191, 208, 194, 163, 121, 69, 35, 23 },
            { 24, 51, 95, 152, 191, 203, 186, 152, 115, 65, 31, 20 },
            { 23, 51, 95, 157, 200, 203, 194, 156, 113, 62, 30, 19 },
            { 23, 50, 92, 151, 200, 196, 187, 153, 11, 61, 30, 18 },
            { 25, 51, 95, 152, 196, 198, 190, 156, 115, 64, 32, 20 },
            { 26, 54, 96, 150, 192, 200, 189, 157, 115, 66, 33, 21 },
            { 30, 58, 101, 165, 203, 220, 206, 173, 128, 74, 39, 24 },
            { 29, 57, 104, 164, 205, 220, 199, 167, 120, 68, 35, 22 },
            { 19, 46, 88, 148, 196, 193, 185, 150, 101, 55, 25, 15 },
            { 21, 46, 89, 146, 198, 191, 183, 150, 106, 57, 27, 15 },
            { 19, 45, 89, 143, 194, 188, 177, 144, 101, 54, 25, 14 },
            { 17, 43, 85, 145, 189, 185, 170, 139, 98, 51, 22, 12 },
            { 16, 41, 87, 155, 205, 206, 185, 148, 101, 51, 21, 11 },
            { 14, 39, 84, 143, 205, 201, 178, 145, 100, 50, 19, 9 },
            { 12, 34, 79, 135, 196, 190, 168, 144, 90, 46, 16, 7 },
            { 24, 52, 96, 155, 201, 198, 183, 150, 107, 61, 30, 18 }
        } };
        return epc_solar_irradiances.at(postcode_id);
    }

    std::array<float, 12> initSolarHeightFactors(float latitude) {
        // solar_declination_current sdc
        std::array<float, 12> solar_height_factors = {};
        size_t i = 0;
        for (float sdc : solar_declination) {
            solar_height_factors.at(i) = std::cosf((PI / 180.0f) * (latitude - sdc));
            ++i;
        }
        return solar_height_factors;
    }

    std::array<float, 12> initRatiosSgSouth() {
        std::array<float, 12> ratios_sg_south = {};
        size_t i = 0;
        for (float solar_height_factor : solar_height_factors) {
            ratios_sg_south.at(i) = asg_s * solar_height_factor * solar_height_factor + bsg_s * solar_height_factor + csg_s;
            ++i;
        }
        return ratios_sg_south;
    }

    std::array<float, 12> initRatiosSgNorth() {
        std::array<float, 12> ratios_sg_north = {};
        size_t i = 0;
        for (float solar_height_factor : solar_height_factors) {
            ratios_sg_north.at(i) = asg_n * solar_height_factor * solar_height_factor + bsg_n * solar_height_factor + csg_n;
            ++i;
        }
        return ratios_sg_north;
    }

    std::array<float, 12> initIncidentIrradiancesSgSouth() {
        std::array<float, 12> incident_irradiances_sg_south;
        for (size_t i = 0; i < 12; ++i) {
            incident_irradiances_sg_south.at(i) = epc_solar_irradiance[i] * ratios_sg_south[i];
        }
        return incident_irradiances_sg_south;
    }

    std::array<float, 12> initIncidentIrradiancesSgNorth() {
        std::array<float, 12> incident_irradiances_sg_north;
        for (size_t i = 0; i < 12; ++i) {
            incident_irradiances_sg_north.at(i) = epc_solar_irradiance[i] * ratios_sg_north[i];
        }
        return incident_irradiances_sg_north;
    }

    float initSolarGainHouseFactor(float house_size) {
        return (house_size * 0.15f / 2) * 0.77f * 0.7f * 0.76f * 0.9f / 1000;
    }

    std::array<float, 12> initSolarGainsSouth() {
        std::array<float, 12> solar_gains_south = {};
        size_t i = 0;
        for (float incident_irradiance_sg_south : incident_irradiances_sg_south) {
            solar_gains_south.at(i) = solar_gain_house_factor * incident_irradiance_sg_south;
            ++i;
        }
        return solar_gains_south;
    }

    std::array<float, 12> initSolarGainsNorth() {
        std::array<float, 12> solar_gains_north = {};
        size_t i = 0;
        for (float incident_irradiance_sg_north : incident_irradiances_sg_north) {
            solar_gains_north.at(i) = solar_gain_house_factor * incident_irradiance_sg_north;
            ++i;
        }
        return solar_gains_north;
    }

    void calcEpcDay(const std::array<int, 24>& epc_temp_profile, float& inside_temp_current, float outside_temp_current, float thermal_transmittance_current, float solar_gain_south, float solar_gain_north, float& epc_demand) {

        for (size_t hour = 0; hour < 24; ++hour) {
            float desired_temp_current = static_cast<float>(epc_temp_profile.at(hour));
            const float heat_flow_out = (house_size * thermal_transmittance_current * (inside_temp_current - outside_temp_current)) / 1000;

            // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
            inside_temp_current += (-heat_flow_out + solar_gain_south + solar_gain_north + epc_body_gain) / heat_capacity;
            if (inside_temp_current < desired_temp_current) {  //  Requires heating
                const float space_hr_demand = (desired_temp_current - inside_temp_current) * heat_capacity;
                inside_temp_current = desired_temp_current;
                //fmt::print("i{}\n", inside_temp_current);
                
                epc_demand += space_hr_demand / 0.9f;
                
            }
            //std::cout << epc_demand << '\n';
            //fmt::print("{:.4f}, {:.4f}, {:.4f}\n", heat_flow_out, inside_temp_current, epc_demand);
        }
    }

    void calcEpcMonth(int month, int num_days, float thermal_transmittance_current, float& inside_temp_current, float& epc_demand) {
        float outside_temp_current = epc_outside_temp.at(month);
        int solar_irradiance_current = epc_solar_irradiance.at(month);
        float solar_height_factor = solar_height_factors.at(month);
        float solar_declination_current = solar_declination.at(month);
        float solar_gain_south = solar_gains_south.at(month);
        float solar_gain_north = solar_gains_north.at(month);

        // fmt::print("{:.2f}, {}, {:.2f}, {:.2f}, {:.4f}, {:.2f}\n", outside_temp_current, solar_irradiance_current, solar_height_factor, solar_declination_current, solar_gain_south, solar_gain_north);
        //fmt::print("{:.4f}, {:.4f}, {:.4f}", ratio_sg_south, incident_irradiance_sg_south, solar_gain_south);

        for (size_t day = 0; day < num_days; ++day) {
            const std::array<int, 24>* epc_temp_profile;
            if (month >= 5 && month <= 8) { // summer no heating
                epc_temp_profile = &epc_temp_profile_summer;
            }
            else if (day % 7 >= 5) { // weekend not summer
                epc_temp_profile = &epc_temp_profile_weekend;
            }
            else { // weekday not s
                epc_temp_profile = &epc_temp_profile_other;
            }

            calcEpcDay(*epc_temp_profile, inside_temp_current, outside_temp_current, thermal_transmittance_current, solar_gain_south, solar_gain_north, epc_demand);
        }
        //std::cout << epc_demand << '\n';
    }

    void calcEpcYear() {
        float thermal_transmittance = 0.5;
        float optimised_epc_demand = 0;

        constexpr std::array<int, 12> days_in_months = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

        // thermal_transmittance_current == ttc
        for (float thermal_transmittance_current = 0.5f; thermal_transmittance_current < 3.0f; thermal_transmittance_current += 0.01f) {
            int month = 0;
            float inside_temp_current = 20;  // Initial temperature
            float epc_demand = 0;

            for (int days_in_month : days_in_months) {
                calcEpcMonth(month, days_in_month, thermal_transmittance_current, inside_temp_current, epc_demand);
                ++month;
            }

            const float epc_optimal_heating_demand_diff = std::abs(epc_space_heating - optimised_epc_demand);
            const float epc_heating_demand_diff = std::abs(epc_space_heating - epc_demand);

            // std::cout << epc_space_heating << ' ' << optimised_epc_demand << ' ' << epc_demand << '\n';

            if (epc_heating_demand_diff < epc_optimal_heating_demand_diff) {
                optimised_epc_demand = epc_demand;
                thermal_transmittance = thermal_transmittance_current;
            }
            else { // if the epc heating demand difference is increasing the most optimal has already been found
                break;
            }
            //fmt::print("{:.2f}, {:.2f}, {}, {:.2f}, {:.2f}\n", epc_demand, optimised_epc_demand, epc_space_heating, epc_heating_demand_diff, epc_optimal_heating_demand_diff);
            //fmt::print("EPC Demand: {:.2f}\n", epc_demand);
            //fmt::print("Inside Temperature: {:.2f}\n", inside_temp_current);
        }

        std::cout << "Dwelling Thermal Transmittance: " << thermal_transmittance << '\n';
        std::cout << "Optimised EPC Demand: " << optimised_epc_demand << '\n';
        //fmt::print("Dwelling Thermal Transmittance: {:.2f}\n", thermal_transmittance);
        //fmt::print("Optimised EPC Demand: {:.2f}\n", optimised_epc_demand);
        this->thermal_transmittance = thermal_transmittance;
    }

    void calcDemandDay(const std::array<float, 24>& temp_profile, float& inside_temp_current, float ratio_sg_south, float ratio_sg_north, float cwt_current, float dhw_mf_current, float& demand_total, float& dhw_total, float& max_hourly_demand, size_t& hour_year_counter) {
        for (size_t hour = 0; hour < 24; ++hour) {
            float desired_temp_current = static_cast<float>(temp_profile.at(hour));
            float dhw_hr_current = dhw_hourly_ratios.at(hour);

            //std::string outside_temp_str;

            //std::getline(outside_temps_file, outside_temp_str);
            //fmt::print("hour: {}, str: {}, {}\n", hour, outside_temp_str, outside_temps_file.tellg());
            //std::cout << "inside calcDemandDay" << '\n';
            float outside_temp_current = outside_temps.at(hour_year_counter);

            //std::cout << "outside_temp_current: " << outside_temp_current << ", " << hour_year_counter << '\n';

            //std::string solar_irradiance_str;
            //std::getline(solar_irradiances_file, solar_irradiance_str);
            float solar_irradiance_current = solar_irradiances.at(hour_year_counter);
            ++hour_year_counter;

            float dhw_hr_demand = (dhw_avg_daily_vol * 4.18f * (hot_water_temp - cwt_current) / 3600) * dhw_mf_current * dhw_hr_current;

            float incident_irradiance_sg_s = solar_irradiance_current * ratio_sg_south;
            float incident_irradiance_sg_n = solar_irradiance_current * ratio_sg_north;
            float solar_gain_south = incident_irradiance_sg_s * solar_gain_house_factor;
            float solar_gain_north = incident_irradiance_sg_n * solar_gain_house_factor;

            //float solar_irradiance_current = Solar_Irradiance[Weather_Count]
            const float heat_loss = (house_size * thermal_transmittance * (inside_temp_current - outside_temp_current)) / 1000;

            // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
            inside_temp_current += (-heat_loss + solar_gain_south + solar_gain_north + body_heat_gain) / heat_capacity;
            
            float space_hr_demand = 0;
            if (inside_temp_current < desired_temp_current) {  //  Requires heating
                space_hr_demand = (desired_temp_current - inside_temp_current) * heat_capacity;
                inside_temp_current = desired_temp_current;
                //fmt::print("i{}\n", inside_temp_current);
            }

            //std::cout << hour << ' ' << space_hr_demand << ' ' << demand_total - dhw_hr_demand << '\n';
            const float hourly_demand = dhw_hr_demand + space_hr_demand;
            max_hourly_demand = std::max(max_hourly_demand, hourly_demand);
            demand_total += hourly_demand;
            dhw_total += dhw_hr_demand;
            //fmt::print("{:.4f}, {:.4f}, {:.4f}, {:.4f}, {:.4f}, {:.4f}\n", heat_loss, inside_temp_current, demand_total, dhw_total, dhw_hr_demand, solar_irradiance_current);
        }
    }

    void calcDemandMonth(int month, int num_days, const std::array<float, 24>& temp_profile, float inside_temp_current, float& demand_total, float& dhw_total, float& max_hourly_demand, size_t& hour_year_counter) {
        const float dhw_mf_current = dhw_monthly_factor.at(month);
        const float cwt_current = cold_water_temp.at(month);
        const float ratio_sg_south = ratios_sg_south.at(month);
        const float ratio_sg_north = ratios_sg_north.at(month);

        for (size_t day = 0; day < num_days; ++day) {
            //fmt::print("day: {}", day);
            //std::cout << day << '\n';
            calcDemandDay(temp_profile, inside_temp_current, ratio_sg_south, ratio_sg_north, cwt_current, dhw_mf_current, demand_total, dhw_total, max_hourly_demand, hour_year_counter);
        }
    }

    Demand calcDemandYear(const std::array<float, 24>& temp_profile) {
        // temp_profile either for hp or non-hp / boiler
        // temp_profiles: HP_Temp_Profile, Temp_Profile
        size_t hour_year_counter = 0;

        float max_hourly_demand = 0;
        float demand_total = 0;

        float inside_temp_current = temp;
        int weather_count = 0;
        float dhw_total = 0;

        constexpr std::array<int, 12> days_in_months = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

        int month = 0;
        for (int days_in_month : days_in_months) {
            //fmt::print("month: {}", month);
            //std::cout << "m " << month << '\n';
            calcDemandMonth(month, days_in_month, temp_profile, inside_temp_current, demand_total, dhw_total, max_hourly_demand, hour_year_counter);
            ++month;
        }

        const float space_demand = demand_total - dhw_total;
        std::cout << "Yearly Hot Water Demand: " + to_string_with_precision(dhw_total, 4) << +" kWh\n";
        std::cout << "Yearly Space demand: " + to_string_with_precision(space_demand, 4) << +" kWh\n";
        std::cout << "Yearly Total demand: " + to_string_with_precision(demand_total, 4) << +" kWh\n";
        std::cout << "Max hourly demand: " + to_string_with_precision(max_hourly_demand, 4) << +" kWh\n";
        //fmt::print("Space demand: {:.2f} kWh\n", space_demand);
        //fmt::print("Yearly total thermal demand: {:.2f} kWh\n", demand_total);
        //fmt::print("Max hourly demand: {:.2f} kWh\n", max_hourly_demand);
        return { demand_total , max_hourly_demand };
    }


    //void initOutsideTempFile() {
    //    const float lat_rounded = std::roundf(latitude * 2) / 2;
    //    const float lon_rounded = std::roundf(longitude * 2) / 2;
    //    const std::string filename = fmt::format("outside_temps\\lat_{:.1f}_lon_{:.1f}.csv", lat_rounded, lon_rounded);
    //    fmt::print("{}, {}\n", std::filesystem::current_path().string(), filename);
    //    std::filesystem::path path(filename);
    //    if (!std::filesystem::exists(path)) {
    //        throw std::runtime_error("File does not exist");
    //    }

    //    outside_temps_file.open(filename);
    //}

    //void initSolarIrradiancesFile() {
    //    const float lat_rounded = std::roundf(latitude * 2) / 2;
    //    const float lon_rounded = std::roundf(longitude * 2) / 2;
    //    const std::string filename = fmt::format("solar_irradiances\\lat_{:.1f}_lon_{:.1f}.csv", lat_rounded, lon_rounded);
    //    fmt::print("{}, {}\n", std::filesystem::current_path().string(), filename);
    //    std::filesystem::path path(filename);
    //    if (!std::filesystem::exists(path)) {
    //        throw std::runtime_error("File does not exist");
    //    }

    //    solar_irradiances_file.open(filename);
    //}

    //void initAgileTariffFile() {
    //    const float lat_rounded = std::roundf(latitude * 2) / 2;
    //    const float lon_rounded = std::roundf(longitude * 2) / 2;
    //    const std::string filename = "agile_tariff.csv";
    //    std::filesystem::path path(filename);
    //    if (!std::filesystem::exists(path)) {
    //        throw std::runtime_error("File does not exist");
    //    }

    //    agile_tariff_file.open(filename);
    //}

    float initColdestOutsideTemp() {
        const std::map<std::string, float> coldest_outside_temps = { { "50.0_-3.5", 4.61f }, { "50.0_-4.0", 4.554f }, { "50.0_-4.5", 4.406f }, { "50.0_-5.0", 4.017f }, { "50.0_-5.5", 4.492f }, { "50.5_-0.5", 3.02f }, { "50.5_-1.0", 3.188f }, { "50.5_-1.5", 2.812f }, { "50.5_-2.0", 2.583f }, { "50.5_-2.5", 2.774f }, { "50.5_-3.0", 2.697f }, { "50.5_-3.5", 1.744f }, { "50.5_-4.0", 0.854f }, { "50.5_-4.5", 1.27f }, { "50.5_-5.0", 2.708f }, { "50.5_0.0", 2.886f }, { "50.5_0.5", 2.764f }, { "51.0_-0.5", -3.846f }, { "51.0_-1.0", -4.285f }, { "51.0_-1.5", -4.421f }, { "51.0_-2.0", -4.274f }, { "51.0_-2.5", -3.764f }, { "51.0_-3.0", -2.635f }, { "51.0_-3.5", -1.712f }, { "51.0_-4.0", -0.232f }, { "51.0_-4.5", 1.638f }, { "51.0_0.0", -3.344f }, { "51.0_0.5", -2.101f }, { "51.0_1.0", 0.307f }, { "51.0_1.5", 1.271f }, { "51.5_-0.5", -5.969f }, { "51.5_-1.0", -5.673f }, { "51.5_-1.5", -5.09f }, { "51.5_-2.0", -4.292f }, { "51.5_-2.5", -3.039f }, { "51.5_-3.0", -1.591f }, { "51.5_-3.5", 0.221f }, { "51.5_-4.0", 1.249f }, { "51.5_-4.5", 2.001f }, { "51.5_-5.0", 2.948f }, { "51.5_0.0", -5.628f }, { "51.5_0.5", -4.165f }, { "51.5_1.0", -1.369f }, { "51.5_1.5", 1.813f }, { "52.0_-0.5", -5.601f }, { "52.0_-1.0", -5.283f }, { "52.0_-1.5", -4.854f }, { "52.0_-2.0", -4.37f }, { "52.0_-2.5", -3.7f }, { "52.0_-3.0", -3.597f }, { "52.0_-3.5", -3.13f }, { "52.0_-4.0", -2.297f }, { "52.0_-4.5", -0.642f }, { "52.0_-5.0", 2.044f }, { "52.0_-5.5", 3.622f }, { "52.0_0.0", -5.439f }, { "52.0_0.5", -4.533f }, { "52.0_1.0", -2.836f }, { "52.0_1.5", 0.146f }, { "52.5_-0.5", -4.979f }, { "52.5_-1.0", -4.814f }, { "52.5_-1.5", -4.451f }, { "52.5_-2.0", -3.991f }, { "52.5_-2.5", -3.603f }, { "52.5_-3.0", -3.359f }, { "52.5_-3.5", -3.007f }, { "52.5_-4.0", -0.479f }, { "52.5_-4.5", 2.769f }, { "52.5_0.0", -4.845f }, { "52.5_0.5", -4.0f }, { "52.5_1.0", -3.96f }, { "52.5_1.5", -1.778f }, { "52.5_2.0", 1.576f }, { "53.0_-0.5", -4.434f }, { "53.0_-1.0", -4.51f }, { "53.0_-1.5", -4.234f }, { "53.0_-2.0", -3.806f }, { "53.0_-2.5", -3.409f }, { "53.0_-3.0", -2.964f }, { "53.0_-3.5", -2.419f }, { "53.0_-4.0", -0.304f }, { "53.0_-4.5", 1.987f }, { "53.0_-5.0", 3.827f }, { "53.0_0.0", -4.07f }, { "53.0_0.5", -1.754f }, { "53.0_1.0", 0.277f }, { "53.0_1.5", 1.709f }, { "53.0_2.0", 2.397f }, { "53.5_-0.5", -4.156f }, { "53.5_-1.0", -4.141f }, { "53.5_-1.5", -3.834f }, { "53.5_-2.0", -3.492f }, { "53.5_-2.5", -2.729f }, { "53.5_-3.0", -1.344f }, { "53.5_-3.5", 0.446f }, { "53.5_-4.0", 1.524f }, { "53.5_-4.5", 2.578f }, { "53.5_0.0", -2.173f }, { "53.5_0.5", 1.351f }, { "54.0_-0.5", -2.622f }, { "54.0_-1.0", -3.424f }, { "54.0_-1.5", -3.834f }, { "54.0_-2.0", -3.837f }, { "54.0_-2.5", -2.766f }, { "54.0_-3.0", -0.56f }, { "54.0_-3.5", 1.22f }, { "54.0_-5.5", 3.297f }, { "54.0_-6.0", 1.151f }, { "54.0_-6.5", -1.496f }, { "54.0_-7.0", -3.164f }, { "54.0_-7.5", -3.294f }, { "54.0_-8.0", -2.848f }, { "54.0_0.0", 0.231f }, { "54.5_-0.5", 0.579f }, { "54.5_-1.0", -1.903f }, { "54.5_-1.5", -4.414f }, { "54.5_-2.0", -5.579f }, { "54.5_-2.5", -5.161f }, { "54.5_-3.0", -2.187f }, { "54.5_-3.5", -0.424f }, { "54.5_-4.0", 1.047f }, { "54.5_-4.5", 2.244f }, { "54.5_-5.0", 2.994f }, { "54.5_-5.5", 1.337f }, { "54.5_-6.0", -0.575f }, { "54.5_-6.5", -2.338f }, { "54.5_-7.0", -3.041f }, { "54.5_-7.5", -2.662f }, { "54.5_-8.0", -1.808f }, { "55.0_-1.5", -0.996f }, { "55.0_-2.0", -4.155f }, { "55.0_-2.5", -6.204f }, { "55.0_-3.0", -4.514f }, { "55.0_-3.5", -2.703f }, { "55.0_-4.0", -1.58f }, { "55.0_-4.5", -0.407f }, { "55.0_-5.0", 0.806f }, { "55.0_-5.5", 2.081f }, { "55.0_-6.0", 0.887f }, { "55.0_-6.5", -0.469f }, { "55.0_-7.0", -0.993f }, { "55.0_-7.5", -0.77f }, { "55.5_-1.5", 0.873f }, { "55.5_-2.0", -2.474f }, { "55.5_-2.5", -5.702f }, { "55.5_-3.0", -5.566f }, { "55.5_-3.5", -4.895f }, { "55.5_-4.0", -4.132f }, { "55.5_-4.5", -2.358f }, { "55.5_-5.0", -0.579f }, { "55.5_-5.5", 1.338f }, { "55.5_-6.0", 2.057f }, { "55.5_-6.5", 2.505f }, { "56.0_-2.0", 1.815f }, { "56.0_-2.5", 0.195f }, { "56.0_-3.0", -2.189f }, { "56.0_-3.5", -4.626f }, { "56.0_-4.0", -5.49f }, { "56.0_-4.5", -4.919f }, { "56.0_-5.0", -3.499f }, { "56.0_-5.5", -1.181f }, { "56.0_-6.0", 1.063f }, { "56.0_-6.5", 2.977f }, { "56.5_-2.5", -0.305f }, { "56.5_-3.0", -3.11f }, { "56.5_-3.5", -5.41f }, { "56.5_-4.0", -6.757f }, { "56.5_-4.5", -7.005f }, { "56.5_-5.0", -5.879f }, { "56.5_-5.5", -3.253f }, { "56.5_-6.0", 0.046f }, { "56.5_-6.5", 2.699f }, { "56.5_-7.0", 4.242f }, { "57.0_-2.0", 1.061f }, { "57.0_-2.5", -4.347f }, { "57.0_-3.0", -6.774f }, { "57.0_-3.5", -8.256f }, { "57.0_-4.0", -8.531f }, { "57.0_-4.5", -8.952f }, { "57.0_-5.0", -7.613f }, { "57.0_-5.5", -4.211f }, { "57.0_-6.0", -0.368f }, { "57.0_-6.5", 2.421f }, { "57.0_-7.0", 3.249f }, { "57.0_-7.5", 4.066f }, { "57.5_-2.0", 0.562f }, { "57.5_-2.5", -2.636f }, { "57.5_-3.0", -3.24f }, { "57.5_-3.5", -3.825f }, { "57.5_-4.0", -4.351f }, { "57.5_-4.5", -5.412f }, { "57.5_-5.0", -7.049f }, { "57.5_-5.5", -3.771f }, { "57.5_-6.0", 0.002f }, { "57.5_-6.5", 2.105f }, { "57.5_-7.0", 2.649f }, { "57.5_-7.5", 3.287f }, { "58.0_-3.5", 1.614f }, { "58.0_-4.0", -0.872f }, { "58.0_-4.5", -2.392f }, { "58.0_-5.0", -2.029f }, { "58.0_-5.5", 0.609f }, { "58.0_-6.0", 2.139f }, { "58.0_-6.5", 2.056f }, { "58.0_-7.0", 1.757f }, { "58.5_-3.0", 1.924f }, { "58.5_-3.5", 1.382f }, { "58.5_-4.0", 0.97f }, { "58.5_-4.5", 0.903f }, { "58.5_-5.0", 1.605f }, { "58.5_-5.5", 2.935f }, { "58.5_-6.0", 2.901f }, { "58.5_-6.5", 2.723f }, { "58.5_-7.0", 2.661f }, { "59.0_-2.5", 2.975f }, { "59.0_-3.0", 2.525f }, { "59.0_-3.5", 3.066f }, { "59.5_-1.5", 3.281f }, { "59.5_-2.5", 3.684f }, { "59.5_-3.0", 3.79f }, { "60.0_-1.0", 2.361f }, { "60.0_-1.5", 2.383f }, { "60.5_-1.0", 1.794f }, { "60.5_-1.5", 1.783f }, { "61.0_-1.0", 1.721f }
        };

        const float lat_rounded = std::roundf(latitude * 2) / 2;
        const float lon_rounded = std::roundf(longitude * 2) / 2;
        const std::string key = to_string_with_precision(lat_rounded, 1) + "_" + to_string_with_precision(lon_rounded, 1);
        return coldest_outside_temps.at(key);
    }

    std::vector<float> importDataFile(std::string filename) {
        //std::filesystem::path path(filename);
        //if (!std::filesystem::exists(path)) {
        //
        //    throw std::runtime_error("Path does not exist");
        //}

        //std::cout << "import file: " << filename << '\n';

        std::ifstream infile(filename);
        std::string line;
        float x;
        std::vector<float> data;
        data.reserve(8760);
        int i = 0;
        while (i < 8760)
        {
            std::getline(infile, line);
            //fmt::print("a {}, \n", line);
            std::stringstream ss(line);
            while (ss >> x) {
                //std::cout << 'x' << x << '\n';
                data.push_back(x);
                //fmt::print("b {}, \n", x);
            }
            ++i;
        }
        //fmt::print("\n");
        infile.close();
        return data;
    }

    std::vector<float> importWeatherData(const std::string& data_type) {
        // using vector instead of array because: function exceeds stack size, consider moving some data to heap (C6262)
        // data_type = "outside_temps or solar_irradiances"
        const float lat_rounded = std::roundf(latitude * 2) / 2;
        const float lon_rounded = std::roundf(longitude * 2) / 2;
        const std::string filename = "assets/" + data_type + "/lat_" + to_string_with_precision(lat_rounded, 1) + "_lon_" + to_string_with_precision(lon_rounded, 1) + ".csv";
        //const std::string filename = fmt::format("{}\\lat_{:.1f}_lon_{:.1f}.csv", data_type, lat_rounded, lon_rounded);
        //fmt::print("{}, {}\n", std::filesystem::current_path().string(), filename);
        //std::cout << "filename: " << filename << '\n';
        return importDataFile(filename);
    }

    std::vector<float> importAgileTariff(const std::string& data_type) {
        // using vector instead of array because: function exceeds stack size, consider moving some data to heap (C6262)
        const std::string filename = "assets/agile_tariff.csv";
        return importDataFile(filename);
    }

    enum class HeatOptions : int {
        ERH = 0, // electric resistance heating
        ASHP = 1, // air source heat pump
        GSHP = 2 // ground source heat pump
    };

    enum class SolarOptions : int {
        None = 0,
        PV = 1, // PV = Photovoltaics
        FP = 2, // FP = Flat Plate
        ET = 3, // ET = Evacuate Tube
        FP_PV = 4,
        ET_PV = 5,
        PVT = 6 // PVT = Photovoltaic thermal hybrid solar collector
    };

    struct TesTariffSpecs {
        float total_operational_cost;
        float cap_ex;
        HeatOptions hp_option;
        SolarOptions solar_option;
        int pv_size;
        int solar_thermal_size;
        float tes_volume;
        float net_present_cost;
        float operation_emissions;

        TesTariffSpecs(float total_operational_cost = 0, float cap_ex = 0, HeatOptions hp_option = static_cast<HeatOptions>(0), SolarOptions solar_option = static_cast<SolarOptions>(0), int pv_size = 0, int solar_thermal_size = 0, float tes_volume = 0, float net_present_cost = 0, float operation_emissions = 0) :
            total_operational_cost(total_operational_cost), cap_ex(cap_ex), hp_option(hp_option), solar_option(solar_option), pv_size(pv_size), solar_thermal_size(solar_thermal_size), tes_volume(tes_volume), net_present_cost(net_present_cost), operation_emissions(operation_emissions) {

        }
    };

    //struct HPnSpecs {
    //    int hp_option;
    //    std::array<HeatNinja::TesTariffSpecs, 7> optimum_tes_and_tariff_spec;

    //    HPnSpecs(int hp_option) : hp_option(hp_option) {

    //    }
    //};

    struct HPSolarSpecs {
        HeatOptions hp_option;
        SolarOptions solar_option;
        HeatNinja::TesTariffSpecs optimum_tes_and_tariff_spec;

        HPSolarSpecs(int index) : hp_option(static_cast<HeatOptions>(index / 7)), solar_option(static_cast<SolarOptions>(index % 7)){
            //std::cout << index % 7 << ' ' << index / 7 << '\n';
        }
    };

    void HpOptionLoop(int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec) {
        for (int hp_option = 0; hp_option < 3; ++hp_option) {
            //fmt::print("hp_option {}\n", hp_option);
            //std::cout << "hp_option" << hp_option << "\n";
            
            SolarOptionLoop(static_cast<HeatOptions>(hp_option), solar_maximum, tes_range, ground_temp, optimum_tes_and_tariff_spec);
        }
    }

    void SolarOptionLoop(HeatOptions hp_option, int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec) {
        const std::array<float, 24>* temp_profile;
        switch (hp_option)
        {
        case HeatOptions::ASHP:
        case HeatOptions::GSHP:
            temp_profile = &(this->hp_temp_profile);
            break;
        default:
            temp_profile = &(this->temp_profile);
            break;
        }

        float cop_worst;
        if (hp_option == HeatOptions::ERH) { //Electrical Resistance Heater
            cop_worst = 1;
        }
        else if (hp_option == HeatOptions::ASHP) { // ASHP, source A review of domestic heat pumps
            // hot - coldest water temperature difference (hcwtd)
            const float hcwtd = hot_water_temp - coldest_outside_temp;
            cop_worst = 6.81f - 0.121f * hcwtd + 0.000630f * hcwtd * hcwtd; // ASHP at coldest temp
        }
        else { // GSHP, source A review of domestic heat pumps
            const float hcwtd = hot_water_temp - coldest_outside_temp;
            cop_worst = 8.77f - 0.150f * hcwtd + 0.000734f * hcwtd * hcwtd; // GSHP ~constant temp at 100m
        }

        float hp_electrical_power;
        if (hp_option == HeatOptions::ERH) {  // Electrical Resistance Heater
            hp_electrical_power = boiler_demand.max_hourly;
        }
        else {  // ASHP or GSHP
            hp_electrical_power = hp_demand.max_hourly / cop_worst;
        }

        if (hp_electrical_power * cop_worst < 4.0f) {  // Mitsubishi have 4kWth ASHP
            hp_electrical_power = 4.0f / cop_worst;  // Kensa have 3kWth GSHP
        }
        if (hp_electrical_power > 7.0f) { // Typical maximum size for domestic power
            hp_electrical_power = 7.0f;
        }

        for (int solar_option_int = 0; solar_option_int < 7; ++solar_option_int) {
            SolarOptions solar_option = static_cast<SolarOptions>(solar_option_int);
            float optimum_tes_npc = 1000000;
            TesTariffSpecs current_tes_and_tariff_specs;
            int solar_size_range = solar_maximum / 2;
            if (solar_option == SolarOptions::None) {
                solar_size_range = 1;
            }
            else if (solar_option == SolarOptions::FP_PV || solar_option == SolarOptions::ET_PV) {
                solar_size_range -= 1;
            }
            //std::cout << "solar_option" << solar_option << "\n";
            //fmt::print("    solar_option {}\n", solar_option);
            SolarSizeLoop(hp_option, solar_option, solar_size_range, optimum_tes_npc, solar_maximum, tes_range, cop_worst, hp_electrical_power, ground_temp, current_tes_and_tariff_specs, temp_profile);
            const int index = solar_option_int + static_cast<int>(hp_option) * 7;
            optimum_tes_and_tariff_spec.at(index) = current_tes_and_tariff_specs;
        }
    }

#ifndef USING_EMSCRIPTEN_MACRO
    void HpOptionLoop_Thread(int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec) {
        std::array<TesTariffSpecs, 7> specs1, specs2, specs3;

        std::thread th1([this, solar_maximum, tes_range, ground_temp, &specs1] { this->SolarOptionLoop_Thread(HeatOptions::ERH, solar_maximum, tes_range, ground_temp, specs1); });
        std::thread th2([this, solar_maximum, tes_range, ground_temp, &specs2] { this->SolarOptionLoop_Thread(HeatOptions::ASHP, solar_maximum, tes_range, ground_temp, specs2); });
        std::thread th3([this, solar_maximum, tes_range, ground_temp, &specs3] { this->SolarOptionLoop_Thread(HeatOptions::GSHP, solar_maximum, tes_range, ground_temp, specs3); });
        th1.join();
        th2.join();
        th3.join();

        int i = 0;
        for (auto& spec : specs1) {
            optimum_tes_and_tariff_spec.at(i) = spec;
            ++i;
        }
        for (auto& spec : specs2) {
            optimum_tes_and_tariff_spec.at(i) = spec;
            ++i;
        }
        for (auto& spec : specs3) {
            optimum_tes_and_tariff_spec.at(i) = spec;
            ++i;
        }
    }

    void SolarOptionLoop_Thread(HeatOptions hp_option, int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 7>& optimum_tes_and_tariff_spec) {
        const std::array<float, 24>* temp_profile;
        switch (hp_option)
        {
        case HeatOptions::ASHP:
        case HeatOptions::GSHP:
            temp_profile = &(this->hp_temp_profile);
            break;
        default:
            temp_profile = &(this->temp_profile);
            break;
        }

        float cop_worst;
        if (hp_option == HeatOptions::ERH) { //Electrical Resistance Heater
            cop_worst = 1;
        }
        else if (hp_option == HeatOptions::ASHP) { // ASHP, source A review of domestic heat pumps
            // hot - coldest water temperature difference (hcwtd)
            const float hcwtd = hot_water_temp - coldest_outside_temp;
            cop_worst = 6.81f - 0.121f * hcwtd + 0.000630f * hcwtd * hcwtd; // ASHP at coldest temp
        }
        else { // GSHP, source A review of domestic heat pumps
            const float hcwtd = hot_water_temp - coldest_outside_temp;
            cop_worst = 8.77f - 0.150f * hcwtd + 0.000734f * hcwtd * hcwtd; // GSHP ~constant temp at 100m
        }

        float hp_electrical_power;
        if (hp_option == HeatOptions::ERH) {  // Electrical Resistance Heater
            hp_electrical_power = boiler_demand.max_hourly;
        }
        else {  // ASHP or GSHP
            hp_electrical_power = hp_demand.max_hourly / cop_worst;
        }

        if (hp_electrical_power * cop_worst < 4.0f) {  // Mitsubishi have 4kWth ASHP
            hp_electrical_power = 4.0f / cop_worst;  // Kensa have 3kWth GSHP
        }
        if (hp_electrical_power > 7.0f) { // Typical maximum size for domestic power
            hp_electrical_power = 7.0f;
        }

        for (int solar_option_int = 0; solar_option_int < 7; ++solar_option_int) {
            SolarOptions solar_option = static_cast<SolarOptions>(solar_option_int);
            float optimum_tes_npc = 1000000;
            TesTariffSpecs current_tes_and_tariff_specs;
            int solar_size_range = solar_maximum / 2;
            if (solar_option == SolarOptions::None) {
                solar_size_range = 1;
            }
            else if (solar_option == SolarOptions::FP_PV || solar_option == SolarOptions::ET_PV) {
                solar_size_range -= 1;
            }
            //std::cout << "solar_option" << solar_option << "\n";
            //fmt::print("    solar_option {}\n", solar_option);
            SolarSizeLoop(hp_option, solar_option, solar_size_range, optimum_tes_npc, solar_maximum, tes_range, cop_worst, hp_electrical_power, ground_temp, current_tes_and_tariff_specs, temp_profile);
            //optimum_tes_and_tariff_spec.at(static_cast<size_t>(solar_option) + static_cast<size_t>(hp_option * 7)) = current_tes_and_tariff_specs;
            optimum_tes_and_tariff_spec.at(solar_option_int) = current_tes_and_tariff_specs;
        }
    }

    void HPSolarOptionLoop_ParUnseq(int solar_maximum, float tes_range, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, float ground_temp) {
        std::array<HPSolarSpecs, 21> hp_solar_specs = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
        std::for_each(std::execution::par_unseq, hp_solar_specs.begin(), hp_solar_specs.end(), [&](HPSolarSpecs& hp_solar_spec) {
            HeatOptions hp_option = hp_solar_spec.hp_option;
            SolarOptions solar_option = hp_solar_spec.solar_option;

            const std::array<float, 24>* temp_profile;
            switch (hp_option)
            {
            case HeatOptions::ASHP:
            case HeatOptions::GSHP:
                temp_profile = &(this->hp_temp_profile);
                break;
            default:
                temp_profile = &(this->temp_profile);
                break;
            }

            float cop_worst;
            if (hp_option == HeatOptions::ERH) { //Electrical Resistance Heater
                cop_worst = 1;
            }
            else if (hp_option == HeatOptions::ASHP) { // ASHP, source A review of domestic heat pumps
             // hot - coldest water temperature difference (hcwtd)
                const float hcwtd = hot_water_temp - coldest_outside_temp;
                cop_worst = 6.81f - 0.121f * hcwtd + 0.000630f * hcwtd * hcwtd; // ASHP at coldest temp
            }
            else { // GSHP, source A review of domestic heat pumps
                const float hcwtd = hot_water_temp - coldest_outside_temp;
                cop_worst = 8.77f - 0.150f * hcwtd + 0.000734f * hcwtd * hcwtd; // GSHP ~constant temp at 100m
            }

            float hp_electrical_power;
            if (hp_option == HeatOptions::ERH) {  // Electrical Resistance Heater
                hp_electrical_power = boiler_demand.max_hourly;
            }
            else {  // ASHP or GSHP
                hp_electrical_power = hp_demand.max_hourly / cop_worst;
            }

            if (hp_electrical_power * cop_worst < 4.0f) {  // Mitsubishi have 4kWth ASHP
                hp_electrical_power = 4.0f / cop_worst;  // Kensa have 3kWth GSHP
            }
            if (hp_electrical_power > 7.0f) { // Typical maximum size for domestic power
                hp_electrical_power = 7.0f;
            }

            //for (int solar_option = 0; solar_option < 7; ++solar_option) {
            float optimum_tes_npc = 1000000;
            TesTariffSpecs current_tes_and_tariff_specs;
            int solar_size_range = solar_maximum / 2;
            if (solar_option == SolarOptions::None) {
                solar_size_range = 1;
            }
            else if (solar_option == SolarOptions::FP_PV || solar_option == SolarOptions::ET_PV) {
                solar_size_range -= 1;
            }
            //std::cout << "solar_option" << solar_option << "\n";
            //fmt::print("    solar_option {}\n", solar_option);
            SolarSizeLoop(hp_option, solar_option, solar_size_range, optimum_tes_npc, solar_maximum, tes_range, cop_worst, hp_electrical_power, ground_temp, current_tes_and_tariff_specs, temp_profile);
            //optimum_tes_and_tariff_spec.at(static_cast<size_t>(solar_option) + static_cast<size_t>(hp_option * 7)) = current_tes_and_tariff_specs;
            hp_solar_spec.optimum_tes_and_tariff_spec = current_tes_and_tariff_specs;
            //}
        });

        size_t i = 0;
        for (auto& hp_solar_spec : hp_solar_specs) {
            optimum_tes_and_tariff_spec.at(i) = hp_solar_spec.optimum_tes_and_tariff_spec;
            ++i; 
        }
    }
#endif

    void SolarSizeLoop(HeatOptions hp_option, SolarOptions solar_option, int solar_size_range, float& optimum_tes_npc, int solar_maximum, float tes_range, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& current_tes_and_tariff_specs, const std::array<float, 24>* temp_profile) {
        for (int solar_size = 0; solar_size < solar_size_range; ++solar_size) {
            //fmt::print("        solar_size {}\n", solar_size);
            TesOptionLoop(hp_option, solar_option, solar_size, solar_maximum, tes_range, cop_worst, hp_electrical_power, optimum_tes_npc, ground_temp, current_tes_and_tariff_specs, temp_profile);
        }
    }

    void TesOptionLoop(HeatOptions hp_option, SolarOptions solar_option, int solar_size, int solar_maximum, float tes_range, float cop_worst, float hp_electrical_power, float& optimum_tes_npc, float ground_temp, TesTariffSpecs& current_tes_and_tariff_specs, const std::array<float, 24>* temp_profile) {
        // if (2 <= solar_option) solar_thermal_size = solar_size * 2 + 2 else 0
        const int solar_thermal_size = [&]() -> const int {
            switch (solar_option)
            {
            case SolarOptions::None:
            case SolarOptions::PV:
                return 0;
            default:
                return (solar_size * 2 + 2);
            }
        } ();
        
        //float solar_thermal_size = static_cast<float>((solar_size * 2 + 2) * static_cast<int>(SolarOptions::FP <= solar_option));
        
        int pv_size = 0;
        if (solar_option == SolarOptions::PV || solar_option == SolarOptions::PVT) {
            pv_size = solar_size * 2 + 2;
        }
        else if (solar_option == SolarOptions::FP_PV || solar_option == SolarOptions::ET_PV) {
            pv_size = solar_maximum - solar_thermal_size;
        }

        //std::cout << "solar_thermal_size: " << solar_thermal_size << ", pv_size: " << pv_size << '\n';

        for (int tes_option = 0; tes_option < tes_range; ++tes_option) {
            //fmt::print("            tes_option {}\n", tes_option);
            float optimum_tariff = 1000000;
            for (int tariff = 0; tariff < 5; ++tariff) {
                //fmt::print("                tariff {}\n", tariff);
                TariffLoop(hp_option, solar_option, tes_option, optimum_tariff, solar_thermal_size, pv_size, cop_worst, hp_electrical_power, optimum_tes_npc, current_tes_and_tariff_specs, ground_temp, tariff, temp_profile);
            }
        }
    }

    void TariffLoop(HeatOptions hp_option, SolarOptions solar_option, int tes_option, float& optimum_tariff, int solar_thermal_size, int pv_size, float cop_worst, float hp_electrical_power, float& optimum_tes_npc, TesTariffSpecs& current_tes_and_tariff_specs, float ground_temp, int tariff, const std::array<float, 24>* temp_profile) {
        size_t hour_year_counter = 0;
        //std::cout << "here\n";
        float inside_temp_current = temp;  // Initial temp
        float solar_thermal_generation_total = 0;
        float operational_costs_peak = 0;
        float operational_costs_off_peak = 0;
        float operation_emissions = 0;

        float tes_volume_current = 0.1f + tes_option * 0.1f; // m3
        const float tes_radius = std::pow((tes_volume_current / (2 * PI)), (1.0f / 3.0f));  //For cylinder with height = 2x radius
        const float tes_charge_full = tes_volume_current * 1000 * 4.18f * (hot_water_temp - 40) / 3600; // 40 min temp
        const float tes_charge_boost = tes_volume_current * 1000 * 4.18f * (60 - 40) / 3600; //  # kWh, 60C HP with PV boost
        const float tes_charge_max = tes_volume_current * 1000 * 4.18f * (95 - 40) / 3600; //  # kWh, 95C electric and solar

        const float tes_charge_min = 10 * 4.18f * (hot_water_temp - 10) / 3600; // 10litres hot min amount
        //CWT coming in from DHW re - fill, accounted for by DHW energy out, DHW min useful temperature 40°C
        //Space heating return temperature would also be ~40°C with flow at 51°C
        float tes_state_of_charge = tes_charge_full;  // kWh, for H2O, starts full to prevent initial demand spike
        // https ://www.sciencedirect.com/science/article/pii/S0306261916302045

        constexpr std::array<int, 12> days_in_months = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

        int month = 0;
        for (int days_in_month : days_in_months) {
            float ratio_sg_south = ratios_sg_south.at(month);
            float ratio_sg_north = ratios_sg_north.at(month);
            float cwt_current = cold_water_temp.at(month);
            float dhw_mf_current = dhw_monthly_factor.at(month);

            const float solar_declination_current = solar_declination.at(month);
            float ratio_roof_south = ratios_roof_south.at(month);

            for (size_t day = 0; day < days_in_month; ++day) {
                calcHeaterDay(temp_profile, inside_temp_current, ratio_sg_south, ratio_sg_north, cwt_current, dhw_mf_current, tes_state_of_charge, tes_charge_full, tes_charge_boost, tes_charge_max, tes_radius, ground_temp, hp_option, solar_option, pv_size, solar_thermal_size, hp_electrical_power, tariff, tes_volume_current, operational_costs_peak, operational_costs_off_peak, operation_emissions, solar_thermal_generation_total, ratio_roof_south, tes_charge_min, hour_year_counter);
            }
            ++month;
        }

        // print options
        float hp_electrical_power_worst = hp_electrical_power * cop_worst; // hp option
        // solar option print solar_thermal_size or pv_size
        float total_operational_cost = operational_costs_peak + operational_costs_off_peak; // tariff

        //std::cout << hp_electrical_power_worst << ", " << pv_size << ", " << solar_thermal_size << ", " << tes_volume_current << ", " << operational_costs_peak << ", " << operational_costs_off_peak << '\n';


        if (total_operational_cost < optimum_tariff) {
            optimum_tariff = total_operational_cost;
            float cap_ex;
            if (hp_option == HeatOptions::ERH) { // Electric Heater
                cap_ex = 100; //Small additional cost to a TES, https://zenodo.org/record/4692649#.YQEbio5KjIV
            }
            else if (hp_option == HeatOptions::ASHP) { // ASHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
                cap_ex = (200 + 4750 / std::powf(hp_electrical_power_worst, 1.25f)) * hp_electrical_power_worst + 1500;  // £s
            }
            else {  // GSHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
                cap_ex = (200 + 4750 / std::powf(hp_electrical_power_worst, 1.25f)) * hp_electrical_power_worst + 800 * hp_electrical_power_worst;
            }

            if (solar_option == SolarOptions::PV || solar_option == SolarOptions::FP_PV || solar_option == SolarOptions::ET_PV) { // PV panels installed
                if (pv_size * 0.2f < 4.0f) { // Less than 4kWp
                    cap_ex += pv_size * 0.2f * 1100; // m2 * 0.2kWp / m2 * £1100 / kWp = £
                }
                else {  // Larger than 4kWp lower £ / kWp
                    cap_ex += pv_size * 0.2f * 900; // m2 * 0.2kWp / m2 * £900 / kWp = £
                }
            }
            if (solar_option >= SolarOptions::FP) { // Solar thermal collector
                if (solar_option == SolarOptions::FP || solar_option == SolarOptions::FP_PV) { // Flat plate solar thermal
                    // Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
                    // Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
                    cap_ex += solar_thermal_size * (225 + 270 / (9 * 1.6f)) + 490 + 800 + 800;
                }
                else if (solar_option == SolarOptions::PVT) { // PVT
                    // https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
                    cap_ex += (solar_thermal_size / 1.6f) * (480 + 270 / 9) + 640 + 490 + 800 + 1440;
                }
                else {  // Evacuated tube solar thermal
                    // Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
                    // Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
                    cap_ex += solar_thermal_size * (280 + 270 / (9 * 1.6f)) + 490 + 800 + 800;
                }
            }

            cap_ex += 2068.3f * std::powf(tes_volume_current, 0.553f);
            // Formula based on this data https ://assets.publishing.service.gov.uk/government/uploads/system/uploads/attachment_data/file/545249/DELTA_EE_DECC_TES_Final__1_.pdf

            float net_present_cost_current = cap_ex; // £s
            float discount_rate_current = 1;

            //std::cout << "a: " << net_present_cost_current << ' ' << total_operational_cost << '\n';
            for (int year = 0; year < npc_years; ++year) { // Optimum for 20 years cost
                net_present_cost_current += total_operational_cost / discount_rate_current;
                discount_rate_current *= discount_rate;
            }
            //std::cout << "b: " << net_present_cost_current << ' ' << optimum_tes_npc << '\n';

            if (net_present_cost_current < optimum_tes_npc) {  // Lowest cost TES& tariff for heating tech
                    //For OpEx vs CapEx plots, with optimised TESand tariff
                optimum_tes_npc = net_present_cost_current;
                current_tes_and_tariff_specs = { total_operational_cost, cap_ex, hp_option, solar_option, pv_size, solar_thermal_size, tes_volume_current, net_present_cost_current, operation_emissions };

                // fmt::print("{} {} {} {} {} {} {} {} {} \n", total_operational_cost, cap_ex, hp_option, solar_option, pv_size, solar_thermal_size, tes_volume_current, net_present_cost_current, operation_emissions);
            }
        }
    }

    float ax2bxc(float a, float b, float c, float x) {
        return a * x * x + b * x + c;
    }

    float ax3bx2cxd(float a, float b, float c, float d, float x) {
        const float x2 = x * x;
        const float x3 = x2 * x;
        return a * x3 + b * x2 + c * x + d;
    }

    std::array<float, 12> initRatiosRoofSouth() {
        const float pf = std::sin(PI / 180 * 35 / 2);  // Assume roof is 35° from horizontal
        const float a = ax3bx2cxd(-0.66f, -0.106f, 2.93f, 0, pf);
        const float b = ax3bx2cxd(3.63f, -0.374f, -7.4f, 0, pf);
        const float c = ax3bx2cxd(-2.71f, -0.991f, 4.59f, 1, pf);

        std::array<float, 12> ratios_roof_south = {};
        size_t month = 0;
        for (const float& solar_declination_current : solar_declination) {
            const float solar_height_factor = std::cos(PI / 180 * (latitude - solar_declination_current));
            ratios_roof_south.at(month) = ax2bxc(a, b, c, solar_height_factor);
            ++month;
        }

        return ratios_roof_south;
    }

    void calcHeaterDay(const std::array<float, 24>* temp_profile, float& inside_temp_current, float ratio_sg_south, float ratio_sg_north, float cwt_current, float dhw_mf_current, float& tes_state_of_charge, float tes_charge_full, float tes_charge_boost, float tes_charge_max, float tes_radius, float ground_temp, HeatOptions hp_option, SolarOptions solar_option, int pv_size, int solar_thermal_size, float hp_electrical_power, int tariff, float& tes_volume_current, float& operational_costs_peak, float& operational_costs_off_peak, float& operation_emissions, float& solar_thermal_generation_total, float ratio_roof_south, float tes_charge_min, size_t& hour_year_counter) {
        const float pi_d = PI * tes_radius * 2;
        const float pi_r2 = PI * tes_radius * tes_radius;
        const float pi_d2 = pi_d * tes_radius * 2;

        for (size_t hour = 0; hour < 24; ++hour) {
            float desired_min_temp_current = static_cast<float>(temp_profile->at(hour));
            float dhw_hr_current = dhw_hourly_ratios.at(hour);

            const float outside_temp_current = outside_temps.at(hour_year_counter);
            const float solar_irradiance_current = solar_irradiances.at(hour_year_counter);
            const float agile_tariff_current = agile_tariff.at(hour_year_counter);

            const float dhw_hr_demand = (dhw_avg_daily_vol * 4.18f * (hot_water_temp - cwt_current) / 3600) * dhw_mf_current * dhw_hr_current;

            const float incident_irradiance_sg_s = solar_irradiance_current * ratio_sg_south;
            const float incident_irradiance_sg_n = solar_irradiance_current * ratio_sg_north;
            const float solar_gain_south = incident_irradiance_sg_s * solar_gain_house_factor;
            const float solar_gain_north = incident_irradiance_sg_n * solar_gain_house_factor;

            // Does not account for angle of the sun across the day
            const float incident_irradiance_roof_south = solar_irradiance_current * ratio_roof_south / 1000; // kW / m2

            //float solar_irradiance_current = Solar_Irradiance[Weather_Count]
            //const float heat_loss = (house_size * thermal_transmittance * (inside_temp_current - outside_temp_current)) / 1000;
            const float heat_loss = house_size_thermal_transmittance_product * (inside_temp_current - outside_temp_current);

            // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
            inside_temp_current += (-heat_loss + solar_gain_south + solar_gain_north + body_heat_gain) / heat_capacity;
            //std::cout << inside_temp_current << ' ' << heat_loss << '\n';
            float tes_upper_temperature;
            float tes_lower_temperature;
            float tes_thermocline_height;

            if (tes_state_of_charge <= tes_charge_full) {  // Currently at nominal temperature ranges
                tes_upper_temperature = 51;
                tes_lower_temperature = cwt_current; // Bottom of the tank would still be at CWT,
                tes_thermocline_height = tes_state_of_charge / tes_charge_full; // %, from top down, .25 is top 25 %
            }
            else if (tes_state_of_charge <= tes_charge_boost) { // At boosted temperature ranges
                tes_upper_temperature = 60;
                tes_lower_temperature = 51;
                tes_thermocline_height = (tes_state_of_charge - tes_charge_full) / (tes_charge_boost - tes_charge_full);
            }
            else { // At max tes temperature
                tes_upper_temperature = 95;
                tes_lower_temperature = 60;
                tes_thermocline_height = (tes_state_of_charge - tes_charge_boost) / (tes_charge_max - tes_charge_boost);
            }

            if (tes_thermocline_height < 0) {
                tes_thermocline_height = 0;
            }
            else if (tes_thermocline_height > 1) {
                tes_thermocline_height = 1;
            }
            //tes_thermocline_height = std::max(std::min(tes_thermocline_height, 1.0f), 0.0f);

            const float tes_upper_losses = (tes_upper_temperature - inside_temp_current) * u_value * (pi_d2 * tes_thermocline_height + pi_r2); // losses in kWh
            const float tes_lower_losses = (tes_lower_temperature - inside_temp_current) * u_value * (pi_d2 * (1 - tes_thermocline_height) + pi_r2);
            //const float tes_upper_losses = (tes_upper_temperature - inside_temp_current) * u_value * (pi_d * (tes_thermocline_height * tes_radius * 2) + pi_r2); // losses in kWh
            //const float tes_lower_losses = (tes_lower_temperature - inside_temp_current) * u_value * (pi_d * ((1 - tes_thermocline_height) * tes_radius * 2) + pi_r2);
            const float total_losses = tes_upper_losses + tes_lower_losses;

            //const float total_losses = [&]() -> const float {
            //    const float pi_d = PI * tes_radius * 2;
            //    const float pi_r2 = PI * tes_radius * tes_radius;
            //    const float tes_upper_losses = (tes_upper_temperature - inside_temp_current) * u_value * (pi_d * (tes_thermocline_height * tes_radius * 2) + pi_r2) / 1000; // losses in kWh
            //    const float tes_lower_losses = (tes_lower_temperature - inside_temp_current) * u_value * (pi_d * ((1 - tes_thermocline_height) * tes_radius * 2) + pi_r2) / 1000;
            //    return tes_upper_losses + tes_lower_losses;
            //}();

            tes_state_of_charge -= total_losses;
            inside_temp_current += total_losses / heat_capacity; // TES inside house
            //std::cout << "ict" << inside_temp_current << '\n';

            const float cop_current = [&]() -> const float {
                switch (hp_option)
                {
                case HeatOptions::ERH: //Electric Heater
                    return 1.0f;
                case HeatOptions::ASHP: // ASHP, source A review of domestic heat pumps
                    return ax2bxc(0.00063f, -0.121f, 6.81f, hot_water_temp - outside_temp_current);
                default: // GSHP, source A review of domestic heat pumps
                    return ax2bxc(0.000734f, -0.150f, 8.77f, hot_water_temp - ground_temp);
                }
            }();

            const float cop_boost = [&]() -> const float {
                switch (hp_option)
                {
                case HeatOptions::ERH: //Electric Heater
                    return 1.0f;
                case HeatOptions::ASHP: // ASHP, source A review of domestic heat pumps
                    return ax2bxc(0.00063f, -0.121f, 6.81f, 60 - outside_temp_current);
                default: // GSHP, source A review of domestic heat pumps
                    return ax2bxc(0.000734f, -0.150f, 8.77f, 60 - ground_temp);
                }
            }();
            
            //float cop_current;
            //float cop_boost;
            //switch (hp_option)
            //{
            //case 0: //Electric Heater
            //    cop_current = 1;
            //    cop_boost = 1;
            //    break;
            //case 1: // ASHP, source A review of domestic heat pumps
            //    cop_current = ax2bxc(0.00063f, -0.121f, 6.81f, hot_water_temp - outside_temp_current);
            //    cop_boost = ax2bxc(0.00063f, -0.121f, 6.81f, 60 - outside_temp_current);
            //    break;
            //default: // GSHP, source A review of domestic heat pumps
            //    cop_current = ax2bxc(0.000734f, -0.150f, 8.77f, hot_water_temp - ground_temp);
            //    cop_boost = ax2bxc(0.000734f, -0.150f, 8.77f, 60 - ground_temp);
            //    break;
            //}

            float pv_efficiency;
            switch (solar_option)
            {
            case SolarOptions::PVT: // PVT
                pv_efficiency = (14.7f * (1 - 0.0045f * ((tes_upper_temperature + tes_lower_temperature) / 2.0f - 25))) / 100;
                // https://www.sciencedirect.com/science/article/pii/S0306261919313443#b0175
                break;
            default:
                pv_efficiency = 0.1928f;
                // Technology Library https ://zenodo.org/record/4692649#.YQEbio5KjIV
                // monocrystalline used for domestic
                break;
            }

            float pv_generation_current = pv_size * pv_efficiency * incident_irradiance_roof_south * 0.8f;  // 80 % shading factor

            float solar_thermal_generation_current = 0; // if solar_option < 2
            if (solar_option >= SolarOptions::FP) {
                const float solar_thermal_collector_temperature = (tes_upper_temperature + tes_lower_temperature) / 2;
                // Collector to heat from tes lower temperature to tes upper temperature, so use the average temperature

                float a, b, c;
                if (incident_irradiance_roof_south == 0) {
                    solar_thermal_generation_current = 0;
                }
                else {
                    switch (solar_option)
                    {
                    case SolarOptions::FP: // Flat plate
                    case SolarOptions::FP_PV:
                        // https://www.sciencedirect.com/science/article/pii/B9781782422136000023
                        a = -0.000038f; b = -0.0035f; c = 0.78f;
                        break;
                    case SolarOptions::PVT: // PVT 
                        // https://www.sciencedirect.com/science/article/pii/S0306261919313443#b0175
                        a = -0.0000176f; b = -0.003325f; c = 0.726f;
                        break;
                    default: // Evacuated tube
                        // https://www.sciencedirect.com/science/article/pii/B9781782422136000023
                        a = -0.00002f; b = -0.0009f; c = 0.625f;
                        break;
                    }
                    solar_thermal_generation_current = 0.8f * solar_thermal_size * ax2bxc(a, b, c * incident_irradiance_roof_south, solar_thermal_collector_temperature - outside_temp_current);
                }
                solar_thermal_generation_current = std::max(solar_thermal_generation_current, 0.0f);
                tes_state_of_charge += solar_thermal_generation_current;
                // Dumps any excess solar generated heat to prevent boiling TES
                tes_state_of_charge = std::min(tes_state_of_charge, tes_charge_max);
            }

            float space_hr_demand;

            if (inside_temp_current > desired_min_temp_current) {
                space_hr_demand = 0;
            }
            else {
                space_hr_demand = (desired_min_temp_current - inside_temp_current) * heat_capacity;
                //std::cout << space_hr_demand << ' ' << desired_min_temp_current << ' ' << inside_temp_current << '\n';
                if ((space_hr_demand + dhw_hr_demand) < (tes_state_of_charge + hp_electrical_power * cop_current)) {
                    inside_temp_current = desired_min_temp_current;
                }
                else {
                    if (tes_state_of_charge > 0) { // Priority to space demand over TES charging
                        space_hr_demand = (tes_state_of_charge + hp_electrical_power * cop_current) - dhw_hr_demand;
                    }
                    else {
                        space_hr_demand = (hp_electrical_power * cop_current) - dhw_hr_demand;
                    }
                    inside_temp_current += space_hr_demand / heat_capacity;
                }
            }

            //std::cout << space_hr_demand << ' ' << dhw_hr_demand << ' ' << tes_state_of_charge << '\n';
            float electrical_demand_current;
            // Determines electrical demand for space and dhw demands
            const float space_water_demand = space_hr_demand + dhw_hr_demand;
            if (space_water_demand < tes_state_of_charge) {
                // TES can provide all demand
                
                tes_state_of_charge -= space_water_demand;
                electrical_demand_current = 0;
            }
            else if (space_water_demand < (tes_state_of_charge + hp_electrical_power * cop_current)) {
                if (tes_state_of_charge > 0) {
                    electrical_demand_current = (space_water_demand - tes_state_of_charge) / cop_current;
                    tes_state_of_charge = 0;  // TES needs support so taken to empty if it had any charge
                }
                else {
                    electrical_demand_current = space_water_demand / cop_current;
                }
            }
            else {
                // TES and HP can't meet hour demand
                electrical_demand_current = hp_electrical_power;
                tes_state_of_charge = std::min(0.0f, tes_state_of_charge);
            }

            // Charges TES at off peak electricity times
            if (tes_volume_current > 0 && tes_state_of_charge < tes_charge_full &&
                ((tariff == 0 && 12 < hour && hour < 16) ||
                (tariff == 1 && (hour == 23 || hour < 6)) ||
                (tariff == 2 && 12 < hour && hour < 16) ||
                (tariff == 3 && 0 <= hour && hour < 5) ||
                (tariff == 4 && agile_tariff_current < 9.0f))) {
                // Flat rate and smart tariff charges TES at typical day peak air temperature times
                // GSHP is not affected so can keep to these times too
                if ((tes_charge_full - tes_state_of_charge) < ((hp_electrical_power - electrical_demand_current) * cop_current)) {
                    // Small top up
                    electrical_demand_current += (tes_charge_full - tes_state_of_charge) / cop_current;
                    tes_state_of_charge = tes_charge_full;
                }
                else { // HP can not fully top up in one hour
                    tes_state_of_charge += (hp_electrical_power - electrical_demand_current) * cop_current;
                    electrical_demand_current = hp_electrical_power;
                }
            }
            float pv_remaining_current = pv_generation_current - electrical_demand_current;

            //Boost temperature if any spare PV generated electricity, as reduced cop, raises to nominal temp above first
            const float tes_boost_state_charge_diff = tes_charge_boost - tes_state_of_charge;
            if (pv_remaining_current > 0 && tes_boost_state_charge_diff > 0) {
                if ((tes_boost_state_charge_diff < (pv_remaining_current * cop_boost)) && (tes_boost_state_charge_diff < ((hp_electrical_power - electrical_demand_current) * cop_boost))) {
                    electrical_demand_current += tes_boost_state_charge_diff / cop_boost;
                    tes_state_of_charge = tes_charge_boost;
                }
                else if (pv_remaining_current < hp_electrical_power) {
                    tes_state_of_charge += pv_remaining_current * cop_boost;
                    electrical_demand_current += pv_remaining_current;
                }
                else {
                    tes_state_of_charge += (hp_electrical_power - electrical_demand_current) * cop_boost;
                    electrical_demand_current = hp_electrical_power;
                }
            }

            //std::cout << tes_state_of_charge << ' ' << tes_charge_min << ' ' << electrical_demand_current << '\n';
            if (tes_state_of_charge < tes_charge_min) { // Take back up to 10L capacity if possible no matter what time
                if ((tes_charge_min - tes_state_of_charge) < (hp_electrical_power - electrical_demand_current) * cop_current) {
                    electrical_demand_current += (tes_charge_min - tes_state_of_charge) / cop_current;
                    tes_state_of_charge = tes_charge_min;
                }
                else if (electrical_demand_current < hp_electrical_power) { // Can't take all the way back up to 10L charge
                    tes_state_of_charge += (hp_electrical_power - electrical_demand_current) * cop_current;
                }
            }

            float electrical_import;
            float pv_equivalent_revenue;
            if (pv_generation_current > electrical_demand_current) {
                // Generating more electricity than using
                pv_equivalent_revenue = pv_generation_current - electrical_demand_current;
                electrical_import = 0;
            }
            else {
                pv_equivalent_revenue = 0;
                electrical_import = electrical_demand_current - pv_generation_current;
            }

            // Operational costs summation
            //std::cout << tariff << ' ' << operational_costs_peak << '\n';
            switch (tariff)
            {
            case 0:
                // Flat rate tariff https://www.nimblefins.co.uk/average-cost-electricity-kwh-uk#:~:text=Unit%20Cost%20of%20Electricity%20per,more%20than%20the%20UK%20average
                // Average solar rate https://www.greenmatch.co.uk/solar-energy/solar-panels/solar-panel-grants
                operational_costs_peak += 0.163f * electrical_import - pv_equivalent_revenue * (0.163f + 0.035f) / 2;
                break;
            case 1:
                // Economy 7 tariff, same source as flat rate above
                if (hour < 6 || hour == 23) { // Off Peak
                    operational_costs_off_peak += 0.095f * electrical_import - pv_equivalent_revenue * (0.095f + 0.035f) / 2;
                }
                else { // Peak
                    operational_costs_peak += 0.199f * electrical_import - pv_equivalent_revenue * (0.199f + 0.035f) / 2;
                }
                break;
            case 2:
                // Bulb smart, for East Midlands values 2021
                // https://help.bulb.co.uk/hc/en-us/articles/360017795731-About-Bulb-s-smart-tariff
                if (15 < hour && hour < 19) { // Peak winter times throughout the year
                    operational_costs_peak += 0.2529f * electrical_import - pv_equivalent_revenue * (0.2529f + 0.035f) / 2;
                }
                else { // Off peak
                    operational_costs_off_peak += 0.1279f * electrical_import - pv_equivalent_revenue * (0.1279f + 0.035f) / 2;
                }
                break;
            case 3:
                // Octopus Go EV, LE10 0YE 2012, https://octopus.energy/go/rates/
                // https://www.octopusreferral.link/octopus-energy-go-tariff/
                if (0 <= hour && hour < 5) { // Off Peak
                    operational_costs_off_peak += 0.05f * electrical_import - pv_equivalent_revenue * (0.05f + 0.03f) / 2;
                }
                else { // Peak
                    operational_costs_peak += 0.1533f * electrical_import - pv_equivalent_revenue * (0.1533f + 0.03f) / 2;
                }
                break;
            default:
                // Octopus Agile file 2020
                // 2021 Octopus export rates https ://octopus.energy/outgoing/
                if (agile_tariff_current < 9.0f) { // Off peak, lower range of variable costs
                    operational_costs_off_peak += (agile_tariff_current / 100) * electrical_import - pv_equivalent_revenue * ((agile_tariff_current / 100) + 0.055f) / 2;
                }
                else { // Peak, upper range of variable costs
                    operational_costs_peak += (agile_tariff_current / 100) * electrical_import - pv_equivalent_revenue * ((agile_tariff_current / 100) + 0.055f) / 2;
                }
                break;
            }

            // Operational emissions summation
            // 22.5 average ST
            // from https ://post.parliament.uk/research-briefings/post-pn-0523/
            float operational_emissions_current = solar_thermal_generation_current * 22.5f;

            if (pv_size > 0) {
                operational_emissions_current += (pv_generation_current - pv_equivalent_revenue) * 75 + pv_equivalent_revenue * (75 - grid_emissions);
            }
            // https://www.parliament.uk/globalassets/documents/post/postpn_383-carbon-footprint-electricity-generation.pdf
            // 75 for PV, 75 - Grid_Emissions show emissions saved for the grid or for reducing other electrical bills
            operational_emissions_current += electrical_import * grid_emissions;
            operation_emissions += operational_emissions_current;
            //fmt::print("{}\n", operational_costs_peak);
            //std::cout << "e: " << operational_emissions_current << '\n';
            solar_thermal_generation_total += solar_thermal_generation_current;
            hour_year_counter++;
        }
    }

    std::string initHeaterTesSettings() {
        // HEATER & TES SETTINGS
        //std::cout << "Electrified heating options at annual costs:" << '\n';
        std::array<TesTariffSpecs, 21> optimum_tes_and_tariff_spec;

        float ground_temp = 15 - (latitude - 50) * (4.0f / 9.0f); // Linear regression ground temp across UK at 100m depth
        float tes_range = tes_volume_max / 0.1f;

        int solar_maximum = static_cast<int>(house_size / 8) * 2;  // Quarter of the roof for solar, even number

        house_size_thermal_transmittance_product = house_size * thermal_transmittance / 1000;
        
        
#ifdef USING_EMSCRIPTEN_MACRO
        //SINGLE THREAD
        HpOptionLoop(solar_maximum, tes_range, ground_temp, optimum_tes_and_tariff_spec);
#else
        //MULTI THREADED
        //HpOptionLoop_Thread(solar_maximum, tes_range, ground_temp, optimum_tes_and_tariff_spec);
        // EXECUTION:PAR_UNSEQ
        HPSolarOptionLoop_ParUnseq(solar_maximum, tes_range, optimum_tes_and_tariff_spec, ground_temp);
#endif
        
        std::cout << "\n--- Optimum TES and Net Present Cost per Heating & Solar Option ---";
        std::cout << "\nHP Opt, Solar Opt, PV Size, Solar Size, TES Vol, OPEX, CAPEX, NPC, Emissions\n";

        std::array<std::string, 3> heat_opt_names = { "ERH", "ASHP", "GSHP" };
        std::array<std::string, 7> solar_opt_names = { "None", "PV", "FP", "ET", "FP+PV", "ET+PV", "PVT"};

        for (const auto& s : optimum_tes_and_tariff_spec) {
            //fmt::print("[ {}, {}, {}, {}, {}, {}, {}, {}, {} ]\n", s.total_operational_cost, s.cap_ex, s.hp_option, s.solar_option, s.pv_size, s.solar_thermal_size, s.tes_volume, s.net_present_cost, s.operation_emissions);
            std::cout << heat_opt_names.at(static_cast<int>(s.hp_option)) << ", " << solar_opt_names.at(static_cast<int>(s.solar_option)) << ", " << s.pv_size << ", " << s.solar_thermal_size << ", " << s.tes_volume << ", " << to_string_with_precision(s.total_operational_cost, 0) << ", " << to_string_with_precision(s.cap_ex, 0) << ", " << to_string_with_precision(s.net_present_cost, 0) << ", " << to_string_with_precision(s.operation_emissions, 0) << "\n";
            //fmt::print("total_operational_cost {},\ncap_ex {},\nhp_option {},\nsolar_option {},\npv_size {},\nsolar_thermal_size {},\ntes_volume {},\nnet_present_cost {},\noperation_emissions {} \n\n\n", s.total_operational_cost, s.cap_ex, s.hp_option, s.solar_option, s.pv_size, s.solar_thermal_size, s.tes_volume, s.net_present_cost, s.operation_emissions);
        }

        std::stringstream ss;
        ss << '[';
        int i = 0;
        for (const auto& s : optimum_tes_and_tariff_spec) {
            ss << "[\"" << heat_opt_names.at(static_cast<int>(s.hp_option)) << "\", \"" << solar_opt_names.at(static_cast<int>(s.solar_option)) << "\", " << s.pv_size << ", " << s.solar_thermal_size << ", " << s.tes_volume << ", " << to_string_with_precision(s.total_operational_cost, 0) << ", " << to_string_with_precision(s.cap_ex, 0) << ", " << to_string_with_precision(s.net_present_cost, 0) << ", " << to_string_with_precision(s.operation_emissions, 0) << "]";
            if (i < 20) {
                ss << ',';
            }
            ++i;
        }
        ss << ']';
        std::cout << ss.str() << '\n';
        return ss.str();
    }
};

//std::ostream& operator<<(std::ostream& os, HeatNinja::HeatOptions heat_option)
//{
//    switch (heat_option)
//    {
//    case HeatNinja::HeatOptions::ERH:
//        os << "ERH";
//        break;
//    case HeatNinja::HeatOptions::ASHP:
//        os << "ASHP";
//        break;
//    case HeatNinja::HeatOptions::GSHP:
//        os << "GSHP";
//        break;
//    default:
//        os.setstate(std::ios_base::failbit);
//        break;
//    }
//    return os;
//}

inline const char* cstr(const std::string& message) {
    char* cstr = new char[message.length() + 1];
    std::strcpy(cstr, message.c_str());
    return cstr;
}

extern "C" {
    const char* sim_test_args(const char* postcode_char, float latitude, float longitude, 
        int num_occupants, float house_size, float temp, int epc_space_heating, float tes_volume_max)
    {
        const std::string postcode(postcode_char);

        HeatNinja heat_ninja(num_occupants, postcode, epc_space_heating, house_size,
            tes_volume_max, temp, latitude, longitude);

        return cstr(heat_ninja.initHeaterTesSettings());
    }


    const char* sim_test()
    {
#ifndef USING_EMSCRIPTEN_MACRO
        auto start_time = std::chrono::steady_clock::now();
#endif
        float x = (1 / 3);
        //fmt::print("Program Begun {}\n", x);
        std::cout << "BEGIN " << '\n';
        int num_occupants = 2;
        std::string location = "CV4 7AL";
        int epc_space_heating = 3000;
        float house_size = 60.0;
        float tes_volume_max = 0.5;
        float temp = 20.0;

        // need alternative to pgeocode
        const float latitude = 52.3833f;
        const float longitude = -1.5833f;

        HeatNinja heat_ninja(num_occupants, location, epc_space_heating, house_size,
            tes_volume_max, temp, latitude, longitude);

        std::string output = heat_ninja.initHeaterTesSettings();
        //fmt::print("Program Completed in {}s\n\n", elapsed_time / 1000000000.0);

#ifndef USING_EMSCRIPTEN_MACRO
        auto end_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        std::cout << "ELAPSED TIME: " << elapsed_time / 1000.0 << ' s' << '\n';
#endif
        return cstr(output);
    }
}




///////////////////////////////////////////////////////////////////

std::vector<float> importDataFile(std::string filename) {
    std::ifstream infile(filename);
    std::string line;
    float x;
    std::vector<float> data;
    data.reserve(8760);
    int i = 0;
    while (i < 8760)
    {
        std::getline(infile, line);
        std::stringstream ss(line);
        while (ss >> x) {
            data.push_back(x);
        }
        ++i;
    }
    infile.close();
    return data;
}

void print_outside_temps_c(const char* filename_char) {
    std::string filename(filename_char);
    filename = "assets/outside_temps/" + filename + ".csv";
    std::cout << filename << "\n";
    std::vector<float> data = importDataFile(filename);
    for (float i : data) {
        std::cout << i << ", ";
    }
    std::cout << "\n";
}

std::string vec2str(const std::vector<float>& vec) {
    std::stringstream ss;
    for (float i : vec) {
        ss << i << ",";
    }
    return ss.str();
}



extern "C" {
    const char* return_vector(int x) {
        std::vector<float> vec = { 0.025f, 0.018f, 0.011f, 0.010f, 0.008f, 0.013f, 0.017f, 0.044f, 0.088f, 0.075f, 0.060f, 0.056f, 0.050f, 0.043f, 0.036f, 0.029f, 0.030f, 0.036f, 0.053f, 0.074f, 0.071f, 0.059f, 0.050f, 0.041f };

        return cstr(vec2str(vec));
    };

    int print_outside_temps(const char* filename) {
        print_outside_temps_c(filename);
        return 0;
    }

    int print_example_file(int x) {
        std::cout << "calling print_example_file" << "\n";
        std::string filename = "assets/input.txt";

        std::ifstream file2(filename);
        std::string str;
        while (std::getline(file2, str))
        {
            std::cout << str << "\n";
        }
        return 0;
    }

    int call_class(int x) {
        std::cout << "calling call_class" << "\n";
        Animal dear(x);
        return 0;
    }

    double int_sqrt(int x)
    {
        std::cout << "calling int_sqrt" << "\n";
        return std::sqrt(x);
    }

    int speed_test(int x) {
        const std::array<const int, 24> hours = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
        long long int total = 0;
#ifndef USING_EMSCRIPTEN_MACRO
        auto t1 = std::chrono::high_resolution_clock::now();
#endif

        for (int i = 0; i < 100000; ++i) {
            for (int day = 0; day < 365; ++day) {
                for (int hour = 0; hour < 24; ++hour) {
                    total += hours.at(hour) + x;
                }
            }
            //std::cout << total << '\n';
        }
        std::cout << total << '\n';
        
#ifndef USING_EMSCRIPTEN_MACRO
            auto t2 = std::chrono::high_resolution_clock::now();
            std::cout << total << ' ' << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1) / 1000.0;
#endif

        return 0;
    }
}



int main()
{
    //std::cout << "Water and Space Heating (WASH) Simulator\n";
    //std::cout << int_sqrt(100) << '\n';
    //call_class(67);
    //print_example_file(0);
    //print_outside_temps("lat_50.0_lon_-3.5");
    //std::cout << return_vector(0) << '\n';
    sim_test();
    //speed_test(1);
}
