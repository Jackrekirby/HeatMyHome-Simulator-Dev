import clipboard
import subprocess
import os

from datetime import datetime

now = datetime.now()
dt_string = now.strftime("%d/%m/%Y %H:%M:%S")
print(f'\n\n______________________________ Auto Compile ______________________________\n')
print(f' - 1 - Compiling at: {dt_string}')

# ____ INPUTS ____________________________________________________________

print(f' - 2 - Reading Inputs')

cwd = os.getcwd() + '\\'
print(f' - 3 - Current Working Directory: {cwd}')

# emscripten inputs
cpp_dir = cwd + 'wasm_simulator\\'
cpp_files = ['animal.cpp', 'main.cpp']
cpp_preload_file = 'assets'
virtual_preload_file = 'assets'
compiler_optimisation = '-O3 '

emsdk_dir = '..\\emsdk'
emsdk_env_bat = 'emsdk_env.bat'

web_dir = cwd + ''
web_file = 'cpp.js'

exported_functions = ['return_vector', 'print_outside_temps',
                      'print_example_file', 'call_class', 'int_sqrt', 'speed_test']
exported_runtime_methods = ['ccall', 'cwrap']

batch_file = 'archive\\call_empp.bat'

# javascript file merge inputs
js_merge_file = 'index.js'
js_files = ['extra.js', web_file]
just_js = False

# ____ AUTOMATED ________________________________________________________

if not just_js:
    print(f' - 4 - Generating Emscripten Command')

    cmd = f'em++ {compiler_optimisation}'

    for file in cpp_files:
        cmd += f'"{cpp_dir}{file}" '

    cmd += f'-o "{web_dir}{web_file}" '

    cmd += f'-s EXPORTED_FUNCTIONS=['

    for i, fnc in enumerate(exported_functions):
        cmd += f'"_{fnc}"'
        if i + 1 < len(exported_functions):
            cmd += ','

    cmd += f'] -s EXPORTED_RUNTIME_METHODS=['

    for i, method in enumerate(exported_runtime_methods):
        cmd += f'"{method}"'
        if i + 1 < len(exported_runtime_methods):
            cmd += ','

    cmd += f'] --preload-file "{cpp_dir}{cpp_preload_file}@{virtual_preload_file}" '
    cmd += '-s LLD_REPORT_UNDEFINED'

    print(f'\n{cmd}\n')

    print(f' - 5 - Generating Batch File')
    f = open(batch_file, 'w')
    f.write(f'cd {emsdk_dir}' + '\n')
    f.write(f'call {emsdk_env_bat}' + '\n')
    f.write(cmd + '\n')
    f.write('pause')
    f.close()

    print(f'___________________________ Calling Emscripten ___________________________\n')
    subprocess.call(batch_file)
    print(f'\n_____________________ Completed Emscripten Compilation ___________________\n')


print(f' - 6 - Merging JavaScript Files')
f = open(js_merge_file, "w")
f.write(f'console.log("Last Updated: {dt_string}");\n\n')
for filename in js_files:
    f.write(
        f"// {f'&{filename}&'.center(77).replace(' ', '_').replace('&', ' ')}\n\n{open(filename, 'r').read()}\n\n")
f.close()

print(f' - 7 - Merged JavaScript Files')
print(f'\n__________________________ Auto Compile Completed ________________________\n')
