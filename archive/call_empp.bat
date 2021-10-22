cd C:\dev\emsdk\
call emsdk_env.bat
em++ "C:\dev\wasm_website\WebAssemblyTesting\animal.cpp" "C:\dev\wasm_website\WebAssemblyTesting\main.cpp" -o "C:\dev\wasm_website\cpp.js" -s EXPORTED_FUNCTIONS=["_return_vector","_print_outside_temps","_print_example_file","_call_class","_int_sqrt"] -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap"] --preload-file "C:\dev\wasm_website\WebAssemblyTesting\assets@assets" -s LLD_REPORT_UNDEFINED
pause