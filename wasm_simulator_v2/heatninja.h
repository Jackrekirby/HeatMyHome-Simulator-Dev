#pragma once

/* Tasks
1. asg_s ... should not be precalculated
2. make things private (SCRAP no classes)
3. store less class variables, locally scope them
4. should const things be input parameters / file imports? (avoid large hardcoded variables)
5. error handling: such as if file of long and latitude doesnt exist
6. correct typing of int, float, unsigned int, size_t
7. rename variables to hourly, monthly, yearly
8. add month constexpr to make arrays clearer
9. structs which are only in the cpp should be declared there
10. pass values by const
11. updates loops from y + (int * x) to float
12. dont pass this unless this is needed (this being heat ninja object)
*/

#include <string>
#include <array>
#include <vector>

namespace tools {
    std::string printArray(const auto& arr);

    template <typename T>
    std::string to_string_with_precision(const T a_value, const int n = 3);

    float ax2bxc(float a, float b, float c, float x);

    float ax3bx2cxd(float a, float b, float c, float d, float x);
}

class HeatNinja {
public:
    // nested structures / classes
    struct Demand {
        float total;
        float max_hourly;

        Demand(float total = 0, float max_hourly = 0);
    };

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

    enum class Tariff : int {
        FlatRate = 0,
        Economy7 = 1,
        BulbSmart = 2,
        OctopusGo = 3,
        OctopusAgile = 4
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
        Tariff tariff;

        TesTariffSpecs(float total_operational_cost = 0, float cap_ex = 0, HeatOptions hp_option = static_cast<HeatOptions>(0), SolarOptions solar_option = static_cast<SolarOptions>(0), int pv_size = 0, int solar_thermal_size = 0, float tes_volume = 0, float net_present_cost = 0, float operation_emissions = 0, Tariff tariff = static_cast<Tariff>(0));
    };

    struct HPSolarSpecs {
        HeatOptions hp_option;
        SolarOptions solar_option;
        HeatNinja::TesTariffSpecs optimum_tes_and_tariff_spec;

        HPSolarSpecs(int index);
    };

    struct TesTempAndHeight {
        float upper_temperature;
        float lower_temperature;
        float thermocline_height;

        TesTempAndHeight(float upper_temperature, float lower_temperature, float thermocline_height);

        float clamp_height(float height);
    };

    struct CopCurrentAndBoost
    {
        float current;
        float boost;
    };

    // static compile time constants
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr std::array<float, 24> dhw_hourly_ratios = { 0.025f, 0.018f, 0.011f, 0.010f, 0.008f, 0.013f, 0.017f, 0.044f, 0.088f, 0.075f, 0.060f, 0.056f, 0.050f, 0.043f, 0.036f, 0.029f, 0.030f, 0.036f, 0.053f, 0.074f, 0.071f, 0.059f, 0.050f, 0.041f };

    static constexpr int hot_water_temp = 51;
    static constexpr int grid_emissions = 212;
    static constexpr std::array<float, 24> hw_hourly_ratios = { 0.025f, 0.018f, 0.011f, 0.010f, 0.008f, 0.013f, 0.017f, 0.044f, 0.088f, 0.075f, 0.060f, 0.056f,
            0.050f, 0.043f, 0.036f, 0.029f, 0.030f, 0.036f, 0.053f, 0.074f, 0.071f, 0.059f, 0.050f, 0.041f };
    static constexpr std::array<float, 12> dhw_monthly_factor = { 1.10f, 1.06f, 1.02f, 0.98f, 0.94f, 0.90f, 0.90f, 0.94f, 0.98f, 1.02f, 1.06f, 1.10f };

    static constexpr std::array<float, 12> solar_declination = { -20.7f, -12.8f, -1.8f, 9.8f, 18.8f, 23.1f, 21.2f, 13.7f, 2.9f, -8.7f, -18.4f, -23.0f };  // Monthly values

