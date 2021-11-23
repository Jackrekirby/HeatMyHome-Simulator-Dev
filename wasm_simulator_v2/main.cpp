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

struct NpcSurface {
    std::vector<float> npcs;
    int tes_vols;
    int solar_sizes;

    NpcSurface(int tes_vols, int solar_sizes, const std::vector<float>& npcs)
        : tes_vols(tes_vols), solar_sizes(solar_sizes), npcs(npcs) {

    }

    float at(int tes_vol_index, int solar_size) {
        return npcs.at(tes_vol_index + solar_size * tes_vols);
    }
};

NpcSurface readSurfaceFile(std::string filename) {
    std::ifstream infile(filename);
    std::string line;

    int solar_sizes = 0, tes_vols = 0;
    std::vector<float> npcs;
    
    while (std::getline(infile, line))
    {
        //std::cout << "NEW LINE" << "\n";
        std::stringstream ss(line);
        std::string number;
        //float x;
        while (std::getline(ss, number, ',')) {
            //std::cout << number << ", ";
            npcs.push_back(std::stof(number));
        }
        if (solar_sizes == 0) tes_vols = static_cast<int>(npcs.size());
        //std::cout << '\n';
        ++solar_sizes;
    }
    infile.close();

   // std::cout << "num_solar_sizes: " << solar_sizes << ", num_tes_vols: " << tes_vols << '\n';

    return { tes_vols, solar_sizes, npcs };
}

void surface_minima_finder() {
    //NpcSurface npcs = readSurfaceFile("../matlab/c_surfaces/1.csv");
    //std::cout << npcs.at(29, 16) << '\n';

    //for (float solar_size = 0; solar_size < npcs.solar_sizes; 
    //    solar_size += (npcs.solar_sizes / 5.0f)) {
    //    std::cout << "solar_size: " << solar_size << '\n';
    //}

    std::vector<int> a = { 0, 57 };
    while (!a.empty()) {
        std::vector<int> b;
        for (int i = 0; i < a.size() - 1; i++) {
            const int a1 = a.at(i), a2 = a.at(i + 1);
            const int delta = a2 - a1;
            
            if (delta > 1) {
                if(b.empty() || b.back() != a1) b.push_back(a1);
                b.push_back(a1 + delta / 2);
                b.push_back(a2);
            }
        }

        for (int bb : b) {
            std::cout << bb << ',';
        }
        std::cout << '\n';
        a = b;
    }
}

//float calc_or_get_npc(size_t x, size_t y) {
//    return 0;
//}
//
//void test(size_t x, size_t y) {
//    float p11 = calc_or_get_npc(x, y);
//    float p11 = calc_or_get_npc(x, y);
//}

struct Segment {
    size_t i1;
    size_t i2;

    size_t calculate_length() {
        return i2 - i1;
    }
};

float find_min(float a, float b) {
    if (a < b) {
        return a;
    }
    else {
        return b;
    }
}

