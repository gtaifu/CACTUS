# CACTUS(Control Architecture Simulator)

## Version

The latest version is `1.1.1`.

## Introduction

To our understanding, a heterogeneous classical-quantum computing architecture contains three computing parts (as shown in the following picture):

<img src="docs/figs/heterogeneous_arch.png?raw=true" width="500">

- a powerful classical host, which can be a classical CPU, or a cluster, or even a supercomputer;
  - Responsible for complex classical computing as well as loading the quantum task on the quantum coprocessor;
- a quantum control processor, which can execute a quantum instruction set architecture (QISA), such as eQASM, which consists of quantum instructions and auxiliary classical instructions:
  - The execution result of quantum instructions applies quantum operations on qubits, and
  - that of auxiliary classical instructions updates classical registers and control program flow
- and a quantum core consisting of multiple qubits.
  - Performing quantum state evolution.

CACTUS is a **cycle-accurate architectural simulator** for the _quantum control processor_. The goals of CACTUS include and are not limited by:

- To simulate the execution of eQASM instructions, which can be used for quantum software debugging;
- To serve verification of the VHDL design;
- To perform design space exploration of QuMA
- ...



## Dependencies

CACTUS depends on _SystemC_ and building CACTUS relies on _cmake_. The following tools with corresponding minimium versions are required:

- **cmake**: v3.11.0
- **SystemC**: v2.3.2
- **C++ compiler**
  - Windows: VS2017. VS2019 can also be used, however, with **cmake v3.14.5**
  - Linux: GCC v7.4.0
