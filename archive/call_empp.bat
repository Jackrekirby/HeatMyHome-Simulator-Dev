cd ..\emsdk
call emsdk_env.bat
em++ -O3 -std=c++20 "C:\dev\heatmyhome_simulator\wasm_simulator_v2\heatninja.cpp" "C:\dev\heatmyhome_simulator\wasm_simulator_v2\main.cpp" -o "C:\dev\heatmyhome_simulator\docs\cpp.js" -s EXPORTED_FUNCTIONS=["_run_simulation"] -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap"] --preload-file "C:\dev\heatmyhome_simulator\wasm_simulator_v2\assets@assets" -s LLD_REPORT_UNDEFINED
pause