#pragma once
#include <array>
#include <vector>
#include <string>

namespace heatninja2d {
    // key terms
    // erh = electric resistance heating
    // hp = heat pump
    // dhw = domestic hot water

    struct SimulationOptions {
        bool output_demand;
        bool output_optimal_specs;
        bool output_all_specs; // not compatible with multithreading or optimal surfaces
        size_t output_file_index;

        bool use_multithreading;
        bool use_optimisation_surfaces;
    };

    // tools
    std::string double_to_string(const double value, const int precision);

    double ax2bxc(double a, double b, double c, double x);

    double ax3bx2cxd(double a, double b, double c, double d, double x);

    // simulation

    std::string run_simulation(const double thermostat_temperature, const double latitude, const double longitude, const int num_occupants, const double house_size, const std::string& postcode, const int epc_space_heating, const double tes_volume_max, const SimulationOptions& simulation_options);

    double round_coordinate(const double coordinate);

    std::vector<double> import_weather_data(const std::string& data_type, const double latitude, const double longitude);

    std::vector<double> import_per_hour_of_year_data(const std::string& filename);

    std::array<double, 24> calculate_erh_hourly_temperature_profile(const double t);

    std::array<double, 24> calculate_hp_hourly_temperature_profile(const double t);

    std::array<double, 12> calculate_monthly_cold_water_temperatures(const double latitude);

    std::array<double, 12> calculate_monthly_solar_height_factors(const double latitude, const std::array<double, 12>& monthly_solar_declination);

    std::array<double, 12> calculate_monthly_incident_irradiance_solar_gains_south(const std::array<double, 12>& monthly_solar_gain_ratios_south, const std::array<int, 12>& monthly_epc_solar_irradiances);

    std::array<double, 12> calculate_monthly_incident_irradiance_solar_gains_north(const std::array<double, 12>& monthly_solar_gain_ratios_north, const std::array<int, 12>& monthly_epc_solar_irradiances);

    std::array<double, 12> calculate_monthly_solar_gain_ratios_south(const std::array<double, 12>& monthly_solar_height_factors);

    std::array<double, 12> calculate_monthly_solar_gain_ratios_north(const std::array<double, 12>& monthly_solar_height_factors);

    std::array<double, 12> calculate_monthly_solar_gains_south(const std::array<double, 12>& incident_irradiance_solar_gains_south, const double solar_gain_house_factor);

    std::array<double, 12> calculate_monthly_solar_gains_north(const std::array<double, 12>& incident_irradiance_solar_gains_north, const double solar_gain_house_factor);

    double calculate_average_daily_hot_water_volume(const int num_occupants);

    double calculate_solar_gain_house_factor(const double house_size);

    double calculate_epc_body_gain(const double house_size);

    double calculate_heat_capacity(const double house_size);

    double calculate_body_heat_gain(const int num_occupants);

    int calculate_region_identifier(const std::string& postcode);

    std::array<double, 12> calculate_monthly_epc_outside_temperatures(int region_identifier);

    std::array<int, 12> calculate_monthly_epc_solar_irradiances(int region_identifier);

    const std::array<double, 24>& select_hourly_epc_temperature_profile(const size_t month, const size_t day, const std::array<double, 24>& summer_profile, const std::array<double, 24>& weekend_profile, const std::array<double, 24>& default_profile);

    struct ThermalTransmittanceAndOptimisedEpcDemand {
        double thermal_transmittance, optimised_epc_demand;
    };

    ThermalTransmittanceAndOptimisedEpcDemand calculate_dwellings_thermal_transmittance(const double house_size, const double epc_body_gain, const std::array<double, 12>& monthly_epc_outside_temperatures, const std::array<int, 12>& monthly_epc_solar_irradiances, const std::array<double, 12>& monthly_solar_height_factors, const std::array<double, 12>& monthly_solar_declinations, const std::array<double, 12>& monthly_solar_gains_south, const std::array<double, 12>& monthly_solar_gains_north, const double heat_capacity, const int epc_space_heating);

    struct Demand {
        double total, max_hourly, space, hot_water;
    };

    Demand calculate_yearly_space_and_hot_water_demand(const std::array<double, 24>& hourly_temperatures_over_day, const double thermostat_temperature, const std::array<double, 12>& hot_water_monthly_factors, const std::array<double, 12>& monthly_cold_water_temperatures, const std::array<double, 12>& monthly_solar_gain_ratios_north, const std::array<double, 12>& monthly_solar_gain_ratios_south, const std::array<double, 24>& dhw_hourly_ratios, const std::vector<double>& hourly_outside_temperatures_over_year, const std::vector<double>& hourly_solar_irradiances_over_year, const double average_daily_hot_water_volume, const int hot_water_temperature, const double solar_gain_house_factor, const double house_size, const double dwelling_thermal_transmittance, const double heat_capacity, const double body_heat_gain);

