//#define EM_COMPATIBLE

#include "heatninja.h"
#include <iostream>
#include <fstream>

#ifndef EM_COMPATIBLE
#include <chrono>
#endif

// FUNCTION DECLARATIONS

void runSimulationWithDefaultParameters();

extern "C" {
    const char* runSimulation(const char* postcode_char, float latitude, float longitude,
        int num_occupants, float house_size, float temp, int epc_space_heating, float tes_volume_max);
}

// FUNCTION DEFINITIONS

void runSimulationWithDefaultParameters()
{
    int num_occupants = 2;
    std::string postcode = "CV4 7AL";
    int epc_space_heating = 3000;
    float house_size = 60.0;
    float tes_volume_max = 3.0;
    float temp = 20.0;
    const float latitude = 52.3833f;
    const float longitude = -1.5833f;

#ifndef EM_COMPATIBLE
    auto start_time = std::chrono::steady_clock::now();
#endif

    std::cout << "--- Simulation Begun ---\n";
    const char* output = runSimulation(postcode.c_str(), latitude, longitude, num_occupants, house_size, temp, epc_space_heating, tes_volume_max);
    std::cout << "--- Simulation Output ---\n" << output << "\nSimulation Complete\n";

#ifndef EM_COMPATIBLE
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Runtime: " << elapsed_time / 1000.0 << " s\n";
#endif
    
}

void readInputFile(std::string filename) {
    std::ifstream infile(filename);
    std::string line;
    int i = 0;
    while (std::getline(infile, line))
    {
        std::stringstream ss(line);

        std::string postcode;
        std::getline(ss, postcode, ',');

        if (postcode == "postcode") continue;
        ++i;
        if (i < 70) {
            continue;
        }

        std::string temporary;

        std::getline(ss, temporary, ',');
        float latitude = std::stof(temporary);

        std::getline(ss, temporary, ',');
        float longitude = std::stof(temporary);

        std::getline(ss, temporary, ',');
        int num_occupants = std::stoi(temporary);

        std::getline(ss, temporary, ',');
        float house_size = std::stof(temporary);

        std::getline(ss, temporary, ',');
        float temp = std::stof(temporary);

        std::getline(ss, temporary, ',');
        int epc_space_heating = std::stoi(temporary);

        std::getline(ss, temporary, ',');
        float tes_volume_max = std::stof(temporary);
    
        std::cout << postcode << ", " << latitude << ", " << longitude << ", " << num_occupants << ", " << house_size << ", " << temp << ", " << epc_space_heating << ", " << tes_volume_max << ", " << '\n';
        runSimulation(postcode.c_str(), latitude, longitude, num_occupants, house_size, temp, epc_space_heating, tes_volume_max);

        //if (i > 5) {
        //    break;
        //}
        
    }
    infile.close();
}

// FUNCTIONS ACCESSIBLE FROM JAVASCRIPT
extern "C" {
    const char* runSimulation(const char* postcode_char, float latitude, float longitude,
        int num_occupants, float house_size, float temp, int epc_space_heating, float tes_volume_max)
    {
        const std::string postcode(postcode_char);

        HeatNinja heat_ninja(num_occupants, postcode, epc_space_heating, house_size,
            tes_volume_max, temp, latitude, longitude);

        std::string result = heat_ninja.initHeaterTesSettings();
        char* result_char = new char[result.size() + 1];
        std::copy(result.begin(), result.end(), result_char);
        result_char[result.size()] = '\0';
        return result_char;
    }
}

int main()
{
    readInputFile("input_list.csv");
    //runSimulationWithDefaultParameters();
}