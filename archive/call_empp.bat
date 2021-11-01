cd ..\emsdk
call emsdk_env.bat
em++ -O3 -std=c++20 -s ASYNCIFY "C:\dev\wasm_website\wasm_simulator\animal.cpp" "C:\dev\wasm_website\wasm_simulator\main.cpp" -o "C:\dev\wasm_website\cpp.js" -s EXPORTED_FUNCTIONS=["_return_vector","_print_outside_temps","_print_example_file","_call_class","_int_sqrt","_speed_test","_sim_test","_sim_test_args"] -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap"] --preload-file "C:\dev\wasm_website\wasm_simulator\assets@assets" -s LLD_REPORT_UNDEFINED
pause