    void calculate_hourly_space_and_hot_water_demand(const std::array<double, 24>& hourly_temperatures_over_day, double& inside_temp_current, const double ratio_solar_gain_south, const double ratio_solar_gain_north, const double cwt_current, const double dhw_mf_current, double& demand_total, double& dhw_total, double& max_hourly_demand, const size_t hour_year_counter, const size_t hour, const std::array<double, 24>& dhw_hourly_ratios, const std::vector<double>& hourly_outside_temperatures_over_year, const std::vector<double>& hourly_solar_irradiances_over_year, const double average_daily_hot_water_volume, const int hot_water_temperature, const double solar_gain_house_factor, const double house_size, const double dwelling_thermal_transmittance, const double heat_capacity, const double body_heat_gain);

    void write_demand_data(const std::string filename, const double dwelling_thermal_transmittance, const double optimised_epc_demand, const double yearly_erh_demand, const double maximum_hourly_erh_demand, const double yearly_erh_space_demand, const double yearly_erh_hot_water_demand, const double yearly_hp_demand, const double maximum_hourly_hp_demand, const double yearly_hp_space_demand, const double yearly_hp_hot_water_demand);

    // OPTIMAL SPECIFICATIONS

    enum class HeatOption : int {
        ERH = 0, // electric resistance heating
        ASHP = 1, // air source heat pump
        GSHP = 2 // ground source heat pump
    };

    enum class SolarOption : int {
        None = 0,
        PV = 1, // PV = Photovoltaics
        FP = 2, // FP = Flat Plate
        ET = 3, // ET = Evacuate Tube
        FP_PV = 4,
        ET_PV = 5,
        PVT = 6 // PVT = Photovoltaic thermal hybrid solar collector
    };

    enum class Tariff : int {
        FlatRate = 0,
        Economy7 = 1,
        BulbSmart = 2,
        OctopusGo = 3,
        OctopusAgile = 4
    };

    struct HeatSolarSystemSpecifications {
        HeatOption heat_option;
        SolarOption solar_option;

        int pv_size;
        int solar_thermal_size;
        double tes_volume;

        Tariff tariff;

        double operational_expenditure;
        double capital_expenditure;
        double net_present_cost;

        double operation_emissions;
    };

    double calculate_coldest_outside_temperature_of_year(const double latitude, const double longitude);

    double calculate_ground_temperature(const double latitude);

    int calculate_tes_range(const double tes_volume_max);

    int calculate_solar_maximum(const double house_size);

    double calculate_house_size_thermal_transmittance_product(const double house_size, const double dwelling_thermal_transmittance);

    const std::array<double, 24>& select_temp_profile(const HeatOption hp_option, const std::array<double, 24>& hp_temp_profile, const std::array<double, 24>& erh_temp_profile);

    double calculate_cop_worst(const HeatOption hp_option, const int hot_water_temp, const double coldest_outside_temp);

    double calculate_hp_electrical_power(const HeatOption hp_option, const double max_hourly_erh_demand, const double max_hourly_hp_demand, const double cop_worst);

    int calculate_solar_size_range(const SolarOption solar_option, const int solar_maximum);

    std::vector<size_t> linearly_space(double range, size_t segments);

    double min_4f(const double a, const double b, const double c, const double d);

    double get_or_calculate(const size_t i, const size_t j, const size_t x_size, double& min_z, std::vector<double>& zs,
        const HeatOption hp_option, const SolarOption solar_option, double& optimum_tes_npc, const int solar_maximum, const double cop_worst, const double hp_electrical_power, const double ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<double, 24>* temp_profile, const double thermostat_temperature, const int hot_water_temperature, const double cumulative_discount_rate, const std::array<double, 12>& monthly_solar_gain_ratios_north, const std::array<double, 12>& monthly_solar_gain_ratios_south, const std::array<double, 12>& monthly_cold_water_temperatures, const std::array<double, 12>& dhw_monthly_factors, const std::array<double, 12>& monthly_solar_declinations, const std::array<double, 12>& monthly_roof_ratios_south, const std::vector<double>& hourly_outside_temperatures_over_year, const std::vector<double>& hourly_solar_irradiances_over_year, const double u_value, const double heat_capacity, const std::vector<double>& agile_tariff_per_hour_over_year, const std::array<double, 24>& hot_water_hourly_ratios, const double average_daily_hot_water_volume, const int grid_emissions, const double solar_gain_house_factor, const double body_heat_gain, const double house_size_thermal_transmittance_product);

