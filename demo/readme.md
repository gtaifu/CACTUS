## Quick run

An released executable file `cactus.exe` based on windows x64 platform has been provided in `./bin` directory.

Before running CACTUS executable file, the quantumsim should be [installed](#quantumsim). SystemC, camke and visual studio 2019 do not need install.

You can run test cases simply with typing a command as follows in a windows shell.

```Powershell
.\bin\cactus -a .\test_cases\echo.eqasm  # -a option used to specify a eqasm file
```

The message on shell *Info: /OSCI/SystemC: Simulation stopped by user.* indicates the CACTUS reaches its simulation time or has executed all quantum operation instructions.  

## Usages

You can get all command options with using *-h* or *--h* option.

If you do not want to modify CACTUS default topology configuration and default qubit gate mapping between eqasm and qubit simulator,'*-a*' or '*-b*' option is recommend.

All options will be read from the configuration file when a '*-c*' option is used.

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
   Specify memory start address and size that will be written into file. The base used is determined by the format. It is used like the format "-s 0x0000 0x10000".
   This parameter is optional. The default value is '[ 0x0000 0x10000 ]'.

  -t    --tp_config
   Specify topology configuration file. A typical configuration file is <CACTUS_root>\test_files\hw_config\cclight_config.json.
   This parameter is optional. The default value is ''.

  -v    --vliw_width
   Specify VLIW width.
   This parameter is optional. The default value is '2'.
```

## Intermediate output

Default directory for intermediate output files is `./sim_output`.

Four intermediate output files can be used to verify whether the test cases are executed correctly.

+ *classical_execute.csv* records instruction flow which are really executed. 
+ *classical_mem.csv* records data memory accesses. 
+ *classical_wb.csv* records register's update. 
+ *meas_result_gen.csv* records qubit measurement results.

## <span id="quantumsim">QuantumSim</span> 

The default qubit simulator for CACTUS is currently [QuantumSim](https://gitlab.com/quantumsim/quantumsim). It can be fetched using the following command:

```
git clone git@gitlab.com:quantumsim/quantumsim.git
```

However, **CACTUS uses an older but stable version of QuantumSim, of which the latest code is in the branch `stable/v0.2`**.

After entering the root directory of quantumsim, the required version of QuantumSim can be installed using the following commands:

```Powershell
git checkout stable/v0.2
python setup.py develop   # Quantumsim is installed as a python3 module.
```

To verify the installation of QuantumSim, type the following command in the root directory of QuantumSim:
```Powershell
pytest.exe
```

## Python3
### Windows OS
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