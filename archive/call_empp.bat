cd ..\emsdk
call emsdk_env.bat
em++ -O3 -std=c++20 "C:\dev\wasm_website\wasm_simulator_v2\heatninja2.cpp" "C:\dev\wasm_website\wasm_simulator_v2\main.cpp" -o "C:\dev\wasm_website\docs\cpp.js" -s EXPORTED_FUNCTIONS=["_run_simulation"] -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap"] --preload-file "C:\dev\wasm_website\wasm_simulator_v2\assets@assets" -s LLD_REPORT_UNDEFINED
pause