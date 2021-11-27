#include "heatninja.h"
//#define EM_COMPATIBLE

#include <iostream>
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <map>

#ifndef EM_COMPATIBLE
#include <chrono>
#include <thread>
#include <algorithm>
#include <execution>
#endif

namespace tools {
    std::string printArray(const auto& arr) {
        std::stringstream ss;
        for (const auto& element : arr) {
            ss << element << ", ";
        }
        return ss.str();
    }

    template <typename T>
    std::string to_string_with_precision(const T a_value, const int n)
    {
        std::ostringstream out;
        out.precision(n);
        out << std::fixed << a_value;
        return out.str();
    }

    float ax2bxc(float a, float b, float c, float x) {
        return a * x * x + b * x + c;
    }

    float ax3bx2cxd(float a, float b, float c, float d, float x) {
        const float x2 = x * x;
        const float x3 = x2 * x;
        return a * x3 + b * x2 + c * x + d;
    }
}

HeatNinja::Demand::Demand(float total, float max_hourly)
    : total(total), max_hourly(max_hourly) {

}

HeatNinja::HeatNinja(int num_occupants, const std::string& location, int epc_space_heating,
    float house_size, float tes_volume_max, float temp, float latitude, float longitude)
    : num_occupants(num_occupants),
    location(location),
    epc_space_heating(epc_space_heating),
    house_size(house_size),
    tes_volume_max(std::min(std::max(tes_volume_max, 0.1f), 3.0f)),
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
    ratios_roof_south(initRatiosRoofSouth()),
    cumulative_discount_rate(calculateCumulativeDiscountRate(discount_rate, npc_years))
{
    #ifdef EM_COMPATIBLE
    std::cout << "EM_COMPATIBLE\n";
    #endif
    

    std::cout << "\n--- Energy Performance Certicate Demand ---" << '\n';
    calcEpcYear();
    std::cout << "\n--- Heat Pump Yearly Demand ---" << '\n';
    hp_demand = calcDemandYear(hp_temp_profile);
    std::cout << "\n--- Boiler Yearly Demand ---" << '\n';
    boiler_demand = calcDemandYear(temp_profile);
}

std::array<float, 24> HeatNinja::initTempProfile(float temp) {
    const float temp2 = temp - 2;
    return { temp2, temp2, temp2, temp2, temp2, temp2, temp2, temp, temp, temp, temp, temp,
        temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp2, temp2 };
}

std::array<float, 24> HeatNinja::initHpTempProfile(float temp) {
    return { temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp,
        temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp };
}

