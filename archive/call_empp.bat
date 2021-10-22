cd C:\dev\emsdk\
call emsdk_env.bat
em++ "C:\dev\WebAssemblyTesting\animal.cpp" "C:\dev\WebAssemblyTesting\WebAssemblyTesting.cpp" -o "C:\dev\WebAssembly\cpp.js" -s EXPORTED_FUNCTIONS=["_int_sqrt","_print_file","_get_message"] -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap"] --preload-file "C:\dev\WebAssemblyTesting\assets" -s LLD_REPORT_UNDEFINED
pause