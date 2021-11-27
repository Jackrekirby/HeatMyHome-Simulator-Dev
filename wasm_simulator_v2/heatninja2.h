#pragma once
#include <array>
#include <vector>
#include <string>

namespace heatninja2 {

    // tools

    std::string float_to_string(const float value, const int precision);

    float ax2bxc(float a, float b, float c, float x);

    float ax3bx2cxd(float a, float b, float c, float d, float x);

    // simulation

    void run_simulation(const float thermostat_temperature, const float latitude, const float longitude, const int num_occupants, const float house_size, const std::string& postcode, const int epc_space_heating);

    float round_coordinate(const float coordinate);

    std::vector<float> import_weather_data(const std::string& data_type, const float latitude, const float longitude);

    std::vector<float> import_per_hour_of_year_data(const std::string& filename);

    std::array<float, 24> calculate_erh_hourly_temperature_profile(const float t);

    std::array<float, 24> calculate_hp_hourly_temperature_profile(const float t);

    std::array<float, 12> calculate_monthly_cold_water_temperatures(const float latitude);

    std::array<float, 12> calculate_monthly_solar_height_factors(const float latitude, const std::array<float, 12>& monthly_solar_declination);

    std::array<float, 12> calculate_monthly_incident_irradiance_solar_gains_south(const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<int, 12>& monthly_epc_solar_irradiances);

    std::array<float, 12> calculate_monthly_incident_irradiance_solar_gains_north(const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<int, 12>& monthly_epc_solar_irradiances);

    std::array<float, 12> calculate_monthly_solar_gain_ratios_south(const std::array<float, 12>& monthly_solar_height_factors);

    std::array<float, 12> calculate_monthly_solar_gain_ratios_north(const std::array<float, 12>& monthly_solar_height_factors);

    std::array<float, 12> calculate_monthly_solar_gains_south(const std::array<float, 12>& incident_irradiance_solar_gains_south, const float solar_gain_house_factor);

    std::array<float, 12> calculate_monthly_solar_gains_north(const std::array<float, 12>& incident_irradiance_solar_gains_north, const float solar_gain_house_factor);

    float calculate_average_daily_hot_water_volume(const int num_occupants);

    float calculate_solar_gain_house_factor(const float house_size);

    float calculate_epc_body_gain(const float house_size);

    float calculate_heat_capacity(const float house_size);

    float calculate_body_heat_gain(const int num_occupants);

    struct PostcodeRegion {
        std::string postcode;
        int mininum;
        int maximum;
        int region;
    };

    int calculate_region_identifier(const std::string& postcode);

    std::array<float, 12> calculate_monthly_epc_outside_temperatures(int region_identifier);

    std::array<int, 12> calculate_monthly_epc_solar_irradiances(int region_identifier);

    const std::array<float, 24>& select_hourly_epc_temperature_profile(const size_t month, const size_t day, const std::array<float, 24>& summer_profile, const std::array<float, 24>& weekend_profile, const std::array<float, 24>& default_profile);

    float calculate_dwellings_thermal_transmittance(const float house_size, const float epc_body_gain, const std::array<float, 12>& monthly_epc_outside_temperatures, const std::array<int, 12>& monthly_solar_irradiances, const std::array<float, 12>& monthly_solar_height_factors, const std::array<float, 12>& monthly_solar_declinations, const std::array<float, 12>& monthly_solar_gains_south, const std::array<float, 12>& monthly_solar_gains_north, const float heat_capacity, const int epc_space_heating);

    struct Demand {
        float year_total, hourly_maximum;
    };

    Demand calculate_yearly_space_and_hot_water_demand(const std::array<float, 24>& hourly_temperatures_over_day, const float thermostat_temperature, const std::array<float, 12>& hot_water_monthly_factors, const std::array<float, 12>& monthly_cold_water_temperatures, const std::array<float, 12>& monthly_solar_gain_ratios_north, const std::array<float, 12>& monthly_solar_gain_ratios_south, const std::array<float, 24>& dhw_hourly_ratios, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float average_daily_hot_water_volume, const int hot_water_temperature, const float solar_gain_house_factor, const float house_size, const float dwelling_thermal_transmittance, const float heat_capacity, const float body_heat_gain);

    void calculate_hourly_space_and_hot_water_demand(const std::array<float, 24>& hourly_temperatures_over_day, float& inside_temp_current, const float ratio_solar_gain_south, const float ratio_solar_gain_north, const float cwt_current, const float dhw_mf_current, float& demand_total, float& dhw_total, float& max_hourly_demand, const size_t hour_year_counter, const size_t hour, const std::array<float, 24>& dhw_hourly_ratios, const std::vector<float>& hourly_outside_temperatures_over_year, const std::vector<float>& hourly_solar_irradiances_over_year, const float average_daily_hot_water_volume, const int hot_water_temperature, const float solar_gain_house_factor, const float house_size, const float dwelling_thermal_transmittance, const float heat_capacity, const float body_heat_gain);
}