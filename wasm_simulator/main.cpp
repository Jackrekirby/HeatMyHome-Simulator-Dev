#include <iostream>
#include <sstream>
#include <cmath>
#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>
#include "animal.h"

#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_WARNINGS

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

inline const char* cstr(const std::string& message) {
    char* cstr = new char[message.length() + 1];
    std::strcpy(cstr, message.c_str());
    return cstr;
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
}

int main()
{
    std::cout << "Water and Space Heating (WASH) Simulator\n";
    std::cout << int_sqrt(100) << '\n';
    call_class(67);
    print_example_file(0);
    print_outside_temps("lat_50.0_lon_-3.5");
    std::cout << return_vector(0) << '\n';

}