void surface_minima_finder2() {
    float gradient_factor = 0.5f;

    NpcSurface hidden_npcs = readSurfaceFile("../matlab/c_surfaces/0.csv");

    // calculate true minima
    float true_min_npc = 1000000;
    for (size_t i = 0; i < hidden_npcs.tes_vols; i += 1) {
        const float p = hidden_npcs.at(i, 0);
        if (p < true_min_npc) true_min_npc = p;
    }

    float min_npc = 1000000;
    float max_gradient = 0; // gradient of steepest segment
    float no_npc = 1000000;
    std::vector<float> npcs(hidden_npcs.tes_vols, no_npc);

    std::vector<size_t> tes_indices = { 0, 6, 12, 18, 23, 29 };
    for (size_t i = 0; i < tes_indices.size() - 1; i += 1) {
        const float i1 = tes_indices.at(i);
        const float i2 = tes_indices.at(i + 1);

        const float p1 = hidden_npcs.at(i1, 0);
        const float p2 = hidden_npcs.at(i2, 0);
        if (p1 < min_npc) min_npc = p1;
        const float gradient = (p2 > p1) ? (p2 - p1) / (i2 - i1) : (p1 - p2) / (i2 - i1);
        if (gradient > max_gradient) max_gradient = gradient;

        npcs.at(i1) = p1;
        npcs.at(i2) = p2;
    }

    const float p = hidden_npcs.at(tes_indices.back(), 0);
    if (p < min_npc) min_npc = p;

    std::cout << "true_min_npc: " << true_min_npc << '\n';
    std::cout << "min_npc: " << min_npc << ", max_gradient: " << max_gradient << '\n';

    std::vector<Segment> segments = { {0, 29} };
    float gradient = max_gradient * gradient_factor;
    
    // Keep subdividing until maximum subdivision reached
    // Segment not subdivided if predicted to not contain minima or cannot be divided further
    while (!segments.empty()) {
        // create new list of segments for next stage of subdivision
        std::vector<Segment> new_segments;
        for (Segment& segment : segments) {
            // calculate distance between indices
            const size_t length = segment.calculate_length();
            // assume length > 1 as it is checked when creating a new segment

            // get npc at nodes of segment
            float p1 = npcs.at(segment.i1);
            float p2 = npcs.at(segment.i2);
            // get node with lowest npc
            float min_node = find_min(p1, p2);
            // estimate minimum npc between nodes
            float min_segment = min_node - gradient * length;

            // if segment could have npc lower than the current min subdivide
            if (min_segment < min_npc) {
                // calculate midpoint index of segment
                const size_t midpoint = segment.i1 + length / 2;
                // if midpoint npc does not exist calculate it
                if (npcs.at(midpoint) == no_npc) npcs.at(midpoint) = hidden_npcs.at(midpoint, 0);

                if (npcs.at(midpoint) < min_npc) min_npc = npcs.at(midpoint);

                // create new segments if the segments can be subdivided (length > 1)
                if (midpoint - segment.i1 > 1) new_segments.push_back({ segment.i1, midpoint });
                if (segment.i2 - midpoint > 1) new_segments.push_back({ midpoint, segment.i2 });
            }
        }

        for (Segment& segment : new_segments) {
            std::cout << '[' << segment.i1 << ',' << segment.i2 << "], ";
        }
        std::cout << '\n';
        segments = new_segments;
    }

    std::cout << "final min_npc: " << min_npc << '\n';
    if (min_npc == true_min_npc) {
        std::cout << "FOUND \n";
    }
    else {
        std::cout << "ERROR \n";
    }

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

std::vector<int> linearly_space(float range, int segments) {
    std::vector<int> points;
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
    //std::cout << range << '\n';
}

struct Vec2t {
    size_t tes_index;
    size_t solar_index;
};

struct SubSurface {
    Vec2t p11, p22;
};

void surface_minima_finder3(int file_index, float gradient_factor, int& passed, float& efficiency, int& surfaces) {
    std::stringstream ss;
    ss << "../matlab/c_surfaces/" << file_index << ".csv";
    NpcSurface hidden_npcs = readSurfaceFile(ss.str());

    if (hidden_npcs.solar_sizes == 1) return;

    // calculate true minima
    float true_min_npc = 1000000;
    for (size_t j = 0; j < hidden_npcs.solar_sizes; ++j) {
        for (size_t i = 0; i < hidden_npcs.tes_vols; ++i) {
            const float p = hidden_npcs.at(i, j);
            if (p < true_min_npc) true_min_npc = p;
        }
    }

    float min_npc = 1000000;
    float max_gradient_tes = 0, max_gradient_solar = 0; // gradient of steepest segment
    float no_npc = 1000000;
    std::vector<float> npcs(hidden_npcs.tes_vols * hidden_npcs.solar_sizes, no_npc);


    std::vector<int> tes_indices = linearly_space(hidden_npcs.tes_vols-1, 5);
    std::vector<int> solar_indices = linearly_space(hidden_npcs.solar_sizes-1, 5);

    //for (auto& i : tes_indices) {
    //    std::cout << i << ", ";
    //}
    //std::cout << '\n';

    //for (auto& i : solar_indices) {
    //    std::cout << i << ", ";
    //}
    //std::cout << '\n';

    std::vector<SubSurface> subsurfaces;
    // save space by just storing top and bottom corner
    for (size_t j = 0; j < solar_indices.size() - 1; ++j) {
        for (size_t i = 0; i < tes_indices.size() - 1; ++i) {
            SubSurface subsurface = { 
                {tes_indices.at(i), solar_indices.at(j)}, 
                {tes_indices.at(i+1), solar_indices.at(j+1)}, 
            };
            subsurfaces.emplace_back(subsurface);
        }
    }

    for (auto& subsurface : subsurfaces) {
        npcs.at(subsurface.p11.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols) = hidden_npcs.at(subsurface.p11.tes_index, subsurface.p11.solar_index);
        npcs.at(subsurface.p22.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols) = hidden_npcs.at(subsurface.p22.tes_index, subsurface.p11.solar_index);
        npcs.at(subsurface.p22.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols) = hidden_npcs.at(subsurface.p22.tes_index, subsurface.p22.solar_index);
        npcs.at(subsurface.p11.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols) = hidden_npcs.at(subsurface.p11.tes_index, subsurface.p22.solar_index);

        float p11 = npcs.at(subsurface.p11.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols);
        float p21 = npcs.at(subsurface.p22.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols);
        float p22 = npcs.at(subsurface.p22.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols);
        float p12 = npcs.at(subsurface.p11.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols);

        float min_node = find_min(find_min(p11, p21), find_min(p22, p12));
        if (min_node < min_npc) min_npc = min_node;

        float gradient_tes = std::abs((p21 - p11) / (subsurface.p22.tes_index - subsurface.p11.tes_index));
        float gradient_solar = std::abs((p12 - p11) / (subsurface.p22.solar_index - subsurface.p11.solar_index));

        if (gradient_tes > max_gradient_tes) max_gradient_tes = gradient_tes;
        if (gradient_solar > max_gradient_solar) max_gradient_solar = gradient_solar;
    }

    //std::cout << "true_min_npc: " << true_min_npc << '\n';
    //std::cout << "min_npc: " << min_npc << ", max_gradient_tes: " << max_gradient_tes << ", max_gradient_solar: " << max_gradient_solar << '\n';

    float gradient_tes = max_gradient_tes * gradient_factor;
    float gradient_solar = max_gradient_solar * gradient_factor;

    while (!subsurfaces.empty()) {
        for (auto& subsurface : subsurfaces) {
            //std::cout << "[" << subsurface.p11.tes_index << ", " << subsurface.p11.solar_index << "], "
            //    << "[" << subsurface.p22.tes_index << ", " << subsurface.p22.solar_index << "]\n";
        }

        for (size_t j = 0; j < hidden_npcs.solar_sizes; ++j) {
            for (size_t i = 0; i < hidden_npcs.tes_vols; ++i) {
                float npc = npcs.at(i + j * hidden_npcs.tes_vols);
                if (npc == no_npc) {
                    std::cout << "-";
                }
                else {
                    std::cout << "#";
                }

            }
            std::cout << '\n';
        }

        std::cout << "\n\n";
        std::vector<SubSurface> new_subsurfaces;
        for (auto& subsurface : subsurfaces) {

            // calculate distance between indices
            const size_t tes_length = subsurface.p22.tes_index - subsurface.p11.tes_index;
            const size_t solar_length = subsurface.p22.solar_index - subsurface.p11.solar_index;

            // assume length > 1 as it is checked when creating a new segment

            // get npc at nodes of segment
            float p11 = npcs.at(subsurface.p11.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols);
            float p21 = npcs.at(subsurface.p22.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols);
            float p22 = npcs.at(subsurface.p22.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols);
            float p12 = npcs.at(subsurface.p11.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols);

            // get node with lowest npc
            float min_node = find_min(find_min(p11, p21), find_min(p22, p12));
            // estimate minimum npc between nodes
            float min_segment = min_node - (gradient_tes * tes_length + gradient_solar * solar_length);

            // if segment could have npc lower than the current min subdivide
            if (min_segment < min_npc) {
                // calculate midpoint index of segment
                const size_t midpoint_tes = subsurface.p11.tes_index + tes_length / 2;
                const size_t midpoint_solar = subsurface.p11.solar_index + solar_length / 2;


                //std::cout << midpoint_tes << ", " << midpoint_solar << "\n";
                    

                std::array<Vec2t, 5> new_points = { {
                    {subsurface.p11.tes_index, midpoint_solar},
                    {subsurface.p22.tes_index, midpoint_solar},
                    {midpoint_tes, subsurface.p11.solar_index},
                    {midpoint_tes, subsurface.p22.solar_index},
                    {midpoint_tes, midpoint_solar}
                } };

                for (auto& point : new_points) {
                    size_t index = point.tes_index + point.solar_index * hidden_npcs.tes_vols;
                    // if midpoint npc does not exist calculate it
                    if (npcs.at(index) == no_npc) {
                        const float npc = hidden_npcs.at(point.tes_index, point.solar_index);
                        npcs.at(index) = npc;
                        if (npc < min_npc) min_npc = npc;
                    }
                }

                // create new segments if the segments can be subdivided (length > 1)
                // p11
                if (midpoint_tes - subsurface.p11.tes_index > 1 && midpoint_solar - subsurface.p11.solar_index > 1) {
                    SubSurface new_subsurface = { { subsurface.p11.tes_index, subsurface.p11.solar_index }, {midpoint_tes, midpoint_solar} };
                    new_subsurfaces.emplace_back(new_subsurface);
                }

                // p21
                if (subsurface.p22.tes_index - midpoint_tes > 1 && midpoint_solar - subsurface.p11.solar_index > 1) {
                    SubSurface new_subsurface = { { midpoint_tes, subsurface.p11.solar_index }, {subsurface.p22.tes_index, midpoint_solar} };
                    new_subsurfaces.emplace_back(new_subsurface);
                }

                // p22
                if (subsurface.p22.tes_index - midpoint_tes > 1 && subsurface.p22.solar_index - midpoint_solar > 1) {
                    SubSurface new_subsurface = { { midpoint_tes, midpoint_solar }, {subsurface.p22.tes_index, subsurface.p22.solar_index} };
                    new_subsurfaces.emplace_back(new_subsurface);
                }

                // p12
                if (midpoint_tes - subsurface.p11.tes_index > 1 && subsurface.p22.solar_index - midpoint_solar > 1) {
                    SubSurface new_subsurface = { { subsurface.p11.tes_index, midpoint_solar }, {midpoint_tes, subsurface.p22.solar_index} };
                    new_subsurfaces.emplace_back(new_subsurface);
                }
            }
        }
        subsurfaces = new_subsurfaces;
    }

    int points_searched = 0;
    for (size_t j = 0; j < hidden_npcs.solar_sizes; ++j) {
        for (size_t i = 0; i < hidden_npcs.tes_vols; ++i) {
            float npc = npcs.at(i + j * hidden_npcs.tes_vols);
            if (npc == no_npc) {
                std::cout << "-";
            }
            else {
                std::cout << "#";
                points_searched++;
            }
            
        }
        std::cout << '\n';
    }

    surfaces++;
    if (min_npc == true_min_npc) {
        passed++;
        std::cout << "PASS ";
    }
    else {
        std::cout << "FAIL XXXXXX ";
    }
    float saving = (static_cast<float>(points_searched) / static_cast<float>(npcs.size())) * 100.0f;
    efficiency += saving;
    std::cout << "i: " << file_index << ", final NPC: " << min_npc << ", true NPC: " << true_min_npc << ", Points Searched: " << points_searched << ", Saving: " << saving << '\n';
}

void surface_minima_finder4(int file_index, float gradient_factor, int subdivisions, int& passed, float& efficiency, int& surfaces) {
    std::stringstream ss;
    ss << "../matlab/c_surfaces/" << file_index << ".csv";
    NpcSurface hidden_npcs = readSurfaceFile(ss.str());

    if (hidden_npcs.solar_sizes == 1) return;

    // calculate true minima
    float true_min_npc = 1000000;
    size_t true_min_i = 0, true_min_j = 0;
    for (size_t j = 0; j < hidden_npcs.solar_sizes; ++j) {
        for (size_t i = 0; i < hidden_npcs.tes_vols; ++i) {
            const float p = hidden_npcs.at(i, j);
            if (p < true_min_npc) {
                true_min_npc = p;
                true_min_i = i;
                true_min_j = j;
            }

        }
    }

    float min_npc = 1000000;
    float max_gradient_tes = 0, max_gradient_solar = 0; // gradient of steepest segment
    float no_npc = 1000000;
    std::vector<float> npcs(hidden_npcs.tes_vols * hidden_npcs.solar_sizes, no_npc);


    std::vector<int> tes_indices = linearly_space(hidden_npcs.tes_vols - 1, std::max(hidden_npcs.tes_vols / subdivisions, 3));
    std::vector<int> solar_indices = linearly_space(hidden_npcs.solar_sizes - 1, std::max(hidden_npcs.solar_sizes / subdivisions, 3));

    //for (auto& i : tes_indices) {
    //    std::cout << i << ", ";
    //}
    //std::cout << '\n';

    //for (auto& i : solar_indices) {
    //    std::cout << i << ", ";
    //}
    //std::cout << '\n';

    std::vector<SubSurface> subsurfaces;
    // save space by just storing top and bottom corner
    for (size_t j = 0; j < solar_indices.size() - 1; ++j) {
        for (size_t i = 0; i < tes_indices.size() - 1; ++i) {
            SubSurface subsurface = {
                {tes_indices.at(i), solar_indices.at(j)},
                {tes_indices.at(i + 1), solar_indices.at(j + 1)},
            };
            subsurfaces.emplace_back(subsurface);
        }
    }

    for (auto& subsurface : subsurfaces) {
        npcs.at(subsurface.p11.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols) = hidden_npcs.at(subsurface.p11.tes_index, subsurface.p11.solar_index);
        npcs.at(subsurface.p22.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols) = hidden_npcs.at(subsurface.p22.tes_index, subsurface.p11.solar_index);
        npcs.at(subsurface.p22.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols) = hidden_npcs.at(subsurface.p22.tes_index, subsurface.p22.solar_index);
        npcs.at(subsurface.p11.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols) = hidden_npcs.at(subsurface.p11.tes_index, subsurface.p22.solar_index);

        float p11 = npcs.at(subsurface.p11.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols);
        float p21 = npcs.at(subsurface.p22.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols);
        float p22 = npcs.at(subsurface.p22.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols);
        float p12 = npcs.at(subsurface.p11.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols);

        float min_node = find_min(find_min(p11, p21), find_min(p22, p12));
        if (min_node < min_npc) min_npc = min_node;

        float gradient_tes = std::abs((p21 - p11) / (subsurface.p22.tes_index - subsurface.p11.tes_index));
        float gradient_solar = std::abs((p12 - p11) / (subsurface.p22.solar_index - subsurface.p11.solar_index));

        if (gradient_tes > max_gradient_tes) max_gradient_tes = gradient_tes;
        if (gradient_solar > max_gradient_solar) max_gradient_solar = gradient_solar;
    }

    //std::cout << "true_min_npc: " << true_min_npc << '\n';
    //std::cout << "min_npc: " << min_npc << ", max_gradient_tes: " << max_gradient_tes << ", max_gradient_solar: " << max_gradient_solar << '\n';

    float gradient_tes = max_gradient_tes * gradient_factor;
    float gradient_solar = max_gradient_solar * gradient_factor;

    while (!subsurfaces.empty()) {
        for (auto& subsurface : subsurfaces) {
            //std::cout << "[" << subsurface.p11.tes_index << ", " << subsurface.p11.solar_index << "], "
            //    << "[" << subsurface.p22.tes_index << ", " << subsurface.p22.solar_index << "]\n";
        }

        //for (size_t j = 0; j < hidden_npcs.solar_sizes; ++j) {
        //    for (size_t i = 0; i < hidden_npcs.tes_vols; ++i) {
        //        float npc = npcs.at(i + j * hidden_npcs.tes_vols);
        //        if (npc == no_npc) {
        //            std::cout << "-";
        //        }
        //        else {
        //            std::cout << "#";
        //        }

        //    }
        //    std::cout << '\n';
        //}

        //std::cout << "\n\n";
        std::vector<SubSurface> new_subsurfaces;
        for (auto& subsurface : subsurfaces) {

            // calculate distance between indices
            const size_t tes_length = subsurface.p22.tes_index - subsurface.p11.tes_index;
            const size_t solar_length = subsurface.p22.solar_index - subsurface.p11.solar_index;

            // assume length > 1 as it is checked when creating a new segment

            // get npc at nodes of segment
            float p11 = npcs.at(subsurface.p11.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols);
            float p21 = npcs.at(subsurface.p22.tes_index + subsurface.p11.solar_index * hidden_npcs.tes_vols);
            float p22 = npcs.at(subsurface.p22.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols);
            float p12 = npcs.at(subsurface.p11.tes_index + subsurface.p22.solar_index * hidden_npcs.tes_vols);

            // get node with lowest npc
            float min_node = find_min(find_min(p11, p21), find_min(p22, p12));
            // estimate minimum npc between nodes
            float min_segment = min_node - (gradient_tes * tes_length + gradient_solar * solar_length);

            // if segment could have npc lower than the current min subdivide
            if (min_segment < min_npc) {
                if (tes_length == 1 && solar_length == 1) {
                    // no more subdivision possible
                    // should not be possible to reach
                    std::cout << "UNREACHABLE!\n";
                }
                else if (tes_length == 1) {
                    const size_t midpoint_solar = subsurface.p11.solar_index + solar_length / 2;
                    std::array<Vec2t, 5> new_points = { {
                        {subsurface.p11.tes_index, midpoint_solar},
                        {subsurface.p22.tes_index, midpoint_solar},
                    } };

                    for (auto& point : new_points) {
                        size_t index = point.tes_index + point.solar_index * hidden_npcs.tes_vols;
                        // if midpoint npc does not exist calculate it
                        if (npcs.at(index) == no_npc) {
                            const float npc = hidden_npcs.at(point.tes_index, point.solar_index);
                            npcs.at(index) = npc;
                            if (npc < min_npc) min_npc = npc;
                        }
                    }

                    if (midpoint_solar - subsurface.p11.solar_index > 1) {
                        SubSurface new_subsurface = { { subsurface.p11.tes_index, subsurface.p11.solar_index }, {subsurface.p22.tes_index, midpoint_solar} };
                        new_subsurfaces.emplace_back(new_subsurface);
                    }

                    if (subsurface.p22.solar_index - midpoint_solar > 1) {
                        SubSurface new_subsurface = { { subsurface.p11.tes_index, midpoint_solar }, {subsurface.p22.tes_index, subsurface.p22.solar_index} };
                        new_subsurfaces.emplace_back(new_subsurface);
                    }
                }
                else if (solar_length == 1) {
                    const size_t midpoint_tes = subsurface.p11.tes_index + tes_length / 2;

                    std::array<Vec2t, 5> new_points = { {
                      {midpoint_tes, subsurface.p11.solar_index},
                      {midpoint_tes, subsurface.p22.solar_index},
                    } };

                    for (auto& point : new_points) {
                        size_t index = point.tes_index + point.solar_index * hidden_npcs.tes_vols;
                        // if midpoint npc does not exist calculate it
                        if (npcs.at(index) == no_npc) {
                            const float npc = hidden_npcs.at(point.tes_index, point.solar_index);
                            npcs.at(index) = npc;
                            if (npc < min_npc) min_npc = npc;
                        }
                    }

                    if (midpoint_tes - subsurface.p11.tes_index > 1) {
                        SubSurface new_subsurface = { { subsurface.p11.tes_index, subsurface.p11.solar_index }, {midpoint_tes, subsurface.p22.solar_index} };
                        new_subsurfaces.emplace_back(new_subsurface);
                    }

                    if (subsurface.p22.tes_index - midpoint_tes > 1) {
                        SubSurface new_subsurface = { { midpoint_tes, subsurface.p11.solar_index }, {subsurface.p22.tes_index, subsurface.p22.solar_index} };
                        new_subsurfaces.emplace_back(new_subsurface);
                    }
                }
                else {
                    // midpoint can be found for both axes

                    // calculate midpoint index of segment
                    const size_t midpoint_tes = subsurface.p11.tes_index + tes_length / 2;
                    const size_t midpoint_solar = subsurface.p11.solar_index + solar_length / 2;


                    //std::cout << midpoint_tes << ", " << midpoint_solar << "\n";


                    std::array<Vec2t, 5> new_points = { {
                        {subsurface.p11.tes_index, midpoint_solar},
                        {subsurface.p22.tes_index, midpoint_solar},
                        {midpoint_tes, subsurface.p11.solar_index},
                        {midpoint_tes, subsurface.p22.solar_index},
                        {midpoint_tes, midpoint_solar}
                    } };

                    for (auto& point : new_points) {
                        size_t index = point.tes_index + point.solar_index * hidden_npcs.tes_vols;
                        // if midpoint npc does not exist calculate it
                        if (npcs.at(index) == no_npc) {
                            const float npc = hidden_npcs.at(point.tes_index, point.solar_index);
                            npcs.at(index) = npc;
                            if (npc < min_npc) min_npc = npc;
                        }
                    }

                    // create new segments if the segments can be subdivided (length > 1)
                    // should be xor?
                    // p11
                    if (midpoint_tes - subsurface.p11.tes_index > 1 || midpoint_solar - subsurface.p11.solar_index > 1) {
                        SubSurface new_subsurface = { { subsurface.p11.tes_index, subsurface.p11.solar_index }, {midpoint_tes, midpoint_solar} };
                        new_subsurfaces.emplace_back(new_subsurface);
                    }

                    // p21
                    if (subsurface.p22.tes_index - midpoint_tes > 1 || midpoint_solar - subsurface.p11.solar_index > 1) {
                        SubSurface new_subsurface = { { midpoint_tes, subsurface.p11.solar_index }, {subsurface.p22.tes_index, midpoint_solar} };
                        new_subsurfaces.emplace_back(new_subsurface);
                    }

                    // p22
                    if (subsurface.p22.tes_index - midpoint_tes > 1 || subsurface.p22.solar_index - midpoint_solar > 1) {
                        SubSurface new_subsurface = { { midpoint_tes, midpoint_solar }, {subsurface.p22.tes_index, subsurface.p22.solar_index} };
                        new_subsurfaces.emplace_back(new_subsurface);
                    }

                    // p12
                    if (midpoint_tes - subsurface.p11.tes_index > 1 || subsurface.p22.solar_index - midpoint_solar > 1) {
                        SubSurface new_subsurface = { { subsurface.p11.tes_index, midpoint_solar }, {midpoint_tes, subsurface.p22.solar_index} };
                        new_subsurfaces.emplace_back(new_subsurface);
                    }
                }
            }
        }
        subsurfaces = new_subsurfaces;
    }

    int points_searched = 0;
    for (size_t j = 0; j < hidden_npcs.solar_sizes; ++j) {
        for (size_t i = 0; i < hidden_npcs.tes_vols; ++i) {
            float npc = npcs.at(i + j * hidden_npcs.tes_vols);
            if (npc == no_npc) {
                //std::cout << "-";
            }
            else {
                //std::cout << "#";
                points_searched++;
            }

        }
       // std::cout << '\n';
    }

    surfaces++;
    float saving = (static_cast<float>(points_searched) / static_cast<float>(npcs.size())) * 100.0f;
    efficiency += saving;
    if (min_npc == true_min_npc) {
        passed++;
        //std::cout << "PASS ";
    }
    else {
        //std::cout << "FAIL XXXXXX ";
        //std::cout << "i: " << file_index << ", final NPC: " << min_npc << ", true NPC: " << true_min_npc << ", Points Searched: " << points_searched << ", Saving: " << saving << '\n';

    }

    //for (size_t j = 0; j < hidden_npcs.solar_sizes; ++j) {
    //    for (size_t i = 0; i < hidden_npcs.tes_vols; ++i) {
    //        float npc = npcs.at(i + j * hidden_npcs.tes_vols);
    //        if (npc == no_npc) {
    //            if (i == true_min_i && j == true_min_j) {
    //                std::cout << "O";
    //            }
    //            else {
    //                std::cout << "-";
    //            }

    //        }
    //        else {
    //            if (i == true_min_i && j == true_min_j) {
    //                std::cout << "$";
    //            }
    //            else {
    //                std::cout << "#";
    //            }
    //        }

    //    }
    //    std::cout << '\n';
    //}

    
}

int main()
{
    for (float gf = 0.0f; gf < 0.5; gf += 0.02f) {
        for (int subdivisions = 3; subdivisions < 15; subdivisions++) {
            int passed = 0;
            float efficiency = 0;
            int surfaces = 0;
            //int subdivisions = 8; // every n points
            //surface_minima_finder4(190, 0.15f, subdivisions, passed, efficiency, surfaces);
            for (int i = 0; i < 2020; i++) {
                surface_minima_finder4(i, gf, subdivisions, passed, efficiency, surfaces);
            }
            
            //std::cout << "gf: " << gf << ", sd: " << subdivisions << ", passed " << static_cast<float>(passed) / static_cast<float>(surfaces) * 100.0f << "%, efficiency " << efficiency / surfaces << "%\n";
            std::cout << static_cast<float>(passed) / static_cast<float>(surfaces) * 100.0f << "," << efficiency / surfaces << "\n";
        }
    }

    
    //readInputFile("input_list.csv");
    //runSimulationWithDefaultParameters();
}