- **Python3**
- **Qubit state simulator**
  - Currently, we use [quantumsim](https://gitlab.com/quantumsim/quantumsim) with the version **stable/v0.2**.
  - More qubit simulators are being connected, such as **PQSim** and **QI-Circuit**.

### SystemC
This subsection introduce how to build SystemC. SystemC can be downloaded [here](https://www.accellera.org/downloads/standards/systemc).

#### Installation Under Windows
SystemC can be built in either Makefile-based compilation and VS-solution-based compilation. Note, SystemC should have been built at least in the same method as CACTUS. If both Makefile-based compilation and VS-solution-based compilation are used to build SystemC, then CACTUS can be built in either method. Building SystemC is both methods is suggested.

**Makefile-based compilation**:

Enter the root directory of SystemC and type the following commands:

``` PowerShell
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```

**Visual-Studio-solution-based compilation**:
Enter the root directory of SystemC and type the following commands:

``` PowerShell
mkdir vsbuild
cd vsbuild
cmake -G "Visual Studio 15 2017 Win64" ..
cmake --build .   #  --config release or --config debug, default is debug
```

**Regarding VS2019, the above cmake command should be changed to**:

``` PowerShell
mkdir vsbuild
cd vsbuild
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build .          #  --config release  or  --config debug, default is debug
```

Modify the last lines of systemc-2.3.2\src\sysc\packages\boost\config\compiler\visualc.hpp as follow if an error "Unknown compiler version - please run the configure tests and report the results" is generated at the time of compilation:
```C++
#if (_MSC_VER > 1990)
#  if defined(SC_BOOST_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  else
#     pragma message("Unknown compiler version - please run the configure tests and report the results")
#  endif
#endif
```


After the above steps have been done, both makefile-based compilation and VS-solution-based compilation should be possible.


#### Installation Under Ubuntu
This is tested under gcc (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0.

Enter the root directory of SystemC and type the following commands:

``` bash
mkdir build
cd build
cmake .. -DCMAKE_CXX_STANDARD=11  # -DCMAKE_BUILD_TYPE=debug or -DCMAKE_BUILD_TYPE=release
make
```

### Python3
#### Windows OS
In Windows, Python3 can be installed via installing Anaconda3. After Anaconda is installed, e.g., in `C:\Anacanda3`, then the following entries should be added to the environment variable `Path`:
- `C:\Anaconda3`
- `C:\Anaconda3\Scripts`
- `C:\Anaconda3\Library\bin`

In addition, a new variable `PYTHONHOME` should be created with the value `C:\Anaconda3`.

#### Ubuntu
Please ensure that python3-dev has been installed, which can be installed by:

``` bash
sudo apt-get install python3-dev
```

Without `python3-dev`, cmake will raise the following error:
```
  Could NOT find PythonLibs (missing: PYTHON_LIBRARIES PYTHON_INCLUDE_DIRS)
```

### QuantumSim
The default qubit simulator for CACTUS is currently [QuantumSim](https://gitlab.com/quantumsim/quantumsim). It can be fetched using the following command:

```
git clone git@gitlab.com:quantumsim/quantumsim.git
```

However, **CACTUS uses an older but stable version of QuantumSim, of which the latest code is in the branch `stable/v0.2`**.

After entering the root directory of quantumsim, the required version of QuantumSim can be installed using the following commands:

```
git checkout stable/v0.2
python setup.py develop
```

To verify the installation of QuantumSim, type the following command in the root directory of QuantumSim:
```
pytest.exe
```

## Build the Simulator

### Windows OS
CACTUS can be built using MSVC withing Visual Studio or in the command line. There are two different methods (**makefile-based compilation** and **visual-studio-solution-based compilation**) to build CACTUS.

In the former, a Makefile is generated, and the Windows-make tool `nmake` is used to compile this project. The advantage of this method: it is similar to the compilation in Linux. Disadvantage: the compilation speed is slow.

In the latter, a VS solution is generated, and the Visual Studio builds the projects. The advantage of this method: the Ninja build system can be used, which can significantly reduce the compilation time. Disadvantage: Many other files are generated, which may look messy.

### Makefile-based Compilation

Enter the root directory of CACTUS and type the following commands:

``` PowerShell
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```

The generated executables will be in the directory `<CACTUS_root>\build\bin`.


### VS-Solution-based Compilation

Enter the root directory of CACTUS and type the following commands:

``` PowerShell
mkdir vsbuild
cd vsbuild
cmake -G "Visual Studio 15 2017 Win64" ..
cmake --build .  #  --config release or --config debug, default is debug
```

**Regarding VS2019, the above cmake command should be changed to**：

``` PowerShell
mkdir vsbuild
cd vsbuild
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build .  #  --config release or --config debug, default is debug
```

The generated executables will be in the directory `<CACTUS_root>\vsbuild\bin`.


### Ubuntu
This is tested under gcc (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0.

Enter the root directory of CACTUS and type the following commands:

``` PowerShell
mkdir build
cd build
cmake ..
make   # -DCMAKE_BUILD_TYPE=debug or -DCMAKE_BUILD_TYPE=release
```

Note: we have not yet performed enough test under OS other than Windows till now. If you see any problems, please report the problem as an issue in the CACTUS repository or write an email to Xiang Fu: gtaifu@gmail.com.


## Execution

The generated executable of CACTUS is `cactus` or `cactus.exe`. It accepts some parameters such as the path of the configuration files for execution. Option `-h` or `--help` is used to show the usage information. Some test cases are provided in directory `<CACTUS_root>\demo\`.
```Powershell
cactus -h   # get usage information
```

NOTE: `Cactus` will start execution by reading the file `log_levels.json` within the same directory if option `-l` has not assigned a logging level configuration file. Cmake will copy this file from `<CACTUS_root>\test_files\` to the output binary directory automatically.
```C++
    trace = 0,  // log_levels which are support in file log_levels.json
    debug = 1,
    info = 2,
    warning = 3,
    error = 4,
    critical = 5,
    off = 6
```

The eQASM binary fed to the simulation is the file specified by `-b` option or specified by the value corresponding to the key "qisa binary" in the file which is specified by `-c` option. The eQASM binary can be generated using the assembler [eqasm_assembler](https://gitlab.com/hpcl_quanta/eqasm_assembler). However, the configuration to the microarchitecture is still too complicated. A uniformed format of the input to the microarchitecture will be developed.

The eQASM assembly fed to the simulation is the file specified by `-a` option or specified by the value corresponding to the key "qisa assemble" in the file which is specified by `-c` option.

### Usage

Option `-h` or `--help` is used to get usage information .

NOTE: Options except `-c` are used to specify part of the settings for CACTUS while all settings will be read from the configuration file specified by `-c` option that means `-c` option has the highest priority.

Before reading the command line options, CACTUS has been initialized with the default built-in settings those are the same as the default option values.

You can start the test case simply with typing a command as follows:

```
.\bin\cactus -a .\test_cases\echo.eqasm  # -a option used to specify a eqasm file
```

```
Available parameters:

  -h    --help

   This parameter is optional. The default value is ''.

  -a    --asm
   Specify assembly file which fed to simulation.
   This parameter is optional. The default value is ''.

  -b    --bin
   Specify binary file which fed to simulation.
   This parameter is optional. The default value is ''.

  -c    --config
   Specify configuration file which includes all configs. A typical configuration file is <CACTUS_root>\test_files\test_input_file_list.json.
   This parameter is optional. The default value is ''.

  -d    --dm_size
   Specify data memory size, size unit can be "M" or "K".
   This parameter is optional. The default value is '1M'.

  -f    --file
   Specify the name of data memory dump file. Memory will not dump to a file if file name is not specified by '-f'.
   This parameter is optional. The default value is ''.

  -g    --gate_config
   Specify qubit gate configuration file. A typical configuration file is <CACTUS_root>\test_files\hw_config\qubit_gate_config.json.
   This parameter is optional. The default value is ''.

  -l    --log_level
   Specify log level configuration file. A configuration config file is <CACTUS_root>\test_files\log_levels.json.
   This parameter is optional. The default value is ''.

  -m    --mock_meas
   Specify the file name of mock measurement result.
   This parameter is optional. The default value is ''.

  -n    --q_num
   Specify qubit number.
   This parameter is optional. The default value is '7'.

  -o    --output
   Specify ouput directory for simluation intermediate output.
   This parameter is optional. The default value is './sim_output/'.

  -q    --q_sim
   Specify qubit simulator, 0 for Quantumsim and 1 for QIcircuit.
   This parameter is optional. The default value is '0'.

  -r    --run
   Specify total simulation cycles.
   This parameter is optional. The default value is '3000'.

  -s    --store
   Specify memory start address and size that will be written into file. The base used is determined by the format. It is used like the one "-s 0x0000 0x10000".
   This parameter is optional. The default value is '[ 0x0000 0x10000 ]'.

  -t    --tp_config
   Specify topology configuration file. A typical configuration file is <CACTUS_root>\test_files\hw_config\cclight_config.json.
   This parameter is optional. The default value is ''.

  -v    --vliw_width
   Specify VLIW width.
   This parameter is optional. The default value is '2'.
```

### Configuration file list

The configuration file `test_input_file_list.json` is parsed as follow:
```
    "quantum chip topology": "hw_config/cclight_config.json"      # hardware setting
    "qisa binary": "qvm_test/prog_1.bin"                          # the eQASM binary fed to the simulation
    "qisa assemble": "qvm_test/test_vliw_width.eqasm"             # the eQASM assembly fed to the simulation
    "output directory": "sim_output/"                             # the output directory for simulation intermediate output file
    "qubit_gate_config": "qvm_test/qubit_gate_config.json"        # used to mapping the qubit gate between eQASM and qubit simulator
    "mock result": "./mock_result"                                # the mock measurement result will output to this file
```

#### quantum chip topology

Main configured parameters used in `cclight_config.json` are as follows, the remain parameters are not used in current version, but reserved for future.

```
   "num_sim_cycles": 100        # total running cycles of simulation
   "instruction_type": 1,       # specify the input code is binary or assembly, 0 for binary, 1 for assembly
   "qubit_simulator": 0,        # specify the qubit simulator which will be used, 0 for Quantumsim, 1 for QIcircuit
   "hardware_settings": {
      "qubit_number": 7,        # the number of total qubits
      "vliw_width": 3,          # VLIW width
      "data_memory_size": "1M", # the size of data memory used for load and store instructions
   }
```

#### qubit gate config

File `qubit_gate_config.json` specifies the qubit gates mapping between gate, codeword, gate duration time.

```
"gates_of_eqasm":{
  "assembly": {                  # when running assembly eQASM file, this mapping will be used
    "single_qubit_gate": {}      # defines the mapping between single-qubit gate and gate duration time
    "two_qubit_gate":{}          # defines the mapping between two-qubit gate and gate duration time
  }
  "binary":{                     # when running binary eQASM file, this mapping will be used
    "num_codeword":256           # number of total codewords
    "single_qubit_gate": {}      # defines the mapping between op_code and single-qubit gate, gate duration time
    "two_qubit_gate":{}          # defines the mapping between op_code and two-qubit gate, gate duration time
  }
}
```

## Implemented gates of CACTUS

| Name       | Qubits | Params | Description                                                                                                                                                                                                      |
| ---------- | ------ | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| h          | 1      |        | Hadamard gate                                                                                                                                                                                                    |
| x          | 1      |        | Pauli X (180 degrees rotation over X-axis)                                                                                                                                                                       |
| y          | 1      |        | Pauli Y (180 degrees rotation over Y-axis)                                                                                                                                                                       |
| z          | 1      |        | Pauli Z (180 degrees rotation over Z-axis)                                                                                                                                                                       |
| s          | 1      |        | 90 degrees rotation over Z-axis                                                                                                                                                                                  |
| sdg        | 1      |        | -90 degrees rotation over Z-axis                                                                                                                                                                                 |
| t          | 1      |        | 45 degrees rotation over Z-axis                                                                                                                                                                                  |
| tdg        | 1      |        | -45 degrees rotation over Z-axis                                                                                                                                                                                 |
| x$\theta$  | 1      | theta  | Rotation around the X-axis by given angle, $\theta$ is in the range of 0 to 180 and decimal point is replaced by `_`. An example is that x175_5 represents rotation around X-axis by given angle 175.5           |
| y$\theta$  | 1      | theta  | Rotation around the Y-axis by given angle, $\theta$ is in the range of 0 to 180 and decimal point is replaced by `_`. An example is that y175_5 represents rotation around Y-axis by given angle 175.5           |
| z$\phi$    | 1      | phi    | Rotation around the Z-axis by given angle, $\phi$ is in the range of 0 to 180 and decimal point is replaced by `_`. An example is that z175_5 represents rotation around Z-axis by given angle 175.5             |
| xm$\theta$ | 1      | theta  | Reverse rotation around the X-axis by given angle, $\theta$ is in the range of 0 to 180 and decimal point is replaced by `_`. An example is that xm175_5 represents rotation around X-axis by given angle -175.5 |
| ym$\theta$ | 1      | theta  | Reverse rotation around the Y-axis by given angle, $\theta$ is in the range of 0 to 180 and decimal point is replaced by `_`. An example is that ym175_5 represents rotation around Y-axis by given angle -175.5 |
| zm$\phi$   | 1      | phi    | Reverse rotation around the Z-axis by given angle, $\phi$ is in the range of 0 to 180 and decimal point is replaced by `_`. An example is that zm175_5 represents rotation around Z-axis by given angle -175.5   |
| rx$\theta$ | 1      | theta  | rx$\theta$ is the same as x$\theta$                                                                                                                                                                              |
| ry$\theta$ | 1      | theta  | ry$\theta$ is the same as y$\theta$                                                                                                                                                                              |
| rz$\phi$   | 1      | phi    | rz$\phi$ is the same as z$\phi$                                                                                                                                                                                  |
| cz         | 2      |        | Controlled Pauli Z (180 degrees rotation over Z-axis)                                                                                                                                                            |
| measure    | 1      |        | Measure                                                                                                                                                                                                          |
| mock_meas  |        |        | Mock_meas will output density matrix                                                                                                                                                                             |
`Note:` The name of implemented gates are case insensitive. Mock_meas will output density matrix of qubit state simulator.

## Suggested tools used in this project

The following tools are suggested:

- `Clang-Format` to enable a unified code format across the entire project.

## Contributors

Dr. Xiang Fu initiated this project (CACTUS) and wrote a draft version of CACTUS from 2016 to 2017, when he performed research in quantum control architecture at QuTech, TU Delft. Mengyu Zhang joined this project in December 2017. Based on Xiang's draft version, he finished an early version of CACTUS. Xiang restructured the code into a more readable and stable version after Mengyu graduates in August, 2018.

Currently, CACTUS is maintained by Xiang Fu (HPCL@NUDT) and Peng Zhou (CQC@PCL).

## Contribution

If you have any questions, please feel free to make an issue associated with this repository, or send emails to Xiang Fu: gtaifu at GMAIL.

## Common issues

See the [FAQ](FAQ.md).