    static constexpr std::array<int, 24> epc_temp_profile_summer = { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
    static constexpr std::array<int, 24> epc_temp_profile_weekend = { 7, 7, 7, 7, 7, 7, 7, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 };
    static constexpr std::array<int, 24> epc_temp_profile_other = { 7, 7, 7, 7, 7, 7, 7, 20, 20, 20, 7, 7, 7, 7, 7, 7, 20, 20, 20, 20, 20, 20, 20, 20 };
    static constexpr float u_value = 1.30f / 1000; // 0.00130 kW / m2K linearised from https ://zenodo.org/record/4692649#.YQEbio5KjIV &

    // run time constants - inputs
    const int num_occupants;
    const std::string location;
    const int epc_space_heating;
    const float house_size;
    const float tes_volume_max;
    const float temp;
    const float latitude;
    const float longitude;

    //  run time constants - input dependent 
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
    const int npc_years = 20;
    const float cumulative_discount_rate;

    // non constants
    float thermal_transmittance = 0;
    float house_size_thermal_transmittance_product = 0;
    Demand hp_demand;
    Demand boiler_demand;

    // Constuctor

    HeatNinja(int num_occupants, const std::string& location, int epc_space_heating,
        float house_size, float tes_volume_max, float temp, float latitude, float longitude);

    // const variable initialisation functions

    std::array<float, 24> initTempProfile(float temp);

    std::array<float, 24> initHpTempProfile(float temp);

    std::array<float, 12> initColdWaterTemp(float latitude);

    float initDhwAvgDailyVol(int num_occupants);

    float initHeatCapacity(float house_size);

    float initBodyHeatGain(int num_occupants);

    float initEpcBodyGain(float house_size);

    float calculateCumulativeDiscountRate(float discount_rate, int npc_years);

    //template<typename T>
    //int inPostalRegion(const T& postcode_region, const std::array<std::string, 4>& locations);

    //int initPostcodeIdOLD(const std::string& location);

    int initPostcodeId(const std::string& postcode);

    std::array<float, 12> initEpcOutsideTemp();

    std::array<int, 12> initEpcSolarIrradiance();

    std::array<float, 12> initSolarHeightFactors(float latitude);

    std::array<float, 12> initRatiosSgSouth();

    std::array<float, 12> initRatiosSgNorth();

    std::array<float, 12> initIncidentIrradiancesSgSouth();

    std::array<float, 12> initIncidentIrradiancesSgNorth();

    float initSolarGainHouseFactor(float house_size);

    std::array<float, 12> initSolarGainsSouth();

    std::array<float, 12> initSolarGainsNorth();

    // calculation functions

    void calcEpcDay(const std::array<int, 24>& epc_temp_profile, float& inside_temp_current, float outside_temp_current, float thermal_transmittance_current, float solar_gain_south, float solar_gain_north, float& epc_demand);

    void calcEpcMonth(int month, int num_days, float thermal_transmittance_current, float& inside_temp_current, float& epc_demand);

    void calcEpcYear();

    void calcDemandDay(const std::array<float, 24>& temp_profile, float& inside_temp_current, float ratio_sg_south, float ratio_sg_north, float cwt_current, float dhw_mf_current, float& demand_total, float& dhw_total, float& max_hourly_demand, size_t& hour_year_counter);

    void calcDemandMonth(int month, int num_days, const std::array<float, 24>& temp_profile, float inside_temp_current, float& demand_total, float& dhw_total, float& max_hourly_demand, size_t& hour_year_counter);

    Demand calcDemandYear(const std::array<float, 24>& temp_profile);

    float initColdestOutsideTemp();

    std::vector<float> importDataFile(std::string filename);

    std::vector<float> importWeatherData(const std::string& data_type);

    std::vector<float> importAgileTariff(const std::string& data_type);

    const std::array<float, 24>& select_temp_profile(HeatOptions hp_option, const std::array<float, 24>& hp_temp_profile, const std::array<float, 24>& erh_temp_profile);

    float calculate_cop_worst(HeatOptions hp_option, float hot_water_temp, float coldest_outside_temp);

    float calculate_hp_electrical_power(HeatOptions hp_option, float max_hourly_erh_demand, float max_hourly_hp_demand, float cop_worst);

    void HpOptionLoop(int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, std::ofstream& output_file);

    int calculate_solar_size_range(SolarOptions solar_option, int solar_maximum);

    void SolarOptionLoop(HeatOptions hp_option, int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, std::ofstream& output_file);


    float min_4f(float a, float b, float c, float d);
    std::vector<size_t> linearly_space(float range, size_t segments);

    float get_or_calculate(int i, int j, int x_size, float& min_z, std::vector<float>& zs,
        HeatOptions hp_option, SolarOptions solar_option, float& optimum_tes_npc, int solar_maximum, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& optimal_spec, const std::array<float, 24>* temp_profile);

    void if_unset_calculate(int i, int j, int x_size, float& min_z, std::vector<float>& zs,
        HeatOptions hp_option, SolarOptions solar_option, float& optimum_tes_npc, int solar_maximum, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& optimal_spec, const std::array<float, 24>* temp_profile);


    std::array<TesTariffSpecs, 21> simulate_heat_solar_combinations(int solar_maximum, float tes_range, float ground_temp);

    void simulate_heat_solar_combination(HeatOptions hp_option, SolarOptions solar_option, int solar_maximum, float tes_range, float ground_temp, TesTariffSpecs& optimal_spec);

    int calculate_solar_thermal_size(SolarOptions solar_option, int solar_size);

    int calculate_pv_size(SolarOptions solar_option, int solar_size, int solar_maximum, int solar_thermal_size);

    float calculate_optimal_tariff(HeatOptions hp_option, SolarOptions solar_option, int solar_size, float& optimum_tes_npc, int solar_maximum, int tes_option, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& optimal_spec, const std::array<float, 24>* temp_profile);

#ifndef EM_COMPATIBLE
    void HpOptionLoop_Thread(int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, std::ofstream& output_file);

    void SolarOptionLoop_Thread(HeatOptions hp_option, int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 7>& optimum_tes_and_tariff_spec, std::ofstream& output_file);

    void HPSolarOptionLoop_ParUnseq(int solar_maximum, float tes_range, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, float ground_temp, std::ofstream& output_file);
#endif

    void SolarSizeLoop(HeatOptions hp_option, SolarOptions solar_option, int solar_size_range, float& optimum_tes_npc, int solar_maximum, float tes_range, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& current_tes_and_tariff_specs, const std::array<float, 24>* temp_profile, std::ofstream& output_file);

    void TesOptionLoop(HeatOptions hp_option, SolarOptions solar_option, int solar_size, int solar_maximum, float tes_range, float cop_worst, float hp_electrical_power, float& optimum_tes_npc, float ground_temp, TesTariffSpecs& current_tes_and_tariff_specs, const std::array<float, 24>* temp_profile, std::ofstream& output_file);

    float calculate_capex_heatopt(HeatOptions hp_option, float hp_electrical_power_worst);
    float calculate_capex_pv(SolarOptions solar_option, int pv_size);
    float calculate_capex_solar_thermal(SolarOptions solar_option, int solar_thermal_size);
    float calculate_capex_tes_volume(float tes_volume_current);

    void TariffLoop(HeatOptions hp_option, SolarOptions solar_option, float tes_volume_current, float& optimum_tariff, int solar_thermal_size, int pv_size, float cop_worst, const float hp_electrical_power, float& optimum_tes_npc, TesTariffSpecs& current_tes_and_tariff_specs, float ground_temp, Tariff tariff, const std::array<float, 24>* temp_profile, int solar_size, float capex, std::ofstream& output_file, float& min_npc_of_tariffs);

    std::array<float, 12> initRatiosRoofSouth();

    CopCurrentAndBoost calculate_cop_current_and_boost(HeatNinja::HeatOptions hp_option, float outside_temp_current, float ground_temp);

    void calculate_inside_temp_change(float& inside_temp_current, float outside_temp_current, float solar_irradiance_current, float ratio_sg_south, float ratio_sg_north, float ratio_roof_south);

    TesTempAndHeight calculate_tes_temp_and_thermocline_height(float tes_state_of_charge, float tes_charge_full, float tes_charge_max, float tes_charge_boost, float cwt_current);

    float calculate_pv_efficiency(SolarOptions solar_option, HeatNinja::TesTempAndHeight tes_th);

    float calculate_solar_thermal_generation_current(SolarOptions solar_option, TesTempAndHeight tes_th, int solar_thermal_size, float incident_irradiance_roof_south, float outside_temp_current);

    float calculate_hourly_space_demand(float& inside_temp_current, float desired_min_temp_current, float cop_current, float tes_state_of_charge, float dhw_hr_demand, float hp_electrical_power);

    float calculate_electrical_demand_for_heating(float& tes_state_of_charge, float space_water_demand, float hp_electrical_power, float cop_current);

    void calculate_electrical_demand_for_tes_charging(float& electrical_demand_current, float& tes_state_of_charge, float tes_charge_full, Tariff tariff, int hour, float hp_electrical_power, float cop_current, float agile_tariff_current);

    void boost_tes_and_electrical_demand(float& tes_state_of_charge, float& electrical_demand_current, float pv_remaining_current, float tes_charge_boost, float hp_electrical_power, float cop_boost);

    void recharge_tes_to_minimum(float& tes_state_of_charge, float& electrical_demand_current, float tes_charge_min, float hp_electrical_power, float cop_current);

    void add_electrical_import_cost_to_opex(float& operational_costs_off_peak, float& operational_costs_peak, float electrical_import, Tariff tariff, float agile_tariff_current, int hour);

    void subtract_pv_revenue_from_opex(float& operational_costs_off_peak, float& operational_costs_peak, float pv_equivalent_revenue, Tariff tariff, float agile_tariff_current, int hour);

    float calculate_emissions_solar_thermal(float solar_thermal_generation_current);
    float calculate_emissions_pv_generation(float pv_generation_current, float pv_equivalent_revenue, float grid_emissions, int pv_size);
    float calculate_emissions_grid_import(float electrical_import, float grid_emissions);

    void calcHeaterDay(const std::array<float, 24>* temp_profile, float& inside_temp_current, float ratio_sg_south, float ratio_sg_north, float cwt_current, float dhw_mf_current, float& tes_state_of_charge, float tes_charge_full, float tes_charge_boost, float tes_charge_max, float tes_radius, float ground_temp, HeatOptions hp_option, SolarOptions solar_option, int pv_size, int solar_thermal_size, float hp_electrical_power, Tariff tariff, float& operational_costs_peak, float& operational_costs_off_peak, float& operation_emissions, float& solar_thermal_generation_total, float ratio_roof_south, float tes_charge_min, size_t& hour_year_counter);

    std::string initHeaterTesSettings();
};