    void if_unset_calculate(const size_t i, const size_t j, const size_t x_size, double& min_z, std::vector<double>& zs,
        const HeatOption hp_option, const SolarOption solar_option, double& optimum_tes_npc, const int solar_maximum, const double cop_worst, const double hp_electrical_power, const double ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<double, 24>* temp_profile, const double thermostat_temperature, const int hot_water_temperature, const double cumulative_discount_rate, const std::array<double, 12>& monthly_solar_gain_ratios_north, const std::array<double, 12>& monthly_solar_gain_ratios_south, const std::array<double, 12>& monthly_cold_water_temperatures, const std::array<double, 12>& dhw_monthly_factors, const std::array<double, 12>& monthly_solar_declinations, const std::array<double, 12>& monthly_roof_ratios_south, const std::vector<double>& hourly_outside_temperatures_over_year, const std::vector<double>& hourly_solar_irradiances_over_year, const double u_value, const double heat_capacity, const std::vector<double>& agile_tariff_per_hour_over_year, const std::array<double, 24>& hot_water_hourly_ratios, const double average_daily_hot_water_volume, const int grid_emissions, const double solar_gain_house_factor, const double body_heat_gain, const double house_size_thermal_transmittance_product);

    void simulate_heat_solar_combination(const HeatOption hp_option, const SolarOption solar_option, const int solar_maximum, const int tes_range, const double ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<double, 24>& erh_hourly_temperatures_over_day, const std::array<double, 24>& hp_hourly_temperatures_over_day, const int hot_water_temperature, const double coldest_outside_temperature_of_year, const double maximum_hourly_erh_demand, const double maximum_hourly_hp_demand, const double thermostat_temperature, const double cumulative_discount_rate, const std::array<double, 12>& monthly_solar_gain_ratios_north, const std::array<double, 12>& monthly_solar_gain_ratios_south, const std::array<double, 12>& monthly_cold_water_temperatures, const std::array<double, 12>& dhw_monthly_factors, const std::array<double, 12>& monthly_solar_declinations, const std::array<double, 12>& monthly_roof_ratios_south, const std::vector<double>& hourly_outside_temperatures_over_year, const std::vector<double>& hourly_solar_irradiances_over_year, const double u_value, const double heat_capacity, const std::vector<double>& agile_tariff_per_hour_over_year, const std::array<double, 24>& hot_water_hourly_ratios, const double average_daily_hot_water_volume, const int grid_emissions, const double solar_gain_house_factor, const double body_heat_gain, const double house_size_thermal_transmittance_product);

    int calculate_solar_thermal_size(const SolarOption solar_option, const int solar_size);

    int calculate_pv_size(const SolarOption solar_option, const int solar_size, const int solar_maximum, const int solar_thermal_size);

    double calculate_capex_heatopt(const HeatOption hp_option, const double hp_electrical_power_worst);

    double calculate_capex_pv(const SolarOption solar_option, const int pv_size);

    double calculate_capex_solar_thermal(const SolarOption solar_option, const int solar_thermal_size);

    double calculate_capex_tes_volume(const double tes_volume_current);

    double calculate_cumulative_discount_rate(const double discount_rate, const int npc_years);

    std::array<double, 12> calculate_roof_ratios_south(const std::array<double, 12>& monthly_solar_declinations, const double latitude);

    double calculate_optimal_tariff(const HeatOption hp_option, const SolarOption solar_option, const int solar_size, double& optimum_tes_npc, const int solar_maximum, const int tes_option, const double cop_worst, const double hp_electrical_power, const double ground_temp, HeatSolarSystemSpecifications& optimal_spec, const std::array<double, 24>* temp_profile, const double thermostat_temperature, const int hot_water_temperature, const double cumulative_discount_rate, const std::array<double, 12>& monthly_solar_gain_ratios_north, const std::array<double, 12>& monthly_solar_gain_ratios_south, const std::array<double, 12>& monthly_cold_water_temperatures, const std::array<double, 12>& dhw_monthly_factors, const std::array<double, 12>& monthly_solar_declinations, const std::array<double, 12>& monthly_roof_ratios_south, const std::vector<double>& hourly_outside_temperatures_over_year, const std::vector<double>& hourly_solar_irradiances_over_year, const double u_value, const double heat_capacity, const std::vector<double>& agile_tariff_per_hour_over_year, const std::array<double, 24>& hot_water_hourly_ratios, const double average_daily_hot_water_volume, const int grid_emissions, const double solar_gain_house_factor, const double body_heat_gain, const double house_size_thermal_transmittance_product);

