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

void print_file_c(const char* filename_char) {
    std::string filename(filename_char);
    std::cout << filename << "\n";
    filename = "assets/outside_temps/" + filename + ".csv";
    std::cout << filename << "\n";
    std::vector<float> data = importDataFile(filename);
    for (float i : data) {
        std::cout << i << ", ";
    }
    std::cout << "\n";
}

std::string return_vector(std::vector<float> vec) {
    std::stringstream ss;
    for (float i : vec) {
        ss << i << ",";
    }
    return ss.str();
}

extern "C" {
    inline const char* cstr(const std::string& message) {
        char* cstr = new char[message.length() + 1];
        std::strcpy(cstr, message.c_str());
        return cstr;
    }
    
    const char* get_message(int x) {
        std::vector<float> vec = { 0.025f, 0.018f, 0.011f, 0.010f, 0.008f, 0.013f, 0.017f, 0.044f, 0.088f, 0.075f, 0.060f, 0.056f, 0.050f, 0.043f, 0.036f, 0.029f, 0.030f, 0.036f, 0.053f, 0.074f, 0.071f, 0.059f, 0.050f, 0.041f };
        
        return cstr(return_vector(vec));
    };
}

extern "C" {

    int print_file(const char* filename) {
        print_file_c(filename);
        return 0;
    }

    double int_sqrt(int x)
    {
        std::cout << "called int_sqrt"
            << "\n";

        Animal dear(x);

        std::string filename = "assets/input.txt";

        std::ifstream file2(filename);
        std::string str;
        while (std::getline(file2, str))
        {
            std::cout << str << "\n";
        }

        //print_file("lat_50.0_lon_-3.5");
        //std::vector<float> data = importDataFile("assets/array_data.csv");
        //for (float i : data) {
        //    std::cout << i << ", ";
        //}
        //std::cout << "\n";

        return std::sqrt(x);
    }
}

int main()
{
    std::cout << "Water and Space Heating (WASH) Simulator\n";
    int_sqrt(79);
    std::cout << get_message(1) << "\n";
}
