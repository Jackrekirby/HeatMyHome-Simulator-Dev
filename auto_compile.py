import clipboard
import subprocess
import os

from datetime import datetime

now = datetime.now()
dt_string = now.strftime("%d/%m/%Y %H:%M:%S")
print(f'Auto Compile: {dt_string}')

# ____ INPUTS ____________________________________________________________

print(f'Reading Inputs')

cwd = os.getcwd() + '\\'
print(f'Current Working Directory: {cwd}')

# emscripten inputs
cpp_dir = cwd + 'WebAssemblyTesting\\'
cpp_files = ['animal.cpp', 'main.cpp']
cpp_preload_file = 'assets'
virtual_preload_file = 'assets'

web_dir = cwd + ''
web_file = 'cpp.js'

exported_functions = ['return_vector', 'print_outside_temps',
                      'print_example_file', 'call_class', 'int_sqrt']
exported_runtime_methods = ['ccall', 'cwrap']

batch_file = 'archive\\call_empp.bat'

# javascript file merge inputs
js_merge_file = 'index.js'
js_files = ['extra.js', web_file]
just_js = True

# ____ AUTOMATED ________________________________________________________

if not just_js:
    print(f'Generating Emscripten Command')

    cmd = 'em++ '

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

    print(cmd)

    print(f'Generating Batch File')
    f = open(batch_file, 'w')
    f.write('cd C:\\dev\\emsdk\\' + '\n')
    f.write('call emsdk_env.bat' + '\n')
    f.write(cmd + '\n')
    f.write('pause')
    f.close()

    print(f'Calling Batch File')
    subprocess.call(batch_file)


print(f'Merging JavaScript Files')
f = open(js_merge_file, "w")
f.write(f'console.log("Last Updated: {dt_string}");\n\n')
for filename in js_files:
    f.write(
        f"// {f'&{filename}&'.center(77).replace(' ', '_').replace('&', ' ')}\n\n{open(filename, 'r').read()}\n\n")
f.close()

print(f'Complete')
