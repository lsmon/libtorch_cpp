import os
import platform
import subprocess
import sys
import zipfile
import shutil

root_path = ""
version = ""
os_type = sys.platform


def run_command(command):
    result = subprocess.run(command, shell=True)
    if result.returncode != 0:
        raise Exception(f"Command '{command}' failed with exit code {result.returncode}.")


def clone():
    print("Cloning cassandra cpp-driver...")
    os.chdir(f'{root_path}')
    # Clone or update nlohmann/json using Git (you need Git installed)
    if not os.path.exists("cpp-driver"):
        run_command("git clone https://github.com/datastax/cpp-driver.git cpp-driver")
    else:
        os.chdir("cpp-driver")
        # run_command("git pull")
    print("Cloning completed.")


def build():
    print("Building cassandra cpp-driver...")

    path = os.path.join(root_path, "cpp-driver")
    build = os.path.join(path, "build")
    os.chdir(path)
    os.makedirs(build, exist_ok=True)

    cmake_bin = ""
    compiler_flag = ""
    if os_type == "darwin": 
        cmake_bin = "/opt/homebrew/bin/cmake"
        compiler_flag =  " -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
        compiler_flag += " -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang"
        compiler_flag += " -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++"
        compiler_flag += " -DCMAKE_OSX_ARCHITECTURES=arm64"
        compiler_flag += " -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0"
        compiler_flag += " -DCMAKE_CXX_FLAGS=\"-std=c++17 -stdlib=libc++\""
    else:
        cmake_bin = "cmake"
    cmake_cmd = cmake_bin
    # cmake_cmd += " -DCMAKE_POLICY_VERSION_MINIMUM=3.10"
    cmake_cmd += " -Wno-error=nontrivial-memcall"
    cmake_cmd += " -DCMAKE_BUILD_TYPE:STRING=Debug"
    cmake_cmd += " -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE"
    
    cmake_cmd += compiler_flag
    cmake_cmd += " -S" + path
    cmake_cmd += " -B" + build
    print(cmake_cmd)
    run_command(cmake_cmd)
    cmake_cmd = cmake_bin + " --build " + build + " --config Debug --target all"
    print(cmake_cmd)
    run_command(cmake_cmd)
    print("Building completed.")


def install():
    _path = os.path.join(root_path, "cpp-driver")
    _build = os.path.join(_path, "build")
    _inc = os.path.join(_path, "include")
    lib_path = os.path.join(root_path, "lib")
    inc_path = os.path.join(root_path, "include")

    if not(os.path.exists(lib_path)):
        os.makedirs(lib_path)

    os_name = platform.system()
    if os_name == "Darwin":
        _lib = os.path.join(_build, "libcassandra." + version + ".dylib")
        print("copying " + _lib + " to " + lib_path)
        shutil.copy(_lib, lib_path)
        _lib = os.path.join(_build, "libcassandra.2.dylib")
        print("copying " + _lib + " to " + lib_path)
        shutil.copy(_lib, lib_path)
        _lib = os.path.join(_build, "libcassandra.dylib")
        print("copying " + _lib + " to " + lib_path)
        shutil.copy(_lib, lib_path)
    else:    
        _lib = os.path.join(_build, "libcassandra.so." + version)
        print("copying " + _lib + " to " + lib_path)
        shutil.copy(_lib, lib_path)
        _lib = os.path.join(_build, "libcassandra.so.2")
        print("copying " + _lib + " to " + lib_path)
        shutil.copy(_lib, lib_path)
        _lib = os.path.join(_build, "libcassandra.so")
        print("copying " + _lib + " to " + lib_path)
        shutil.copy(_lib, lib_path)

    if not os.path.exists(inc_path):
        os.makedirs(inc_path)

    copy_hpp(_inc, inc_path)


# Iterate over files in netpp_libinc and copy each file to inc_path
def copy_hpp(src_path, dst_path):
    for root, dirs, files in os.walk(src_path):
        for dir in dirs:
            src_dir = os.path.join(root, dir)
            dst_dir = os.path.join(dst_path, os.path.relpath(src_dir, src_path))
            print("Creating dir: " + dst_dir)
            os.makedirs(dst_dir, exist_ok=True)
            # shutil.copytree(src_dir, dst_dir, dirs_exist_ok=True)
        for file in files:
            src_file = os.path.join(root, file)
            dst_file = os.path.join(dst_path, os.path.relpath(src_file, src_path))
            print("Copying " + src_file + " -> " + dst_file)
            shutil.copy2(src_file, dst_file)


def main():
    clone()
    build()
    install()
    print("All done!")


if __name__ == "__main__":
    num_args = len(sys.argv)
    if num_args < 2:
        print("installation root path is needed")
    else:
        root_path = sys.argv[1]
        version = sys.argv[2]
        main()