    void calculate_inside_temp_change(double& inside_temp_current, const double outside_temp_current, const double solar_irradiance_current, const double ratio_sg_south, const double ratio_sg_north, const double ratio_roof_south, const double solar_gain_house_factor, const double body_heat_gain, const double house_size_thermal_transmittance_product, const double heat_capacity);

    struct TesTempAndHeight {
        double upper_temperature, lower_temperature, thermocline_height;

        TesTempAndHeight(const double upper_temperature, const double lower_temperature, const double thermocline_height);

        double clamp_height(const double height);
    };

    TesTempAndHeight calculate_tes_temp_and_thermocline_height(const double tes_state_of_charge, const double tes_charge_full, const double tes_charge_max, const double tes_charge_boost, const double cwt_current);

    struct CopCurrentAndBoost
    {
        double current;
        double boost;
    };

    CopCurrentAndBoost calculate_cop_current_and_boost(const HeatOption hp_option, const double outside_temp_current, const double ground_temp, const int hot_water_temperature);

    double calculate_pv_efficiency(const SolarOption solar_option, const double tes_upper_temperature, const double tes_lower_temperature);

    double calculate_solar_thermal_generation_current(const SolarOption solar_option, const double tes_upper_temperature, const double tes_lower_temperature, const int solar_thermal_size, const double incident_irradiance_roof_south, const double outside_temp_current);

    double calculate_hourly_space_demand(double& inside_temp_current, const double desired_min_temp_current, const double cop_current, const double tes_state_of_charge, const double dhw_hr_demand, const double hp_electrical_power, const double heat_capacity);

    double calculate_electrical_demand_for_heating(double& tes_state_of_charge, const double space_water_demand, const double hp_electrical_power, const double cop_current);

    void calculate_electrical_demand_for_tes_charging(double& electrical_demand_current, double& tes_state_of_charge, const double tes_charge_full, const Tariff tariff, const int hour, const double hp_electrical_power, const double cop_current, const double agile_tariff_current);

    void boost_tes_and_electrical_demand(double& tes_state_of_charge, double& electrical_demand_current, const double pv_remaining_current, const double tes_charge_boost, const double hp_electrical_power, const double cop_boost);

    void recharge_tes_to_minimum(double& tes_state_of_charge, double& electrical_demand_current, const double tes_charge_min, const double hp_electrical_power, const double cop_current);

    void add_electrical_import_cost_to_opex(double& operational_costs_off_peak, double& operational_costs_peak, const double electrical_import, const Tariff tariff, const double agile_tariff_current, const int hour);

    void subtract_pv_revenue_from_opex(double& operational_costs_off_peak, double& operational_costs_peak, const double pv_equivalent_revenue, const Tariff tariff, const double agile_tariff_current, const int hour);

    double calculate_emissions_solar_thermal(const double solar_thermal_generation_current);

    double calculate_emissions_pv_generation(const double pv_generation_current, const double pv_equivalent_revenue, const int grid_emissions, const int pv_size);

    double calculate_emissions_grid_import(const double electrical_import, const int grid_emissions);

    void simulate_heating_system_for_day(const std::array<double, 24>* temp_profile, double& inside_temp_current, const double ratio_sg_south, const double ratio_sg_north, const double cwt_current, double dhw_mf_current, double& tes_state_of_charge, const double tes_charge_full, const double tes_charge_boost, const double tes_charge_max, const double tes_radius, const double ground_temp, const HeatOption hp_option, const SolarOption solar_option, const int pv_size, const int solar_thermal_size, const double hp_electrical_power, const Tariff tariff, double& operational_costs_peak, double& operational_costs_off_peak, double& operation_emissions, double& solar_thermal_generation_total, const double ratio_roof_south, const double tes_charge_min, size_t& hour_year_counter, const std::vector<double>& hourly_outside_temperatures_over_year, const std::vector<double>& hourly_solar_irradiances_over_year, const double u_value, const double heat_capacity, const std::vector<double>& agile_tariff_per_hour_over_year, const std::array<double, 24>& hot_water_hourly_ratios, const double average_daily_hot_water_volume, const int hot_water_temperature, const int grid_emissions, const double solar_gain_house_factor, const double body_heat_gain, const double house_size_thermal_transmittance_product);

    void print_optimal_specifications(const std::array<HeatSolarSystemSpecifications, 21>& optimal_specifications, const int double_print_precision);

    void write_optimal_specification(const HeatSolarSystemSpecifications& spec, std::ofstream& file);

    void write_optimal_specifications(const std::array<HeatSolarSystemSpecifications, 21>& optimal_specifications, const std::string& filename);

    std::string output_to_javascript(const std::array<HeatSolarSystemSpecifications, 21>& optimal_specifications);
}