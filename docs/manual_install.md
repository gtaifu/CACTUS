## Manual Installation

### cmake
- Go to the website `https://cmake.org/download/` and download `cmake-3.11.0-win64-x64.msi`.
- Execute `cmake-3.11.0-win64-x64.msi`. Note: please add cmake into the system path during the installation.
- If you forgot to add the path of cmake into the system path, please use the following commands in PowerShell to add it:
```PowerShell
    $cmakePath = "C:\Program Files\CMake\bin"
    [Environment]::SetEnvironmentVariable("path",
        [Environment]::GetEnvironmentVariable("path", "user") + ";C:\Program Files\CMake\bin", "user")
```
- Restart your PowerShell and you should be able to use cmake in PowerShell

### SystemC
- download `systemc 2.3.2` from http://www.accellera.org/downloads/standards/systemc. Please select the `Core SystemC Language and Examples` with the version number **2.3.2**.
- unpack systemc_2.3.2 into an installation directory as you wish. For example, `C:\Program Files\SystemC\`.
- Enter the unpacked SystemC direcotry. For example, `cd C:\Program Files\SystemC\`.
- create a `build` directory and enter it. For example, `mkdir build; cd build`.
- configure the SystemC build using the following command:
```
cmake ../ -DCMAKE_CXX_STANDARD=11 -DCMAKE_BUILD_TYPE=Debug
```
- build SystemC with the following command:
```
cmake --build <dir>
```
Now you could find the systemc.lib file in `\src\Debug` folder.