std::array<float, 12> HeatNinja::initColdWaterTemp(float latitude) {
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

float HeatNinja::initDhwAvgDailyVol(int num_occupants) {
    const float showers_vol = (0.45f * num_occupants + 0.65f) * 28.8f;  // Litres, 28.8 equivalent of Mixer with TES
    const float bath_vol = (0.13f * num_occupants + 0.19f) * 50.8f;  // Assumes shower is present
    const float other_vol = 9.8f * num_occupants + 14;
    //fmt::print("Volumes: Shower: {}, Bath: {}, Other: {}\n", showers_vol, bath_vol, other_vol);
    return showers_vol + bath_vol + other_vol;
}

float HeatNinja::initHeatCapacity(float house_size) {
    return (250 * house_size) / 3600;
}

float HeatNinja::initBodyHeatGain(int num_occupants) {
    return (num_occupants * 60) / 1000.0f;
}

float HeatNinja::initEpcBodyGain(float house_size) {
    const float epc_num_occupants = 1 + 1.76f * (1 - std::exp(-0.000349f *
        std::powf((house_size - 13.9f), 2))) + 0.0013f * (house_size - 13.9f);
    return (epc_num_occupants * 60) / 1000;
}

struct PostcodeRegion {
    std::string postcode;
    int mininum;
    int maximum;
    int region;
};

int HeatNinja::initPostcodeId(const std::string& postcode) {
    // regioncodes from https://www.bre.co.uk/filelibrary/SAP/2012/SAP-2012_9-92.pdf p177
    const std::array<PostcodeRegion, 169> regioncodes = { { { "ZE", 0, 0, 20 }, { "YO25", 0, 0, 11 }, { "YO", 15, 16, 11 }, { "YO", 0, 0, 10 }, { "WV", 0, 0, 6 }, { "WS", 0, 0, 6 }, { "WR", 0, 0, 6 }, { "WN", 0, 0, 7 }, { "WF", 0, 0, 11 }, { "WD", 0, 0, 1 }, { "WC", 0, 0, 1 }, { "WA", 0, 0, 7 }, { "W", 0, 0, 1 }, { "UB", 0, 0, 1 }, { "TW", 0, 0, 1 }, { "TS", 0, 0, 10 }, { "TR", 0, 0, 4 }, { "TQ", 0, 0, 4 }, { "TN", 0, 0, 2 }, { "TF", 0, 0, 6 }, { "TD15", 0, 0, 9 }, { "TD12", 0, 0, 9 }, { "TD", 0, 0, 9 }, { "TA", 0, 0, 5 }, { "SY", 15, 25, 13 }, { "SY14", 0, 0, 7 }, { "SY", 0, 0, 6 }, { "SW", 0, 0, 1 }, { "ST", 0, 0, 6 }, { "SS", 0, 0, 12 }, { "SR", 7, 8, 10 }, { "SR", 0, 0, 9 }, { "SP", 6, 11, 3 }, { "SP", 0, 0, 5 }, { "SO", 0, 0, 3 }, { "SN7", 0, 0, 1 }, { "SN", 0, 0, 5 }, { "SM", 0, 0, 1 }, { "SL", 0, 0, 1 }, { "SK", 22, 23, 6 }, { "SK17", 0, 0, 6 }, { "SK13", 0, 0, 6 }, { "SK", 0, 0, 7 }, { "SG", 0, 0, 1 }, { "SE", 0, 0, 1 }, { "SA", 61, 73, 13 }, { "SA", 31, 48, 13 }, { "SA", 14, 20, 13 }, { "SA", 0, 0, 5 }, { "S", 40, 45, 6 }, { "S", 32, 33, 6 }, { "S18", 0, 0, 6 }, { "S", 0, 0, 11 }, { "RM", 0, 0, 12 }, { "RH", 10, 20, 2 }, { "RH", 0, 0, 1 }, { "RG", 21, 29, 3 }, { "RG", 0, 0, 1 }, { "PR", 0, 0, 7 }, { "PO", 18, 22, 2 }, { "PO", 0, 0, 3 }, { "PL", 0, 0, 4 }, { "PH50", 0, 0, 14 }, { "PH49", 0, 0, 14 }, { "PH", 30, 44, 17 }, { "PH26", 0, 0, 16 }, { "PH", 19, 25, 17 }, { "PH", 0, 0, 15 }, { "PE", 20, 25, 11 }, { "PE", 9, 12, 11 }, { "PE", 0, 0, 12 }, { "PA", 0, 0, 14 }, { "OX", 0, 0, 1 }, { "OL", 0, 0, 7 }, { "NW", 0, 0, 1 }, { "NR", 0, 0, 12 }, { "NP8", 0, 0, 13 }, { "NP", 0, 0, 5 }, { "NN", 0, 0, 6 }, { "NG", 0, 0, 11 }, { "NE", 0, 0, 9 }, { "N", 0, 0, 1 }, { "ML", 0, 0, 14 }, { "MK", 0, 0, 1 }, { "ME", 0, 0, 2 }, { "M", 0, 0, 7 }, { "LU", 0, 0, 1 }, { "LS24", 0, 0, 10 }, { "LS", 0, 0, 11 }, { "LN", 0, 0, 11 }, { "LL", 30, 78, 13 }, { "LL", 23, 27, 13 }, { "LL", 0, 0, 7 }, { "LE", 0, 0, 6 }, { "LD", 0, 0, 13 }, { "LA", 7, 23, 8 }, { "LA", 0, 0, 7 }, { "L", 0, 0, 7 }, { "KY", 0, 0, 15 }, { "KW", 15, 17, 19 }, { "KW", 0, 0, 17 }, { "KT", 0, 0, 1 }, { "KA", 0, 0, 14 }, { "IV36", 0, 0, 16 }, { "IV", 30, 32, 16 }, { "IV", 0, 0, 17 }, { "IP", 0, 0, 12 }, { "IG", 0, 0, 12 }, { "HX", 0, 0, 11 }, { "HU", 0, 0, 11 }, { "HS", 0, 0, 18 }, { "HR", 0, 0, 6 }, { "HP", 0, 0, 1 }, { "HG", 0, 0, 10 }, { "HD", 0, 0, 11 }, { "HA", 0, 0, 1 }, { "GU", 51, 52, 3 }, { "GU46", 0, 0, 3 }, { "GU", 30, 35, 3 }, { "GU", 28, 29, 2 }, { "GU14", 0, 0, 3 }, { "GU", 11, 12, 3 }, { "GU", 0, 0, 1 }, { "GL", 0, 0, 5 }, { "G", 0, 0, 14 }, { "FY", 0, 0, 7 }, { "FK", 0, 0, 14 }, { "EX", 0, 0, 4 }, { "EN9", 0, 0, 12 }, { "EN", 0, 0, 1 }, { "EH", 43, 46, 9 }, { "EH", 0, 0, 15 }, { "EC", 0, 0, 1 }, { "E", 0, 0, 1 }, { "DY", 0, 0, 6 }, { "DT", 0, 0, 3 }, { "DN", 0, 0, 11 }, { "DL", 0, 0, 10 }, { "DH", 4, 5, 9 }, { "DH", 0, 0, 10 }, { "DG", 0, 0, 8 }, { "DE", 0, 0, 6 }, { "DD", 0, 0, 15 }, { "DA", 0, 0, 2 }, { "CW", 0, 0, 7 }, { "CV", 0, 0, 6 }, { "CT", 0, 0, 2 }, { "CR", 0, 0, 1 }, { "CO", 0, 0, 12 }, { "CM", 21, 23, 1 }, { "CM", 0, 0, 12 }, { "CH", 5, 8, 7 }, { "CH", 0, 0, 7 }, { "CF", 0, 0, 5 }, { "CB", 0, 0, 12 }, { "CA", 0, 0, 8 }, { "BT", 0, 0, 21 }, { "BS", 0, 0, 5 }, { "BR", 0, 0, 2 }, { "BN", 0, 0, 2 }, { "BL", 0, 0, 7 }, { "BH", 0, 0, 3 }, { "BD", 23, 24, 10 }, { "BD", 0, 0, 11 }, { "BB", 0, 0, 7 }, { "BA", 0, 0, 5 }, { "B", 0, 0, 6 }, { "AL", 0, 0, 1 }, { "AB", 0, 0, 16 } } };

    // extract the digit from the outcode
    std::string digits_str = "";
    for (size_t i = 0; i < postcode.size(); ++i) {
        if (isdigit(postcode.at(i))) {
            digits_str += postcode.substr(i, 1);
            if (digits_str.length() > 1) break;
        }
        else if (digits_str.length() > 0) {
            break;
        }
    }
    int digits = std::stoi(digits_str);
    //std::cout << digits << '\n';

    const std::array<std::string, 4> sub_postcodes = { postcode.substr(0, 1), postcode.substr(0, 2), postcode.substr(0, 3), postcode.substr(0, 4) };
    for (auto& regioncode : regioncodes) {
        // search for the outcode in the list of regioncodes
        if (regioncode.postcode == sub_postcodes.at(regioncode.postcode.length() - 1)) {
            // if the region code has a minimum & maximum, check the postcode lies within that range (inclusively).
            if (regioncode.maximum == 0 || (digits >= regioncode.mininum && digits <= regioncode.maximum)) {
                //std::cout << regioncode.postcode << ' ' << regioncode.mininum << ' ' << regioncode.maximum << ' ' << regioncode.region << ' ' << '\n';
                return regioncode.region - 1;
            }
        }
    }
    return -1;
}

std::array<float, 12> HeatNinja::initEpcOutsideTemp() {
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

std::array<int, 12> HeatNinja::initEpcSolarIrradiance() {
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

std::array<float, 12> HeatNinja::initSolarHeightFactors(float latitude) {
    // solar_declination_current sdc
    std::array<float, 12> solar_height_factors = {};
    size_t i = 0;
    for (float sdc : solar_declination) {
        solar_height_factors.at(i) = std::cosf((PI / 180.0f) * (latitude - sdc));
        ++i;
    }
    return solar_height_factors;
}

std::array<float, 12> HeatNinja::initRatiosSgSouth() {
    const float pf_sg = std::sin(PI / 180 * 90 / 2); // Assume windows are vertical, so no in roof windows
    const float asg_s = tools::ax3bx2cxd(-0.66f, -0.106f, 2.93f, 0, pf_sg);
    const float bsg_s = tools::ax3bx2cxd(3.63f, -0.374f, -7.4f, 0, pf_sg);
    const float csg_s = tools::ax3bx2cxd(-2.71f, -0.991f, 4.59f, 1, pf_sg);

    std::array<float, 12> ratios_sg_south = {};
    size_t i = 0;
    for (float solar_height_factor : solar_height_factors) {
        ratios_sg_south.at(i) = tools::ax2bxc(asg_s, bsg_s, csg_s, solar_height_factor);
        ++i;
    }
    return ratios_sg_south;
}

std::array<float, 12> HeatNinja::initRatiosSgNorth() {
    const float pf_sg = std::sin(PI / 180 * 90 / 2); // Assume windows are vertical, so no in roof windows
    const float asg_n = tools::ax3bx2cxd(26.3f, -38.5f, 14.8f, 0, pf_sg);
    const float bsg_n = tools::ax3bx2cxd(-16.5f, 27.3f, -11.9f, 0, pf_sg);
    const float csg_n = tools::ax3bx2cxd(-1.06f, -0.0872f, -0.191f, 1, pf_sg);

    std::array<float, 12> ratios_sg_north = {};
    size_t i = 0;
    for (float solar_height_factor : solar_height_factors) {
        ratios_sg_north.at(i) = tools::ax2bxc(asg_n, bsg_n, csg_n, solar_height_factor);
        ++i;
    }
    return ratios_sg_north;
}

std::array<float, 12> HeatNinja::initIncidentIrradiancesSgSouth() {
    std::array<float, 12> incident_irradiances_sg_south;
    for (size_t i = 0; i < 12; ++i) {
        incident_irradiances_sg_south.at(i) = epc_solar_irradiance[i] * ratios_sg_south[i];
    }
    return incident_irradiances_sg_south;
}

std::array<float, 12> HeatNinja::initIncidentIrradiancesSgNorth() {
    std::array<float, 12> incident_irradiances_sg_north;
    for (size_t i = 0; i < 12; ++i) {
        incident_irradiances_sg_north.at(i) = epc_solar_irradiance[i] * ratios_sg_north[i];
    }
    return incident_irradiances_sg_north;
}

float HeatNinja::initSolarGainHouseFactor(float house_size) {
    return (house_size * 0.15f / 2) * 0.77f * 0.7f * 0.76f * 0.9f / 1000;
}

std::array<float, 12> HeatNinja::initSolarGainsSouth() {
    std::array<float, 12> solar_gains_south = {};
    size_t i = 0;
    for (float incident_irradiance_sg_south : incident_irradiances_sg_south) {
        solar_gains_south.at(i) = solar_gain_house_factor * incident_irradiance_sg_south;
        ++i;
    }
    return solar_gains_south;
}

std::array<float, 12> HeatNinja::initSolarGainsNorth() {
    std::array<float, 12> solar_gains_north = {};
    size_t i = 0;
    for (float incident_irradiance_sg_north : incident_irradiances_sg_north) {
        solar_gains_north.at(i) = solar_gain_house_factor * incident_irradiance_sg_north;
        ++i;
    }
    return solar_gains_north;
}

void HeatNinja::calcEpcDay(const std::array<int, 24>& epc_temp_profile, float& inside_temp_current, float outside_temp_current, float thermal_transmittance_current, float solar_gain_south, float solar_gain_north, float& epc_demand) {

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

void HeatNinja::calcEpcMonth(int month, int num_days, float thermal_transmittance_current, float& inside_temp_current, float& epc_demand) {
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

void HeatNinja::calcEpcYear() {
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

void HeatNinja::calcDemandDay(const std::array<float, 24>& temp_profile, float& inside_temp_current, float ratio_sg_south, float ratio_sg_north, float cwt_current, float dhw_mf_current, float& demand_total, float& dhw_total, float& max_hourly_demand, size_t& hour_year_counter) {
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

void HeatNinja::calcDemandMonth(int month, int num_days, const std::array<float, 24>& temp_profile, float inside_temp_current, float& demand_total, float& dhw_total, float& max_hourly_demand, size_t& hour_year_counter) {
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

HeatNinja::Demand HeatNinja::calcDemandYear(const std::array<float, 24>& temp_profile) {
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
    std::cout << "Yearly Hot Water Demand: " + tools::to_string_with_precision(dhw_total, 4) << +" kWh\n";
    std::cout << "Yearly Space demand: " + tools::to_string_with_precision(space_demand, 4) << +" kWh\n";
    std::cout << "Yearly Total demand: " + tools::to_string_with_precision(demand_total, 4) << +" kWh\n";
    std::cout << "Max hourly demand: " + tools::to_string_with_precision(max_hourly_demand, 4) << +" kWh\n";
    //fmt::print("Space demand: {:.2f} kWh\n", space_demand);
    //fmt::print("Yearly total thermal demand: {:.2f} kWh\n", demand_total);
    //fmt::print("Max hourly demand: {:.2f} kWh\n", max_hourly_demand);
    return { demand_total , max_hourly_demand };
}

float HeatNinja::initColdestOutsideTemp() {
    const std::map<std::string, float> coldest_outside_temps = { { "50.0_-3.5", 4.61f }, { "50.0_-4.0", 4.554f }, { "50.0_-4.5", 4.406f }, { "50.0_-5.0", 4.017f }, { "50.0_-5.5", 4.492f }, { "50.5_-0.5", 3.02f }, { "50.5_-1.0", 3.188f }, { "50.5_-1.5", 2.812f }, { "50.5_-2.0", 2.583f }, { "50.5_-2.5", 2.774f }, { "50.5_-3.0", 2.697f }, { "50.5_-3.5", 1.744f }, { "50.5_-4.0", 0.854f }, { "50.5_-4.5", 1.27f }, { "50.5_-5.0", 2.708f }, { "50.5_0.0", 2.886f }, { "50.5_0.5", 2.764f }, { "51.0_-0.5", -3.846f }, { "51.0_-1.0", -4.285f }, { "51.0_-1.5", -4.421f }, { "51.0_-2.0", -4.274f }, { "51.0_-2.5", -3.764f }, { "51.0_-3.0", -2.635f }, { "51.0_-3.5", -1.712f }, { "51.0_-4.0", -0.232f }, { "51.0_-4.5", 1.638f }, { "51.0_0.0", -3.344f }, { "51.0_0.5", -2.101f }, { "51.0_1.0", 0.307f }, { "51.0_1.5", 1.271f }, { "51.5_-0.5", -5.969f }, { "51.5_-1.0", -5.673f }, { "51.5_-1.5", -5.09f }, { "51.5_-2.0", -4.292f }, { "51.5_-2.5", -3.039f }, { "51.5_-3.0", -1.591f }, { "51.5_-3.5", 0.221f }, { "51.5_-4.0", 1.249f }, { "51.5_-4.5", 2.001f }, { "51.5_-5.0", 2.948f }, { "51.5_0.0", -5.628f }, { "51.5_0.5", -4.165f }, { "51.5_1.0", -1.369f }, { "51.5_1.5", 1.813f }, { "52.0_-0.5", -5.601f }, { "52.0_-1.0", -5.283f }, { "52.0_-1.5", -4.854f }, { "52.0_-2.0", -4.37f }, { "52.0_-2.5", -3.7f }, { "52.0_-3.0", -3.597f }, { "52.0_-3.5", -3.13f }, { "52.0_-4.0", -2.297f }, { "52.0_-4.5", -0.642f }, { "52.0_-5.0", 2.044f }, { "52.0_-5.5", 3.622f }, { "52.0_0.0", -5.439f }, { "52.0_0.5", -4.533f }, { "52.0_1.0", -2.836f }, { "52.0_1.5", 0.146f }, { "52.5_-0.5", -4.979f }, { "52.5_-1.0", -4.814f }, { "52.5_-1.5", -4.451f }, { "52.5_-2.0", -3.991f }, { "52.5_-2.5", -3.603f }, { "52.5_-3.0", -3.359f }, { "52.5_-3.5", -3.007f }, { "52.5_-4.0", -0.479f }, { "52.5_-4.5", 2.769f }, { "52.5_0.0", -4.845f }, { "52.5_0.5", -4.0f }, { "52.5_1.0", -3.96f }, { "52.5_1.5", -1.778f }, { "52.5_2.0", 1.576f }, { "53.0_-0.5", -4.434f }, { "53.0_-1.0", -4.51f }, { "53.0_-1.5", -4.234f }, { "53.0_-2.0", -3.806f }, { "53.0_-2.5", -3.409f }, { "53.0_-3.0", -2.964f }, { "53.0_-3.5", -2.419f }, { "53.0_-4.0", -0.304f }, { "53.0_-4.5", 1.987f }, { "53.0_-5.0", 3.827f }, { "53.0_0.0", -4.07f }, { "53.0_0.5", -1.754f }, { "53.0_1.0", 0.277f }, { "53.0_1.5", 1.709f }, { "53.0_2.0", 2.397f }, { "53.5_-0.5", -4.156f }, { "53.5_-1.0", -4.141f }, { "53.5_-1.5", -3.834f }, { "53.5_-2.0", -3.492f }, { "53.5_-2.5", -2.729f }, { "53.5_-3.0", -1.344f }, { "53.5_-3.5", 0.446f }, { "53.5_-4.0", 1.524f }, { "53.5_-4.5", 2.578f }, { "53.5_0.0", -2.173f }, { "53.5_0.5", 1.351f }, { "54.0_-0.5", -2.622f }, { "54.0_-1.0", -3.424f }, { "54.0_-1.5", -3.834f }, { "54.0_-2.0", -3.837f }, { "54.0_-2.5", -2.766f }, { "54.0_-3.0", -0.56f }, { "54.0_-3.5", 1.22f }, { "54.0_-5.5", 3.297f }, { "54.0_-6.0", 1.151f }, { "54.0_-6.5", -1.496f }, { "54.0_-7.0", -3.164f }, { "54.0_-7.5", -3.294f }, { "54.0_-8.0", -2.848f }, { "54.0_0.0", 0.231f }, { "54.5_-0.5", 0.579f }, { "54.5_-1.0", -1.903f }, { "54.5_-1.5", -4.414f }, { "54.5_-2.0", -5.579f }, { "54.5_-2.5", -5.161f }, { "54.5_-3.0", -2.187f }, { "54.5_-3.5", -0.424f }, { "54.5_-4.0", 1.047f }, { "54.5_-4.5", 2.244f }, { "54.5_-5.0", 2.994f }, { "54.5_-5.5", 1.337f }, { "54.5_-6.0", -0.575f }, { "54.5_-6.5", -2.338f }, { "54.5_-7.0", -3.041f }, { "54.5_-7.5", -2.662f }, { "54.5_-8.0", -1.808f }, { "55.0_-1.5", -0.996f }, { "55.0_-2.0", -4.155f }, { "55.0_-2.5", -6.204f }, { "55.0_-3.0", -4.514f }, { "55.0_-3.5", -2.703f }, { "55.0_-4.0", -1.58f }, { "55.0_-4.5", -0.407f }, { "55.0_-5.0", 0.806f }, { "55.0_-5.5", 2.081f }, { "55.0_-6.0", 0.887f }, { "55.0_-6.5", -0.469f }, { "55.0_-7.0", -0.993f }, { "55.0_-7.5", -0.77f }, { "55.5_-1.5", 0.873f }, { "55.5_-2.0", -2.474f }, { "55.5_-2.5", -5.702f }, { "55.5_-3.0", -5.566f }, { "55.5_-3.5", -4.895f }, { "55.5_-4.0", -4.132f }, { "55.5_-4.5", -2.358f }, { "55.5_-5.0", -0.579f }, { "55.5_-5.5", 1.338f }, { "55.5_-6.0", 2.057f }, { "55.5_-6.5", 2.505f }, { "56.0_-2.0", 1.815f }, { "56.0_-2.5", 0.195f }, { "56.0_-3.0", -2.189f }, { "56.0_-3.5", -4.626f }, { "56.0_-4.0", -5.49f }, { "56.0_-4.5", -4.919f }, { "56.0_-5.0", -3.499f }, { "56.0_-5.5", -1.181f }, { "56.0_-6.0", 1.063f }, { "56.0_-6.5", 2.977f }, { "56.5_-2.5", -0.305f }, { "56.5_-3.0", -3.11f }, { "56.5_-3.5", -5.41f }, { "56.5_-4.0", -6.757f }, { "56.5_-4.5", -7.005f }, { "56.5_-5.0", -5.879f }, { "56.5_-5.5", -3.253f }, { "56.5_-6.0", 0.046f }, { "56.5_-6.5", 2.699f }, { "56.5_-7.0", 4.242f }, { "57.0_-2.0", 1.061f }, { "57.0_-2.5", -4.347f }, { "57.0_-3.0", -6.774f }, { "57.0_-3.5", -8.256f }, { "57.0_-4.0", -8.531f }, { "57.0_-4.5", -8.952f }, { "57.0_-5.0", -7.613f }, { "57.0_-5.5", -4.211f }, { "57.0_-6.0", -0.368f }, { "57.0_-6.5", 2.421f }, { "57.0_-7.0", 3.249f }, { "57.0_-7.5", 4.066f }, { "57.5_-2.0", 0.562f }, { "57.5_-2.5", -2.636f }, { "57.5_-3.0", -3.24f }, { "57.5_-3.5", -3.825f }, { "57.5_-4.0", -4.351f }, { "57.5_-4.5", -5.412f }, { "57.5_-5.0", -7.049f }, { "57.5_-5.5", -3.771f }, { "57.5_-6.0", 0.002f }, { "57.5_-6.5", 2.105f }, { "57.5_-7.0", 2.649f }, { "57.5_-7.5", 3.287f }, { "58.0_-3.5", 1.614f }, { "58.0_-4.0", -0.872f }, { "58.0_-4.5", -2.392f }, { "58.0_-5.0", -2.029f }, { "58.0_-5.5", 0.609f }, { "58.0_-6.0", 2.139f }, { "58.0_-6.5", 2.056f }, { "58.0_-7.0", 1.757f }, { "58.5_-3.0", 1.924f }, { "58.5_-3.5", 1.382f }, { "58.5_-4.0", 0.97f }, { "58.5_-4.5", 0.903f }, { "58.5_-5.0", 1.605f }, { "58.5_-5.5", 2.935f }, { "58.5_-6.0", 2.901f }, { "58.5_-6.5", 2.723f }, { "58.5_-7.0", 2.661f }, { "59.0_-2.5", 2.975f }, { "59.0_-3.0", 2.525f }, { "59.0_-3.5", 3.066f }, { "59.5_-1.5", 3.281f }, { "59.5_-2.5", 3.684f }, { "59.5_-3.0", 3.79f }, { "60.0_-1.0", 2.361f }, { "60.0_-1.5", 2.383f }, { "60.5_-1.0", 1.794f }, { "60.5_-1.5", 1.783f }, { "61.0_-1.0", 1.721f }
    };

    float lat_rounded = std::roundf(latitude * 2) / 2;
    float lon_rounded = std::roundf(longitude * 2) / 2;

    if (lat_rounded == 0.0f) { // incase = -0.0 causes map finding issue
        lat_rounded = 0.0f;
    }
    
    if (lon_rounded == 0.0f) {
        lon_rounded = 0.0f;
    }    

    const std::string key = tools::to_string_with_precision(lat_rounded, 1) + "_" + tools::to_string_with_precision(lon_rounded, 1);
    return coldest_outside_temps.at(key);
}

std::vector<float> HeatNinja::importDataFile(std::string filename) {
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

std::vector<float> HeatNinja::importWeatherData(const std::string& data_type) {
    // using vector instead of array because: function exceeds stack size, consider moving some data to heap (C6262)
    // data_type = "outside_temps or solar_irradiances"
    float lat_rounded = std::roundf(latitude * 2) / 2;
    float lon_rounded = std::roundf(longitude * 2) / 2;
    if (lat_rounded == 0.0f) { // incase = -0.0 causes map finding issue
        lat_rounded = 0.0f;
    }

    if (lon_rounded == 0.0f) {
        lon_rounded = 0.0f;
    }
    const std::string filename = "assets/" + data_type + "/lat_" + tools::to_string_with_precision(lat_rounded, 1) + "_lon_" + tools::to_string_with_precision(lon_rounded, 1) + ".csv";
    //const std::string filename = fmt::format("{}\\lat_{:.1f}_lon_{:.1f}.csv", data_type, lat_rounded, lon_rounded);
    //fmt::print("{}, {}\n", std::filesystem::current_path().string(), filename);
    //std::cout << "filename: " << filename << '\n';
    return importDataFile(filename);
}

std::vector<float> HeatNinja::importAgileTariff(const std::string& data_type) {
    // using vector instead of array because: function exceeds stack size, consider moving some data to heap (C6262)
    const std::string filename = "assets/agile_tariff.csv";
    return importDataFile(filename);
}

HeatNinja::TesTariffSpecs::TesTariffSpecs(float total_operational_cost, float cap_ex, HeatOptions hp_option, SolarOptions solar_option, int pv_size, int solar_thermal_size, float tes_volume, float net_present_cost, float operation_emissions, Tariff tariff) :
    total_operational_cost(total_operational_cost), cap_ex(cap_ex), hp_option(hp_option), solar_option(solar_option), pv_size(pv_size), solar_thermal_size(solar_thermal_size), tes_volume(tes_volume), net_present_cost(net_present_cost), operation_emissions(operation_emissions), tariff(tariff) {
}

HeatNinja::HPSolarSpecs::HPSolarSpecs(int index) : hp_option(static_cast<HeatOptions>(index / 7)), solar_option(static_cast<SolarOptions>(index % 7)) {
    //std::cout << index % 7 << ' ' << index / 7 << '\n';
}

void HeatNinja::HpOptionLoop(int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, std::ofstream& output_file) {
    for (int hp_option = 0; hp_option < 3; ++hp_option) {
        //fmt::print("hp_option {}\n", hp_option);
        //std::cout << "hp_option" << hp_option << "\n";

        SolarOptionLoop(static_cast<HeatOptions>(hp_option), solar_maximum, tes_range, ground_temp, optimum_tes_and_tariff_spec, output_file);
    }
}

const std::array<float, 24>& HeatNinja::select_temp_profile(HeatOptions hp_option, const std::array<float, 24>& hp_temp_profile, const std::array<float, 24>& erh_temp_profile) {
    switch (hp_option)
    {
    case HeatOptions::ASHP:
    case HeatOptions::GSHP:
        return hp_temp_profile;
    default:
        return erh_temp_profile;
    }
}

float HeatNinja::calculate_cop_worst(HeatOptions hp_option, float hot_water_temp, float coldest_outside_temp) {
    switch (hp_option) // hp sources: A review of domestic heat pumps
    {
    case HeatOptions::ERH:
        return 1;
    case HeatOptions::ASHP:
        return tools::ax2bxc(0.000630f, -0.121f, 6.81f, hot_water_temp - coldest_outside_temp);
    default: //HeatOptions::GSHP
        return tools::ax2bxc(0.000734f, -0.150f, 8.77f, hot_water_temp - coldest_outside_temp);
    }
}

float HeatNinja::calculate_hp_electrical_power(HeatOptions hp_option, float max_hourly_erh_demand, float max_hourly_hp_demand, float cop_worst) {
    // Mitsubishi have 4kWth ASHP, Kensa have 3kWth GSHP
    // 7kWth Typical maximum size for domestic power
    switch (hp_option) 
    { 
    case HeatOptions::ERH:
        return std::min(std::max(max_hourly_erh_demand, 4.0f / cop_worst), 7.0f);
    default: // ASHP or GSHP
        return std::min(std::max(max_hourly_hp_demand, 4.0f / cop_worst), 7.0f);
    }
}

int HeatNinja::calculate_solar_size_range(SolarOptions solar_option, int solar_maximum) {
    switch (solar_option)   
    {
    case HeatNinja::SolarOptions::None:
        return 1;
    case HeatNinja::SolarOptions::FP_PV:
    case HeatNinja::SolarOptions::ET_PV:
        return solar_maximum / 2 - 1;
    default:
        return solar_maximum / 2;
    }
}

void HeatNinja::SolarOptionLoop(HeatOptions hp_option, int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, std::ofstream& output_file) {
    const std::array<float, 24>& temp_profile = select_temp_profile(hp_option, this->hp_temp_profile, this->temp_profile);
    const float cop_worst = calculate_cop_worst(hp_option, hot_water_temp, coldest_outside_temp);
    float hp_electrical_power = calculate_hp_electrical_power(hp_option, boiler_demand.max_hourly, hp_demand.max_hourly, cop_worst);

    for (int solar_option_int = 0; solar_option_int < 7; ++solar_option_int) {
        SolarOptions solar_option = static_cast<SolarOptions>(solar_option_int);
        float optimum_tes_npc = 1000000;
        int solar_size_range = calculate_solar_size_range(solar_option, solar_maximum);
        TesTariffSpecs current_tes_and_tariff_specs;
        SolarSizeLoop(hp_option, solar_option, solar_size_range, optimum_tes_npc, solar_maximum, tes_range, cop_worst, hp_electrical_power, ground_temp, current_tes_and_tariff_specs, &temp_profile, output_file);
        const int index = solar_option_int + static_cast<int>(hp_option) * 7;
        optimum_tes_and_tariff_spec.at(index) = current_tes_and_tariff_specs;
    }
}

int file_index = 1449;
std::array<HeatNinja::TesTariffSpecs, 21> HeatNinja::simulate_heat_solar_combinations(int solar_maximum, float tes_range, float ground_temp) {
    //std::array<std::thread, 21> threads;

    std::array<HeatNinja::TesTariffSpecs, 21> optimal_specifications;
#ifndef EM_COMPATIBLE
    std::array<int, 21> is = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
    std::for_each(std::execution::par_unseq, is.begin(), is.end(), [&](int i) {
        simulate_heat_solar_combination(static_cast<HeatOptions>(i / 7), static_cast<SolarOptions>(i % 7), solar_maximum, tes_range, ground_temp, optimal_specifications.at(i));
        });
#else
    for (int i = 0; i < 21; ++i) {
        simulate_heat_solar_combination(static_cast<HeatOptions>(i / 7), static_cast<SolarOptions>(i % 7), solar_maximum, tes_range, ground_temp, optimal_specifications.at(i));
    }
#endif
    return optimal_specifications;
}

std::vector<size_t> HeatNinja::linearly_space(float range, size_t segments) {
    std::vector<size_t> points;
    points.reserve(segments + 1);
    const float step = range / segments;
    int j = 0;
    for (float i = 0; j < range; i += step) {
        j = static_cast<int>(i);
        if (static_cast<float>(i - j) > 0.5f) ++j;
        points.push_back(j);
        //std::cout << i << ", " << j << '\n';
    }
    return points;
}

struct IndexRect {
    size_t i1, j1, i2, j2;
};

float HeatNinja::min_4f(float a, float b, float c, float d) {
    float m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    if (d < m) m = d;
    return m;
}

float HeatNinja::get_or_calculate(size_t i, size_t j, size_t x_size, float& min_z, std::vector<float>& zs,
    HeatOptions hp_option, SolarOptions solar_option, float& optimum_tes_npc, int solar_maximum, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& optimal_spec, const std::array<float, 24>* temp_profile) {
    constexpr float unset_z = 3.40282e+038f;
    float& z = zs.at(i + j * x_size);
    if (z == unset_z) {
        z = calculate_optimal_tariff(hp_option, solar_option, static_cast<int>(j), optimum_tes_npc, solar_maximum, static_cast<int>(i), cop_worst, hp_electrical_power, ground_temp, optimal_spec, temp_profile);
        if (z < min_z) min_z = z;
    }
    return z;
}

void HeatNinja::if_unset_calculate(size_t i, size_t j, size_t x_size, float& min_z, std::vector<float>& zs,
    HeatOptions hp_option, SolarOptions solar_option, float& optimum_tes_npc, int solar_maximum, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& optimal_spec, const std::array<float, 24>* temp_profile) {
    constexpr float unset_z = 3.40282e+038f;
    // i = tes_option, j = solar_size
    if (zs.at(i + j * x_size) == unset_z) {
        // return what ever variable you want to optimise by (designed for npc)
        float z = calculate_optimal_tariff(hp_option, solar_option, static_cast<int>(j), optimum_tes_npc, solar_maximum, static_cast<int>(i), cop_worst, hp_electrical_power, ground_temp, optimal_spec, temp_profile);
        // may not need min_z
        if (z < min_z) min_z = z;
    }
}

void HeatNinja::simulate_heat_solar_combination(HeatOptions hp_option, SolarOptions solar_option, int solar_maximum, float tes_range, float ground_temp, TesTariffSpecs& optimal_spec) {
    const std::array<float, 24>& temp_profile = select_temp_profile(hp_option, this->hp_temp_profile, this->temp_profile);
    const float cop_worst = calculate_cop_worst(hp_option, hot_water_temp, coldest_outside_temp);
    float hp_electrical_power = calculate_hp_electrical_power(hp_option, boiler_demand.max_hourly, hp_demand.max_hourly, cop_worst);
    float optimum_tes_npc = 1000000;
    int solar_size_range = calculate_solar_size_range(solar_option, solar_maximum);

    // OPTIMISER ==========================================================================================
    const size_t min_step = 3;
    float gradient_factor = 0.15f;
    int target_step = 7;
    // user defined variables
    size_t x_size = static_cast<size_t>(tes_range), y_size = static_cast<size_t>(solar_size_range);

    if (y_size > 1 && true) {
        // non-user variables
        constexpr float unset_z = 3.40282e+038f; // if z has no been found yet it is set to max float value
        float min_z = unset_z; // record the current minimum z
        float max_mx = 0, max_my = 0; // gradient of steepest segment

        // create blank surface of z's
        std::vector<float> zs(x_size * y_size, unset_z);

        // calculate initial points to search on surface
        const size_t x_subdivisions = std::max(x_size / target_step, min_step);
        const size_t y_subdivisions = std::max(y_size / target_step, min_step);
        std::vector<size_t> is = linearly_space(static_cast<float>(x_size - 1), x_subdivisions);
        std::vector<size_t> js = linearly_space(static_cast<float>(y_size - 1), y_subdivisions);

        // combine 1D x and y indices into a 2D mesh 
        std::vector<IndexRect> index_rects;
        for (size_t j = 0; j < y_subdivisions; ++j) {
            for (size_t i = 0; i < x_subdivisions; ++i) {
                index_rects.emplace_back(IndexRect{ is.at(i), js.at(j), is.at(i + 1), js.at(j + 1) });
            }
        }

        // calculate z for each position and set the min_z and steepest gradient for x & y
        for (IndexRect& r : index_rects) {
            const float z11 = get_or_calculate(r.i1, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
            const float z21 = get_or_calculate(r.i2, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
            const float z22 = get_or_calculate(r.i2, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
            const float z12 = get_or_calculate(r.i1, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);

            const float mx = std::abs((z11 - z21) / (r.i2 - r.i1));
            const float my = std::abs((z11 - z12) / (r.j2 - r.j1));

            if (mx > max_mx) max_mx = mx;
            if (my > max_my) max_my = my;
        }

        // multiply steepest gradient by user defined factor (how much variation in z is there between points?)
        max_mx *= gradient_factor;
        max_my *= gradient_factor;

        while (!index_rects.empty()) {
            std::vector<IndexRect> next_index_rects;
            for (IndexRect& r : index_rects) {

                // calculate distance between indices
                const size_t di = r.i2 - r.i1;
                const size_t dj = r.j2 - r.j1;

                // assume length > 1 as it is checked when creating a new segment

                // get npc at nodes of segment
                const float z11 = get_or_calculate(r.i1, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
                const float z21 = get_or_calculate(r.i2, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
                const float z22 = get_or_calculate(r.i2, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
                const float z12 = get_or_calculate(r.i1, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);

                // get node with lowest npc
                float min_local_z = min_4f(z11, z21, z22, z12);
                // estimate minimum npc between nodes
                float min_z_estimate = min_local_z - (max_mx * di + max_my * dj);

                // if segment could have npc lower than the current min subdivide
                if (min_z_estimate < min_z) {
                    if (di == 1 && dj == 1) { // no more subdivision possible
                        // should not be possible to reach
                        std::cout << "UNREACHABLE!\n";
                    }
                    else if (di == 1) { // rect only divisible along j
                        const size_t j12 = r.j1 + dj / 2;

                        if_unset_calculate(r.i1, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
                        if_unset_calculate(r.i2, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);

                        // if rect can be subdivided then subdivide
                        if (j12 - r.j1 > 1) next_index_rects.emplace_back(IndexRect{ r.i1, r.j1,  r.i2, j12 });
                        if (r.j2 - j12 > 1) next_index_rects.emplace_back(IndexRect{ r.i1, j12,  r.i2, r.j2 });
                    }
                    else if (dj == 1) { // rect only divisible along i
                        const size_t i12 = r.i1 + di / 2;

                        if_unset_calculate(i12, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
                        if_unset_calculate(i12, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);

                        // if rect can be subdivided then subdivide
                        if (i12 - r.i1 > 1) next_index_rects.emplace_back(IndexRect{ r.i1, r.j1,  i12, r.j2 });
                        if (r.i2 - i12 > 1) next_index_rects.emplace_back(IndexRect{ i12, r.j1,  r.i2, r.j2 });
                    }
                    else {
                        // midpoint can be found for both axes
                        const size_t i12 = r.i1 + di / 2;
                        const size_t j12 = r.j1 + dj / 2;

                        if_unset_calculate(i12, r.j1, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
                        if_unset_calculate(i12, r.j2, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
                        if_unset_calculate(r.i1, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
                        if_unset_calculate(r.i2, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
                        if_unset_calculate(i12, j12, x_size, min_z, zs, hp_option, solar_option, optimum_tes_npc, solar_maximum, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);

                        const bool sub_i1 = i12 - r.i1 == 1, sub_i2 = r.i2 - i12 == 1;
                        const bool sub_j1 = j12 - r.j1 == 1, sub_j2 = r.j2 - j12 == 1;

                        // one of the dimensions must have a length > 1 if the rect is to be subdivided further
                        if (!(sub_i1 && sub_j1)) next_index_rects.emplace_back(IndexRect{ r.i1, r.j1,  i12, j12 });
                        if (!(sub_i2 && sub_j1)) next_index_rects.emplace_back(IndexRect{ i12, r.j1,  r.i2, j12 });
                        if (!(sub_i1 && sub_j2)) next_index_rects.emplace_back(IndexRect{ r.i1, j12,  i12, r.j2 });
                        if (!(sub_i2 && sub_j2)) next_index_rects.emplace_back(IndexRect{ i12, j12,  r.i2, r.j2 });
                    }
                }
            }
            index_rects = next_index_rects;
        }

        if (false) {
            // DEBUG INFORMATION
            int points_searched = 0;
            for (size_t j = 0; j < y_size; ++j) {
                for (size_t i = 0; i < x_size; ++i) {
                    const float z = zs.at(i + j * x_size);
                    if (z == unset_z) {
                        //std::cout << "-";
                    }
                    else {
                        //std::cout << "#";
                        points_searched++;
                    }
                }
                //std::cout << '\n';
            }

            const float efficiency = (static_cast<float>(points_searched) / static_cast<float>(zs.size())) * 100.0f;
            std::cout << "min z: " << min_z << ", points searched: " << points_searched << ", efficiency: " << efficiency << ", gf: " << gradient_factor << ", step: " << target_step << '\n';
        }
        return;
    }

    // brute force method ====================================================================================================

    for (int solar_size = 0; solar_size < solar_size_range; ++solar_size) {
        for (int tes_option = 0; tes_option < tes_range; ++tes_option) {
            calculate_optimal_tariff(hp_option, solar_option, solar_size, optimum_tes_npc, solar_maximum, tes_option, cop_worst, hp_electrical_power, ground_temp, optimal_spec, &temp_profile);
        }
    }
}

#ifndef EM_COMPATIBLE
void HeatNinja::HpOptionLoop_Thread(int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, std::ofstream& output_file) {
    std::array<TesTariffSpecs, 7> specs1, specs2, specs3;

    std::thread th1([this, solar_maximum, tes_range, ground_temp, &specs1, &output_file] { this->SolarOptionLoop_Thread(HeatOptions::ERH, solar_maximum, tes_range, ground_temp, specs1, output_file); });
    std::thread th2([this, solar_maximum, tes_range, ground_temp, &specs2, &output_file] { this->SolarOptionLoop_Thread(HeatOptions::ASHP, solar_maximum, tes_range, ground_temp, specs2, output_file); });
    std::thread th3([this, solar_maximum, tes_range, ground_temp, &specs3, &output_file] { this->SolarOptionLoop_Thread(HeatOptions::GSHP, solar_maximum, tes_range, ground_temp, specs3, output_file); });
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

void HeatNinja::SolarOptionLoop_Thread(HeatOptions hp_option, int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 7>& optimum_tes_and_tariff_spec, std::ofstream& output_file) {
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
        SolarSizeLoop(hp_option, solar_option, solar_size_range, optimum_tes_npc, solar_maximum, tes_range, cop_worst, hp_electrical_power, ground_temp, current_tes_and_tariff_specs, temp_profile, output_file);
        //optimum_tes_and_tariff_spec.at(static_cast<size_t>(solar_option) + static_cast<size_t>(hp_option * 7)) = current_tes_and_tariff_specs;
        optimum_tes_and_tariff_spec.at(solar_option_int) = current_tes_and_tariff_specs;
    }
}

void HeatNinja::HPSolarOptionLoop_ParUnseq(int solar_maximum, float tes_range, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, float ground_temp, std::ofstream& output_file) {
    std::array<HPSolarSpecs, 21> hp_solar_specs = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
    std::for_each(std::execution::seq, hp_solar_specs.begin(), hp_solar_specs.end(), [&](HPSolarSpecs& hp_solar_spec) {
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
        SolarSizeLoop(hp_option, solar_option, solar_size_range, optimum_tes_npc, solar_maximum, tes_range, cop_worst, hp_electrical_power, ground_temp, current_tes_and_tariff_specs, temp_profile, output_file);
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

void HeatNinja::SolarSizeLoop(HeatOptions hp_option, SolarOptions solar_option, int solar_size_range, float& optimum_tes_npc, int solar_maximum, float tes_range, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& current_tes_and_tariff_specs, const std::array<float, 24>* temp_profile, std::ofstream& output_file) {

    for (int solar_size = 0; solar_size < solar_size_range; ++solar_size) {
        //fmt::print("        solar_size {}\n", solar_size);

        TesOptionLoop(hp_option, solar_option, solar_size, solar_maximum, tes_range, cop_worst, hp_electrical_power, optimum_tes_npc, ground_temp, current_tes_and_tariff_specs, temp_profile, output_file);
    }
}

int HeatNinja::calculate_solar_thermal_size(SolarOptions solar_option, int solar_size) {
    switch (solar_option)
    {
    case SolarOptions::None:
    case SolarOptions::PV:
        return 0;
    default:
        return (solar_size * 2 + 2);
    }
}

int HeatNinja::calculate_pv_size(SolarOptions solar_option, int solar_size, int solar_maximum, int solar_thermal_size) {
    switch (solar_option)
    {
    case SolarOptions::PV:
    case SolarOptions::PVT:
        return solar_size * 2 + 2;
    case SolarOptions::FP_PV:
    case SolarOptions::ET_PV:
        return solar_maximum - solar_thermal_size;
    default:
        return 0;
    }
}

void HeatNinja::TesOptionLoop(HeatOptions hp_option, SolarOptions solar_option, int solar_size, int solar_maximum, float tes_range, float cop_worst, float hp_electrical_power, float& optimum_tes_npc, float ground_temp, TesTariffSpecs& current_tes_and_tariff_specs, const std::array<float, 24>* temp_profile, std::ofstream& output_file) {
    const int solar_thermal_size = calculate_solar_thermal_size(solar_option, solar_size);

    const int pv_size = calculate_pv_size(solar_option, solar_size, solar_maximum, solar_thermal_size);

    //std::cout << "solar_thermal_size: " << solar_thermal_size << ", pv_size: " << pv_size << '\n';

    for (int tes_option = 0; tes_option < tes_range; ++tes_option) {
        float tes_volume_current = 0.1f + tes_option * 0.1f; // m3
        float hp_electrical_power_worst = hp_electrical_power * cop_worst; // hp option
        const float capex = calculate_capex_heatopt(hp_option, hp_electrical_power_worst) + calculate_capex_pv(solar_option, pv_size) + calculate_capex_solar_thermal(solar_option, solar_thermal_size) + calculate_capex_tes_volume(tes_volume_current);

        //fmt::print("            tes_option {}\n", tes_option);
        float optimum_tariff = 1000000;
        float min_npc_of_tariffs = 1000000;
        for (int tariff = 0; tariff < 5; ++tariff) {
            //fmt::print("                tariff {}\n", tariff);
            TariffLoop(hp_option, solar_option, tes_volume_current, optimum_tariff, solar_thermal_size, pv_size, cop_worst, hp_electrical_power, optimum_tes_npc, current_tes_and_tariff_specs, ground_temp, static_cast<Tariff>(tariff), temp_profile, solar_size, capex, output_file, min_npc_of_tariffs);
        }
        output_file << min_npc_of_tariffs;
        if (tes_option + 1 < tes_range) output_file << ',';
    }
    output_file << '\n';
}

float HeatNinja::calculateCumulativeDiscountRate(float discount_rate, int npc_years) {
    float discount_rate_current = 1;
    float cumulative_discount_rate = 0;
    for (int year = 0; year < npc_years; ++year) {
        cumulative_discount_rate += 1 / discount_rate_current;
        discount_rate_current *= discount_rate;
    }
    return cumulative_discount_rate;
}

float HeatNinja::calculate_capex_heatopt(HeatOptions hp_option, float hp_electrical_power_worst) {
    switch (hp_option)
    {
    case HeatOptions::ERH: // Small additional cost to a TES, https://zenodo.org/record/4692649#.YQEbio5KjIV
        return 100; 
    case HeatOptions::ASHP: // ASHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
        return (200 + 4750 / std::powf(hp_electrical_power_worst, 1.25f)) * hp_electrical_power_worst + 1500;  // £s
    default: // GSHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
        return (200 + 4750 / std::powf(hp_electrical_power_worst, 1.25f)) * hp_electrical_power_worst + 800 * hp_electrical_power_worst;
    }
}

float HeatNinja::calculate_capex_pv(SolarOptions solar_option, int pv_size) {
    switch (solar_option)
    {
    case HeatNinja::SolarOptions::PV:
    case HeatNinja::SolarOptions::FP_PV:
    case HeatNinja::SolarOptions::ET_PV:
        // PV panels installed
        if (pv_size * 0.2f < 4.0f) { // Less than 4kWp
            return pv_size * 0.2f * 1100; // m2 * 0.2kWp / m2 * £1100 / kWp = £
        }
        else {  // Larger than 4kWp lower £ / kWp
            return pv_size * 0.2f * 900; // m2 * 0.2kWp / m2 * £900 / kWp = £
        }
    default:
        return 0;
    }
}

float HeatNinja::calculate_capex_solar_thermal(SolarOptions solar_option, int solar_thermal_size) {
    switch (solar_option)
    {
    case SolarOptions::FP:
    case SolarOptions::FP_PV: 
        // Flat plate solar thermal
        // Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
        // Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
        return solar_thermal_size * (225 + 270 / (9 * 1.6f)) + 490 + 800 + 800;
    case SolarOptions::PVT:
        // https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
        return (solar_thermal_size / 1.6f) * (480 + 270 / 9) + 640 + 490 + 800 + 1440;
    case SolarOptions::ET:
    case SolarOptions::ET_PV:
        // Evacuated tube solar thermal
        // Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
        // Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
        return solar_thermal_size * (280 + 270 / (9 * 1.6f)) + 490 + 800 + 800;
    default:
        return 0;
    }
}

float HeatNinja::calculate_capex_tes_volume(float tes_volume_current) {
    // Formula based on this data https ://assets.publishing.service.gov.uk/government/uploads/system/uploads/attachment_data/file/545249/DELTA_EE_DECC_TES_Final__1_.pdf
    return 2068.3f * std::powf(tes_volume_current, 0.553f);
}

float HeatNinja::calculate_optimal_tariff(HeatOptions hp_option, SolarOptions solar_option, int solar_size, float& optimum_tes_npc, int solar_maximum, int tes_option, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& optimal_spec, const std::array<float, 24>* temp_profile) {
    // find optimal for given solar_size and tes_vol
    const int solar_thermal_size = calculate_solar_thermal_size(solar_option, solar_size);
    const int pv_size = calculate_pv_size(solar_option, solar_size, solar_maximum, solar_thermal_size);
    float tes_volume_current = 0.1f + tes_option * 0.1f; // m3
    const float hp_electrical_power_worst = hp_electrical_power * cop_worst; // hp option
    const float capex = calculate_capex_heatopt(hp_option, hp_electrical_power_worst) + calculate_capex_pv(solar_option, pv_size) + calculate_capex_solar_thermal(solar_option, solar_thermal_size) + calculate_capex_tes_volume(tes_volume_current);

    const std::array<std::string, 3> heat_opt_names = { "ERH", "ASHP", "GSHP" };
    const std::array<std::string, 7> solar_opt_names = { "None", "PV", "FP", "ET", "FP+PV", "ET+PV", "PVT" };
    const std::array<std::string, 5> tariff_names = { "Flat Rate", "Economy 7", "Bulb Smart", "Octopus Go", "Octopus Agile" };

    float optimum_tariff = 1000000;
    float min_npc = 1000000;
    for (int tariff_int = 0; tariff_int < 5; ++tariff_int) {
        Tariff tariff = static_cast<Tariff>(tariff_int);
        size_t hour_year_counter = 0;
        //std::cout << "here\n";
        float inside_temp_current = temp;  // Initial temp
        float solar_thermal_generation_total = 0;
        float operational_costs_peak = 0;
        float operational_costs_off_peak = 0;
        float operation_emissions = 0;

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
                calcHeaterDay(temp_profile, inside_temp_current, ratio_sg_south, ratio_sg_north, cwt_current, dhw_mf_current, tes_state_of_charge, tes_charge_full, tes_charge_boost, tes_charge_max, tes_radius, ground_temp, hp_option, solar_option, pv_size, solar_thermal_size, hp_electrical_power, tariff, operational_costs_peak, operational_costs_off_peak, operation_emissions, solar_thermal_generation_total, ratio_roof_south, tes_charge_min, hour_year_counter);
            }
            ++month;
        }

        float total_operational_cost = operational_costs_peak + operational_costs_off_peak; // tariff

        //std::cout << hp_electrical_power_worst << ", " << pv_size << ", " << solar_thermal_size << ", " << tes_volume_current << ", " << operational_costs_peak << ", " << operational_costs_off_peak << '\n';

        const float npc = capex + total_operational_cost * cumulative_discount_rate;
        if (npc < min_npc) min_npc = npc;
        //output_file << static_cast<int>(hp_option) << ", " << static_cast<int>(solar_option) << ", " << solar_size << ", " << tes_volume_current << ", " << static_cast<int>(tariff) << ", " << tools::to_string_with_precision(npc, 4) << "\n";

        // JUST RETURN THE SPEC EVEN IF ITS NOT OPTIMAL AS SURFACE NEEDS NPC DATA???

        if (total_operational_cost < optimum_tariff) {
            optimum_tariff = total_operational_cost;

            const float net_present_cost_current = capex + total_operational_cost * cumulative_discount_rate; // £s

            if (net_present_cost_current < optimum_tes_npc) {
                // Lowest cost TES & tariff for heating tech. For OpEx vs CapEx plots, with optimised TES and tariff
                optimum_tes_npc = net_present_cost_current;
                optimal_spec = { total_operational_cost, capex, hp_option, solar_option, pv_size, solar_thermal_size, tes_volume_current, net_present_cost_current, operation_emissions, tariff };
            }
        }
    }
    return min_npc;
}

void HeatNinja::TariffLoop(HeatOptions hp_option, SolarOptions solar_option, float tes_volume_current, float& optimum_tariff, int solar_thermal_size, int pv_size, float cop_worst, const float hp_electrical_power, float& optimum_tes_npc, TesTariffSpecs& current_tes_and_tariff_specs, float ground_temp, Tariff tariff, const std::array<float, 24>* temp_profile, int solar_size, float capex, std::ofstream& output_file, float& min_npc_of_tariffs) {
    size_t hour_year_counter = 0;
    //std::cout << "here\n";
    float inside_temp_current = temp;  // Initial temp
    float solar_thermal_generation_total = 0;
    float operational_costs_peak = 0;
    float operational_costs_off_peak = 0;
    float operation_emissions = 0;

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
            calcHeaterDay(temp_profile, inside_temp_current, ratio_sg_south, ratio_sg_north, cwt_current, dhw_mf_current, tes_state_of_charge, tes_charge_full, tes_charge_boost, tes_charge_max, tes_radius, ground_temp, hp_option, solar_option, pv_size, solar_thermal_size, hp_electrical_power, tariff, operational_costs_peak, operational_costs_off_peak, operation_emissions, solar_thermal_generation_total, ratio_roof_south, tes_charge_min, hour_year_counter);
        }
        ++month;
    }

    float total_operational_cost = operational_costs_peak + operational_costs_off_peak; // tariff

    //std::cout << hp_electrical_power_worst << ", " << pv_size << ", " << solar_thermal_size << ", " << tes_volume_current << ", " << operational_costs_peak << ", " << operational_costs_off_peak << '\n';

    std::array<std::string, 3> heat_opt_names = { "ERH", "ASHP", "GSHP" };
    std::array<std::string, 7> solar_opt_names = { "None", "PV", "FP", "ET", "FP+PV", "ET+PV", "PVT" };
    std::array<std::string, 5> tariff_names = { "Flat Rate", "Economy 7", "Bulb Smart", "Octopus Go", "Octopus Agile" };

    const float npc = capex + total_operational_cost * cumulative_discount_rate;
    min_npc_of_tariffs = std::min(npc, min_npc_of_tariffs);

    //output_file << static_cast<int>(hp_option) << ", " << static_cast<int>(solar_option) << ", " << solar_size << ", " << tes_volume_current << ", " << static_cast<int>(tariff) << ", " << tools::to_string_with_precision(npc, 4) << "\n";

    if (total_operational_cost < optimum_tariff) {
        optimum_tariff = total_operational_cost;
        
        const float net_present_cost_current = capex + total_operational_cost * cumulative_discount_rate; // £s

        if (net_present_cost_current < optimum_tes_npc) {  
            // Lowest cost TES & tariff for heating tech. For OpEx vs CapEx plots, with optimised TES and tariff
            optimum_tes_npc = net_present_cost_current;
            current_tes_and_tariff_specs = { total_operational_cost, capex, hp_option, solar_option, pv_size, solar_thermal_size, tes_volume_current, net_present_cost_current, operation_emissions, tariff };
        }
    }
}

std::array<float, 12> HeatNinja::initRatiosRoofSouth() {
    const float pf = std::sin(PI / 180 * 35 / 2);  // Assume roof is 35° from horizontal
    const float a = tools::ax3bx2cxd(-0.66f, -0.106f, 2.93f, 0, pf);
    const float b = tools::ax3bx2cxd(3.63f, -0.374f, -7.4f, 0, pf);
    const float c = tools::ax3bx2cxd(-2.71f, -0.991f, 4.59f, 1, pf);

    std::array<float, 12> ratios_roof_south = {};
    size_t month = 0;
    for (const float& solar_declination_current : solar_declination) {
        const float solar_height_factor = std::cos(PI / 180 * (latitude - solar_declination_current));
        ratios_roof_south.at(month) = tools::ax2bxc(a, b, c, solar_height_factor);
        ++month;
    }

    return ratios_roof_south;
}

void HeatNinja::calculate_inside_temp_change(float& inside_temp_current, float outside_temp_current, float solar_irradiance_current, float ratio_sg_south, float ratio_sg_north, float ratio_roof_south) {
    const float incident_irradiance_sg_s = solar_irradiance_current * ratio_sg_south;
    const float incident_irradiance_sg_n = solar_irradiance_current * ratio_sg_north;
    const float solar_gain_south = incident_irradiance_sg_s * solar_gain_house_factor;
    const float solar_gain_north = incident_irradiance_sg_n * solar_gain_house_factor;

    //float solar_irradiance_current = Solar_Irradiance[Weather_Count]
    //const float heat_loss = (house_size * thermal_transmittance * (inside_temp_current - outside_temp_current)) / 1000;
    const float heat_loss = house_size_thermal_transmittance_product * (inside_temp_current - outside_temp_current);

    // heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
    inside_temp_current += (-heat_loss + solar_gain_south + solar_gain_north + body_heat_gain) / heat_capacity;
}

HeatNinja::TesTempAndHeight::TesTempAndHeight(float upper_temperature, float lower_temperature, float thermocline_height)
    : upper_temperature(upper_temperature), lower_temperature(lower_temperature), thermocline_height(clamp_height(thermocline_height)){

}

float HeatNinja::TesTempAndHeight::clamp_height(float height) {
    if (height < 0) {
        return 0;
    }
    else if (height > 1) {
        return 1;
    }
    else {
        return height;
    }
}

HeatNinja::TesTempAndHeight HeatNinja::calculate_tes_temp_and_thermocline_height(float tes_state_of_charge, float tes_charge_full, float tes_charge_max, float tes_charge_boost, float cwt_current) {
    if (tes_state_of_charge <= tes_charge_full) {  // Currently at nominal temperature ranges
        // tes_lower_temperature Bottom of the tank would still be at CWT,
        // tes_thermocline_height %, from top down, .25 is top 25 %
        return { 51, cwt_current, tes_state_of_charge / tes_charge_full };
    }
    else if (tes_state_of_charge <= tes_charge_boost) { // At boosted temperature ranges
        return { 60, 51, (tes_state_of_charge - tes_charge_full) / (tes_charge_boost - tes_charge_full) };
    }
    else { // At max tes temperature
        return {95, 60, (tes_state_of_charge - tes_charge_boost) / (tes_charge_max - tes_charge_boost) };
    }
}

HeatNinja::CopCurrentAndBoost HeatNinja::calculate_cop_current_and_boost(HeatNinja::HeatOptions hp_option, float outside_temp_current, float ground_temp) {
    // return current, boost
    switch (hp_option)
    {
    case HeatNinja::HeatOptions::ERH: //Electric Heater
        return { 1.0f, 1.0f };
    case HeatNinja::HeatOptions::ASHP: // ASHP, source A review of domestic heat pumps
        return { tools::ax2bxc(0.00063f, -0.121f, 6.81f, hot_water_temp - outside_temp_current), tools::ax2bxc(0.00063f, -0.121f, 6.81f, 60 - outside_temp_current) };
    default: // GSHP, source A review of domestic heat pumps
        return {tools::ax2bxc(0.000734f, -0.150f, 8.77f, hot_water_temp - ground_temp), tools::ax2bxc(0.000734f, -0.150f, 8.77f, 60 - ground_temp)};
    }
}

float HeatNinja::calculate_pv_efficiency(SolarOptions solar_option, TesTempAndHeight tes_th) {
    switch (solar_option)
    {
    case SolarOptions::PVT: // PVT
        return(14.7f * (1 - 0.0045f * ((tes_th.upper_temperature + tes_th.lower_temperature) / 2.0f - 25))) / 100;
        // https://www.sciencedirect.com/science/article/pii/S0306261919313443#b0175
    default:
        return 0.1928f;
        // Technology Library https ://zenodo.org/record/4692649#.YQEbio5KjIV
        // monocrystalline used for domestic
    }
}

float HeatNinja::calculate_solar_thermal_generation_current(SolarOptions solar_option, TesTempAndHeight tes_th, int solar_thermal_size, float incident_irradiance_roof_south, float outside_temp_current) {
    float solar_thermal_generation_current = 0; // if solar_option < 2
    if (solar_option >= SolarOptions::FP) {
        const float solar_thermal_collector_temperature = (tes_th.upper_temperature + tes_th.lower_temperature) / 2;
        // Collector to heat from tes lower temperature to tes upper temperature, so use the average temperature
        
        if (incident_irradiance_roof_south == 0) {
            return 0;
        }
        else {
            float a, b, c;
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
            return std::max(0.8f * solar_thermal_size * tools::ax2bxc(a, b, c * incident_irradiance_roof_south, solar_thermal_collector_temperature - outside_temp_current), 0.0f);
        }
    }
    else {
        return 0;
    }
}

float HeatNinja::calculate_hourly_space_demand(float& inside_temp_current, float desired_min_temp_current, float cop_current, float tes_state_of_charge, float dhw_hr_demand, float hp_electrical_power) {
    if (inside_temp_current > desired_min_temp_current) {
        return 0;
    }
    else {
        float space_hr_demand = (desired_min_temp_current - inside_temp_current) * heat_capacity;
        //std::cout << space_hr_demand << ' ' << desired_min_temp_current << ' ' << inside_temp_current << '\n';
        if ((space_hr_demand + dhw_hr_demand) < (tes_state_of_charge + hp_electrical_power * cop_current)) {
            inside_temp_current = desired_min_temp_current;
            return space_hr_demand;
        }
        else {
            if (tes_state_of_charge > 0) { // Priority to space demand over TES charging
                space_hr_demand = (tes_state_of_charge + hp_electrical_power * cop_current) - dhw_hr_demand;
            }
            else {
                space_hr_demand = (hp_electrical_power * cop_current) - dhw_hr_demand;
            }
            inside_temp_current += space_hr_demand / heat_capacity;
            return space_hr_demand;
        }
    }
}

float HeatNinja::calculate_electrical_demand_for_heating(float& tes_state_of_charge, float space_water_demand, float hp_electrical_power, float cop_current) {
    if (space_water_demand < tes_state_of_charge) { // TES can provide all demand
        tes_state_of_charge -= space_water_demand;
        return 0;
    }
    else if (space_water_demand < (tes_state_of_charge + hp_electrical_power * cop_current)) {
        if (tes_state_of_charge > 0) {
            const float electrical_demand_current = (space_water_demand - tes_state_of_charge) / cop_current;;
            tes_state_of_charge = 0;  // TES needs support so taken to empty if it had any charge
            return electrical_demand_current;
        }
        else {
            return space_water_demand / cop_current;
        }
    }
    else { // TES and HP can't meet hour demand
        if (tes_state_of_charge > 0) tes_state_of_charge = 0;
        return hp_electrical_power;
    }
}

void HeatNinja::calculate_electrical_demand_for_tes_charging(float& electrical_demand_current, float& tes_state_of_charge, float tes_charge_full, Tariff tariff, int hour, float hp_electrical_power, float cop_current, float agile_tariff_current) {
    // Charges TES at off peak electricity times
    if (tes_state_of_charge < tes_charge_full &&
        ((tariff == Tariff::FlatRate && 12 < hour && hour < 16) ||
            (tariff == Tariff::Economy7 && (hour == 23 || hour < 6)) ||
            (tariff == Tariff::BulbSmart && 12 < hour && hour < 16) ||
            (tariff == Tariff::OctopusGo && 0 <= hour && hour < 5) ||
            (tariff == Tariff::OctopusAgile && agile_tariff_current < 9.0f))) {
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
}

void HeatNinja::boost_tes_and_electrical_demand(float& tes_state_of_charge, float& electrical_demand_current, float pv_remaining_current, float tes_charge_boost, float hp_electrical_power, float cop_boost) {
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
}

void HeatNinja::recharge_tes_to_minimum(float& tes_state_of_charge, float& electrical_demand_current, float tes_charge_min, float hp_electrical_power, float cop_current) {
    if (tes_state_of_charge < tes_charge_min) { // Take back up to 10L capacity if possible no matter what time
        if ((tes_charge_min - tes_state_of_charge) < (hp_electrical_power - electrical_demand_current) * cop_current) {
            electrical_demand_current += (tes_charge_min - tes_state_of_charge) / cop_current;
            tes_state_of_charge = tes_charge_min;
        }
        else if (electrical_demand_current < hp_electrical_power) { // Can't take all the way back up to 10L charge
            tes_state_of_charge += (hp_electrical_power - electrical_demand_current) * cop_current;
        }
    }
}

void HeatNinja::add_electrical_import_cost_to_opex(float& operational_costs_off_peak, float& operational_costs_peak, float electrical_import, Tariff tariff, float agile_tariff_current, int hour) {
    switch (tariff)
    {
    case Tariff::FlatRate:
        // Flat rate tariff https://www.nimblefins.co.uk/average-cost-electricity-kwh-uk#:~:text=Unit%20Cost%20of%20Electricity%20per,more%20than%20the%20UK%20average
        // Average solar rate https://www.greenmatch.co.uk/solar-energy/solar-panels/solar-panel-grants
        operational_costs_peak += 0.163f * electrical_import;
        break;
    case Tariff::Economy7:
        // Economy 7 tariff, same source as flat rate above
        if (hour < 6 || hour == 23) { // Off Peak
            operational_costs_off_peak += 0.095f * electrical_import;
        }
        else { // Peak
            operational_costs_peak += 0.199f * electrical_import;
        }
        break;
    case Tariff::BulbSmart:
        // Bulb smart, for East Midlands values 2021
        // https://help.bulb.co.uk/hc/en-us/articles/360017795731-About-Bulb-s-smart-tariff
        if (15 < hour && hour < 19) { // Peak winter times throughout the year
            operational_costs_peak += 0.2529f * electrical_import;
        }
        else { // Off peak
            operational_costs_off_peak += 0.1279f * electrical_import;
        }
        break;
    case Tariff::OctopusGo:
        // Octopus Go EV, LE10 0YE 2012, https://octopus.energy/go/rates/
        // https://www.octopusreferral.link/octopus-energy-go-tariff/
        if (0 <= hour && hour < 5) { // Off Peak
            operational_costs_off_peak += 0.05f * electrical_import;
        }
        else { // Peak
            operational_costs_peak += 0.1533f * electrical_import;
        }
        break;
    default:
        // Octopus Agile file 2020
        // 2021 Octopus export rates https ://octopus.energy/outgoing/
        if (agile_tariff_current < 9.0f) { // Off peak, lower range of variable costs
            operational_costs_off_peak += (agile_tariff_current / 100) * electrical_import;
        }
        else { // Peak, upper range of variable costs
            operational_costs_peak += (agile_tariff_current / 100) * electrical_import;
        }
        break;
    }
}

void HeatNinja::subtract_pv_revenue_from_opex(float& operational_costs_off_peak, float& operational_costs_peak, float pv_equivalent_revenue, Tariff tariff, float agile_tariff_current, int hour) {
    switch (tariff)
    {
    case Tariff::FlatRate:
        // Flat rate tariff https://www.nimblefins.co.uk/average-cost-electricity-kwh-uk#:~:text=Unit%20Cost%20of%20Electricity%20per,more%20than%20the%20UK%20average
        // Average solar rate https://www.greenmatch.co.uk/solar-energy/solar-panels/solar-panel-grants
        operational_costs_peak -= pv_equivalent_revenue * (0.163f + 0.035f) / 2;
        break;
    case Tariff::Economy7:
        // Economy 7 tariff, same source as flat rate above
        if (hour < 6 || hour == 23) { // Off Peak
            operational_costs_off_peak -= pv_equivalent_revenue * (0.095f + 0.035f) / 2;
        }
        else { // Peak
            operational_costs_peak -= pv_equivalent_revenue * (0.199f + 0.035f) / 2;
        }
        break;
    case Tariff::BulbSmart:
        // Bulb smart, for East Midlands values 2021
        // https://help.bulb.co.uk/hc/en-us/articles/360017795731-About-Bulb-s-smart-tariff
        if (15 < hour && hour < 19) { // Peak winter times throughout the year
            operational_costs_peak -= pv_equivalent_revenue * (0.2529f + 0.035f) / 2;
        }
        else { // Off peak
            operational_costs_off_peak -= pv_equivalent_revenue * (0.1279f + 0.035f) / 2;
        }
        break;
    case Tariff::OctopusGo:
        // Octopus Go EV, LE10 0YE 2012, https://octopus.energy/go/rates/
        // https://www.octopusreferral.link/octopus-energy-go-tariff/
        if (0 <= hour && hour < 5) { // Off Peak
            operational_costs_off_peak -= pv_equivalent_revenue * (0.05f + 0.03f) / 2;
        }
        else { // Peak
            operational_costs_peak -= pv_equivalent_revenue * (0.1533f + 0.03f) / 2;
        }
        break;
    default:
        // Octopus Agile file 2020
        // 2021 Octopus export rates https ://octopus.energy/outgoing/
        if (agile_tariff_current < 9.0f) { // Off peak, lower range of variable costs
            operational_costs_off_peak -= pv_equivalent_revenue * ((agile_tariff_current / 100) + 0.055f) / 2;
        }
        else { // Peak, upper range of variable costs
            operational_costs_peak -= pv_equivalent_revenue * ((agile_tariff_current / 100) + 0.055f) / 2;
        }
        break;
    }
}

float HeatNinja::calculate_emissions_solar_thermal(float solar_thermal_generation_current) {
    // Operational emissions summation
    // 22.5 average ST
    // from https ://post.parliament.uk/research-briefings/post-pn-0523/
    return solar_thermal_generation_current * 22.5f;
}

float HeatNinja::calculate_emissions_pv_generation(float pv_generation_current, float pv_equivalent_revenue, float grid_emissions, int pv_size) {
    // https://www.parliament.uk/globalassets/documents/post/postpn_383-carbon-footprint-electricity-generation.pdf
    // 75 for PV, 75 - Grid_Emissions show emissions saved for the grid or for reducing other electrical bills
    if (pv_size > 0) {
        return (pv_generation_current - pv_equivalent_revenue) * 75 + pv_equivalent_revenue * (75 - grid_emissions);
    }
    else {
        return 0;
    }
}

float HeatNinja::calculate_emissions_grid_import(float electrical_import, float grid_emissions) {
    return electrical_import * grid_emissions;
}

void HeatNinja::calcHeaterDay(const std::array<float, 24>* temp_profile, float& inside_temp_current, float ratio_sg_south, float ratio_sg_north, float cwt_current, float dhw_mf_current, float& tes_state_of_charge, float tes_charge_full, float tes_charge_boost, float tes_charge_max, float tes_radius, float ground_temp, HeatOptions hp_option, SolarOptions solar_option, int pv_size, int solar_thermal_size, const float hp_electrical_power, Tariff tariff, float& operational_costs_peak, float& operational_costs_off_peak, float& operation_emissions, float& solar_thermal_generation_total, float ratio_roof_south, float tes_charge_min, size_t& hour_year_counter) {
    const float pi_d = PI * tes_radius * 2;
    const float pi_r2 = PI * tes_radius * tes_radius;
    const float pi_d2 = pi_d * tes_radius * 2;

    for (size_t hour = 0; hour < 24; ++hour) {        
        const float outside_temp_current = outside_temps.at(hour_year_counter);
        const float solar_irradiance_current = solar_irradiances.at(hour_year_counter);
        calculate_inside_temp_change(inside_temp_current, outside_temp_current, solar_irradiance_current, ratio_sg_south, ratio_sg_north, ratio_roof_south);
        TesTempAndHeight tes_th = calculate_tes_temp_and_thermocline_height(tes_state_of_charge, tes_charge_full, tes_charge_max, tes_charge_boost, cwt_current);

        const float tes_upper_losses = (tes_th.upper_temperature - inside_temp_current) * u_value * (pi_d2 * tes_th.thermocline_height + pi_r2); // losses in kWh
        const float tes_lower_losses = (tes_th.lower_temperature - inside_temp_current) * u_value * (pi_d2 * (1 - tes_th.thermocline_height) + pi_r2);
        const float total_losses = tes_upper_losses + tes_lower_losses;
        tes_state_of_charge -= total_losses;
        inside_temp_current += total_losses / heat_capacity;

        const float desired_min_temp_current = temp_profile->at(hour);
        const float agile_tariff_current = agile_tariff.at(hour_year_counter);
        const float dhw_hr_current = dhw_hourly_ratios.at(hour);
        const float dhw_hr_demand = (dhw_avg_daily_vol * 4.18f * (hot_water_temp - cwt_current) / 3600) * dhw_mf_current * dhw_hr_current;

        CopCurrentAndBoost cop = calculate_cop_current_and_boost(hp_option, outside_temp_current, ground_temp);

        const float pv_efficiency = calculate_pv_efficiency(solar_option, tes_th);

        const float incident_irradiance_roof_south = solar_irradiance_current * ratio_roof_south / 1000; // kW / m2
        float pv_generation_current = pv_size * pv_efficiency * incident_irradiance_roof_south * 0.8f;  // 80 % shading factor

        const float solar_thermal_generation_current = calculate_solar_thermal_generation_current(solar_option, tes_th, solar_thermal_size, incident_irradiance_roof_south, outside_temp_current);
        tes_state_of_charge += solar_thermal_generation_current;
        solar_thermal_generation_total += solar_thermal_generation_current;
        // Dumps any excess solar generated heat to prevent boiling TES
        tes_state_of_charge = std::min(tes_state_of_charge, tes_charge_max);

        const float space_hr_demand = calculate_hourly_space_demand(inside_temp_current, desired_min_temp_current, cop.current, tes_state_of_charge, dhw_hr_demand, hp_electrical_power);

        float electrical_demand_current = calculate_electrical_demand_for_heating(tes_state_of_charge, space_hr_demand + dhw_hr_demand, hp_electrical_power, cop.current);
        calculate_electrical_demand_for_tes_charging(electrical_demand_current, tes_state_of_charge, tes_charge_full, tariff, static_cast<int>(hour), hp_electrical_power, cop.current, agile_tariff_current);
        const float pv_remaining_current = pv_generation_current - electrical_demand_current;

        //Boost temperature if any spare PV generated electricity, as reduced cop, raises to nominal temp above first
        boost_tes_and_electrical_demand(tes_state_of_charge, electrical_demand_current, pv_remaining_current, tes_charge_boost, hp_electrical_power, cop.boost);

        recharge_tes_to_minimum(tes_state_of_charge, electrical_demand_current, tes_charge_min, hp_electrical_power, cop.current);

        float pv_equivalent_revenue;
        float electrical_import;
        if (pv_generation_current > electrical_demand_current) { // Generating more electricity than using
            pv_equivalent_revenue = pv_generation_current - electrical_demand_current;
            electrical_import = 0;
            subtract_pv_revenue_from_opex(operational_costs_off_peak, operational_costs_peak, pv_equivalent_revenue, tariff, agile_tariff_current, static_cast<int>(hour));
        }
        else {
            pv_equivalent_revenue = 0;
            electrical_import = electrical_demand_current - pv_generation_current;
            add_electrical_import_cost_to_opex(operational_costs_off_peak, operational_costs_peak, electrical_import, tariff, agile_tariff_current, static_cast<int>(hour));
        }

        operation_emissions += calculate_emissions_solar_thermal(solar_thermal_generation_current) +
            calculate_emissions_pv_generation(pv_generation_current, pv_equivalent_revenue, grid_emissions, pv_size) +
            calculate_emissions_grid_import(electrical_import, grid_emissions);
        hour_year_counter++;
    }    
}

std::string HeatNinja::initHeaterTesSettings() {
    // HEATER & TES SETTINGS
    //std::cout << "Electrified heating options at annual costs:" << '\n';
    

    float ground_temp = 15 - (latitude - 50) * (4.0f / 9.0f); // Linear regression ground temp across UK at 100m depth
    float tes_range = tes_volume_max / 0.1f; // SHOULD BE AN INT E.G. 3.0 / 0.1 = 30

    int solar_maximum = static_cast<int>(house_size / 8) * 2;  // Quarter of the roof for solar, even number

    house_size_thermal_transmittance_product = house_size * thermal_transmittance / 1000;

    //std::ofstream output_file;
    //output_file.open("../matlab/output.txt");
    //std::array<TesTariffSpecs, 21> optimum_tes_and_tariff_spec;
    //HpOptionLoop(solar_maximum, tes_range, ground_temp, optimum_tes_and_tariff_spec, output_file);
    std::array<HeatNinja::TesTariffSpecs, 21> optimum_tes_and_tariff_spec = simulate_heat_solar_combinations(solar_maximum, tes_range, ground_temp);

#ifdef EM_COMPATIBLE
    //SINGLE THREAD
    //std::array<HeatNinja::TesTariffSpecs, 21> optimum_tes_and_tariff_spec = simulate_heat_solar_combinations(solar_maximum, tes_range, ground_temp, output_file);
    //HpOptionLoop(solar_maximum, tes_range, ground_temp, optimum_tes_and_tariff_spec, output_file);
#else
    //HpOptionLoop(solar_maximum, tes_range, ground_temp, optimum_tes_and_tariff_spec, output_file);
    
    //MULTI THREADED
    //HpOptionLoop_Thread(solar_maximum, tes_range, ground_temp, optimum_tes_and_tariff_spec);
    // EXECUTION:PAR_UNSEQ
    //HpOptionLoop(solar_maximum, tes_range, ground_temp, optimum_tes_and_tariff_spec);
    //HPSolarOptionLoop_ParUnseq(solar_maximum, tes_range, optimum_tes_and_tariff_spec, ground_temp, output_file);
#endif
    //output_file.close();
    std::cout << "\n--- Optimum TES and Net Present Cost per Heating & Solar Option ---";
    std::cout << "\nHP Opt, Solar Opt, PV Size, Solar Size, TES Vol, OPEX, CAPEX, NPC, Emissions, Tariff\n";

    const std::array<std::string, 3> heat_opt_names = { "ERH", "ASHP", "GSHP" };
    const std::array<std::string, 7> solar_opt_names = { "None", "PV", "FP", "ET", "FP+PV", "ET+PV", "PVT" };
    const std::array<std::string, 5> tariff_names = { "Flat Rate", "Economy 7", "Bulb Smart", "Octopus Go", "Octopus Agile" };

    for (const auto& s : optimum_tes_and_tariff_spec) {
        //fmt::print("[ {}, {}, {}, {}, {}, {}, {}, {}, {} ]\n", s.total_operational_cost, s.cap_ex, s.hp_option, s.solar_option, s.pv_size, s.solar_thermal_size, s.tes_volume, s.net_present_cost, s.operation_emissions);
        std::cout << heat_opt_names.at(static_cast<int>(s.hp_option)) << ", " << solar_opt_names.at(static_cast<int>(s.solar_option)) << ", " << s.pv_size << ", " << s.solar_thermal_size << ", " << s.tes_volume << ", " << tools::to_string_with_precision(s.total_operational_cost, 1) << ", " << tools::to_string_with_precision(s.cap_ex, 1) << ", " << tools::to_string_with_precision(s.net_present_cost, 1) << ", " << tools::to_string_with_precision(s.operation_emissions, 1) << ", " << tariff_names.at(static_cast<int>(s.tariff)) << "\n";
        //fmt::print("total_operational_cost {},\ncap_ex {},\nhp_option {},\nsolar_option {},\npv_size {},\nsolar_thermal_size {},\ntes_volume {},\nnet_present_cost {},\noperation_emissions {} \n\n\n", s.total_operational_cost, s.cap_ex, s.hp_option, s.solar_option, s.pv_size, s.solar_thermal_size, s.tes_volume, s.net_present_cost, s.operation_emissions);
    }

    std::stringstream ss;
    ss << '[';
    int i = 0;
    for (const auto& s : optimum_tes_and_tariff_spec) {
        ss << "[\"" << heat_opt_names.at(static_cast<int>(s.hp_option)) << "\", \"" << solar_opt_names.at(static_cast<int>(s.solar_option)) << "\", " << s.pv_size << ", " << s.solar_thermal_size << ", " << s.tes_volume << ", " << tools::to_string_with_precision(s.total_operational_cost, 0) << ", " << tools::to_string_with_precision(s.cap_ex, 0) << ", " << tools::to_string_with_precision(s.net_present_cost, 0) << ", " << tools::to_string_with_precision(s.operation_emissions / 1000, 0) << "]";
        if (i < 20) {
            ss << ',';
        }
        ++i;
    }
    ss << ']';
    //std::cout << ss.str() << '\n';
    return ss.str();
}