#pragma once
#define EM_COMPATIBLE

/* Tasks
1. asg_s ... should not be precalculated
2. make things private
3. store less class variables, locally scope them
4. should const things be input parameters / file imports? (avoid large hardcoded variables)
5. error handling: such as if file of long and latitude doesnt exist
6. remove templates and auto
*/

#include <string>
#include <array>
#include <vector>

namespace tools {
    std::string printArray(const auto& arr) {
    }

    template <typename T>
    std::string to_string_with_precision(const T a_value, const int n = 3);
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

    // static compile time constants
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

    template<typename T>
    int inPostalRegion(const T& postcode_region, const std::array<std::string, 4>& locations);

    int initPostcodeId(const std::string& location);

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

    void calcDemandMonth(int month, int num_days, const std::array<float, 24>& temp_profile, float inside_temp_current, float& demand_total, float& dhw_total, float& max_hourly_demand, size_t& hour_year_counter);

    Demand calcDemandYear(const std::array<float, 24>& temp_profile);

    float initColdestOutsideTemp();

    std::vector<float> importDataFile(std::string filename);

    std::vector<float> importWeatherData(const std::string& data_type);

    std::vector<float> importAgileTariff(const std::string& data_type);

    void HpOptionLoop(int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, std::ofstream& output_file);

    void SolarOptionLoop(HeatOptions hp_option, int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, std::ofstream& output_file);

#ifndef EM_COMPATIBLE
    void HpOptionLoop_Thread(int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, std::ofstream& output_file);

    void SolarOptionLoop_Thread(HeatOptions hp_option, int solar_maximum, float tes_range, float ground_temp, std::array<TesTariffSpecs, 7>& optimum_tes_and_tariff_spec, std::ofstream& output_file);

    void HPSolarOptionLoop_ParUnseq(int solar_maximum, float tes_range, std::array<TesTariffSpecs, 21>& optimum_tes_and_tariff_spec, float ground_temp, std::ofstream& output_file);
#endif

    void SolarSizeLoop(HeatOptions hp_option, SolarOptions solar_option, int solar_size_range, float& optimum_tes_npc, int solar_maximum, float tes_range, float cop_worst, float hp_electrical_power, float ground_temp, TesTariffSpecs& current_tes_and_tariff_specs, const std::array<float, 24>* temp_profile, std::ofstream& output_file);

    void TesOptionLoop(HeatOptions hp_option, SolarOptions solar_option, int solar_size, int solar_maximum, float tes_range, float cop_worst, float hp_electrical_power, float& optimum_tes_npc, float ground_temp, TesTariffSpecs& current_tes_and_tariff_specs, const std::array<float, 24>* temp_profile, std::ofstream& output_file);

    void TariffLoop(HeatOptions hp_option, SolarOptions solar_option, int tes_option, float& optimum_tariff, int solar_thermal_size, int pv_size, float cop_worst, float hp_electrical_power, float& optimum_tes_npc, TesTariffSpecs& current_tes_and_tariff_specs, float ground_temp, Tariff tariff, const std::array<float, 24>* temp_profile, int solar_size, std::ofstream& output_file);

    float ax2bxc(float a, float b, float c, float x);

    float ax3bx2cxd(float a, float b, float c, float d, float x);

    std::array<float, 12> initRatiosRoofSouth();

    void calcHeaterDay(const std::array<float, 24>* temp_profile, float& inside_temp_current, float ratio_sg_south, float ratio_sg_north, float cwt_current, float dhw_mf_current, float& tes_state_of_charge, float tes_charge_full, float tes_charge_boost, float tes_charge_max, float tes_radius, float ground_temp, HeatOptions hp_option, SolarOptions solar_option, int pv_size, int solar_thermal_size, float hp_electrical_power, Tariff tariff, float& tes_volume_current, float& operational_costs_peak, float& operational_costs_off_peak, float& operation_emissions, float& solar_thermal_generation_total, float ratio_roof_south, float tes_charge_min, size_t& hour_year_counter);

    std::string initHeaterTesSettings();
};
