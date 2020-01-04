#include "if_QIcircuit.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "global_counter.h"
#include "global_json.h"
#include "interface_lib.h"
#include "json_wrapper.h"
#include "logger_wrapper.h"
#include "num_util.h"

using json = nlohmann::json;
namespace cactus {

using std::map;
using std::pair;
using std::stringstream;
using std::to_string;
using std::vector;

If_QIcircuit::If_QIcircuit(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("qsim_logger");

    logger->trace("The logger for the If_QIcircuit has been initialized.");

    config();

    init_python_api();

    SC_CTHREAD(log_telf, clock_50MHz.pos());
    SC_CTHREAD(apply_quantum_operation, clock_50MHz.pos());
}

void If_QIcircuit::config() {

    Global_config& global_config = Global_config::get_instance();

    num_qubits       = global_config.num_qubits;
    num_msmt_devices = global_config.num_msmt_devices;
    operation_time   = global_config.single_qubit_gate_time;
    operation_time.insert(global_config.two_qubit_gate_time.begin(),
                          global_config.two_qubit_gate_time.end());
    cycle_time = global_config.cycle_time;
}

void If_QIcircuit::add_telf_header() {}

void If_QIcircuit::log_telf() {
    while (true) {

        wait();

        if (is_telf_on) {
            add_telf_line();
        }
    }
}

void If_QIcircuit::add_telf_line() {}

void If_QIcircuit::init_python_api() {

    auto logger = get_logger_or_exit("qsim_logger");

    // Create the interface_QIcircuit instance and initialize the sparse density matrix with
    // the number of qubits
    logger->trace("Start initializing the interface to QIcircuit.");

    // Initialize the Python interpreter.
    try {
        Py_Initialize();
    } catch (...) {
        PyErr_Print();
        logger->error(
          "Failed to initialized Python interpreter. Please try to fix it. Simulation aborts.");
        exit(EXIT_FAILURE);
    }

    logger->trace("Py_Initialize succeeded.");

    std::string py_cmd1 = "import sys\n";
    std::string py_cmd2 = "sys.path.append(r'" + abs_dir_path_of_exe() + "')\n";
    std::string code    = py_cmd1 + py_cmd2;
    logger->debug("code to add path:\n{}", code);

    try {
        PyRun_SimpleString(code.c_str());
        // PyRun_SimpleString("print(sys.path)"); // for debuging.
    } catch (...) {
        PyErr_Print();
    }

    // Convert the file name to a Python string.
    auto pName = PyUnicode_DecodeFSDefault("interface_QIcircuit");
    if (pName == NULL) {
        PyErr_Print();
        logger->error("Failed to decode the string 'interface_QIcircuit'.");
    }

    // Import the file as a Python module.
    auto pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
        PyErr_Print();
        logger->error(
          "Failed to import the Python module 'interface_QIcircuit'. Simulation aborts!");
        exit(EXIT_FAILURE);
    } else {
        logger->trace("Successfully imported the Python module 'interface_QIcircuit'.");
    }

    // Create a dictionary for the contents of the module.
    auto pDict = PyModule_GetDict(pModule);

    // Create an instance of the Python class interface_QIcircuit
    auto pClass = PyDict_GetItemString(pDict, "interface_QIcircuit");

    if (pClass == NULL) {
        PyErr_Print();
        logger->error(
          "The class interface_QIcircuit is undefined in the module. Simulation aborts.");
        exit(EXIT_FAILURE);
    } else {
        logger->trace(
          "Successfully created an instance of the Python class "
          "'interface_QIcircuit'.");
    }

    if (PyCallable_Check(pClass)) {
        interface = PyObject_CallObject(pClass, NULL);
        if (interface == NULL) {
            logger->error(
              "Failed to call the python object to create the interface "
              "to the qubit state simulator. Simulation aborts.");
            exit(EXIT_FAILURE);
        } else {
            logger->trace("Successfully created the callable object 'interface'.");
        }
    } else {
        logger->error("Given object is not callable. Simulation aborts.");
        exit(EXIT_FAILURE);
    }

    // Initialize the spare density matrix with the number of qubits
    auto pMethod = PyUnicode_FromString("init_circuit");
    // the parameter is the number of qubits
    auto pArgs  = PyLong_FromLong(num_qubits);
    auto pValue = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    Py_DECREF(pArgs);

    if (pValue != NULL) {

        logger->trace("Successfully initialized the circuit in QIcircuit for {} qubits.",
                      num_qubits);
        Py_DECREF(pValue);
        Py_XDECREF(pMethod);
        Py_DECREF(pModule);

    } else {

        Py_DECREF(pMethod);
        Py_DECREF(pModule);
        PyErr_Print();
        logger->error(
          "Failed to initialize the circuit in QIcircuit (function init_circuit). "
          "Simulation aborts.");
        exit(EXIT_FAILURE);
    }
}

void If_QIcircuit::post_py_process(PyObject* pValue, PyObject* pMethod,
                                   const std::string& err_msg) {
    auto logger = get_logger_or_exit("qsim_logger");

    if (pValue != NULL) {
        Py_DECREF(pValue);
        Py_XDECREF(pMethod);
    } else {
        Py_DECREF(pMethod);
        PyErr_Print();
        logger->error(err_msg);
        exit(EXIT_FAILURE);
    }
}

void If_QIcircuit::apply_quantum_operation() {

    auto         logger = get_logger_or_exit("qsim_logger");
    stringstream ss;

    std::string  op_name;
    unsigned int current_cycle = 0;

    Ops_2_qsim           moment;
    vector<unsigned int> target_qubits;
    Res_from_qsim        res_from_qsim;

    while (true) {
        wait();

        moment = ops_2_qsim.read();

        // ------------------------------------------------------------
        // If there is no new quantum operation, wait until next cycle.
        // ------------------------------------------------------------
        if (!moment.triggered) {
            // clear measurement result at the begin of each cycle
            res_from_qsim.reset();
            msmt_res.write(res_from_qsim);
            continue;
        }

        current_cycle = moment.cycle;

        ss.str("");
        ss << "The following operations arrive at cycle: " << current_cycle << std::endl;
        for (size_t op_idx = 0; op_idx < moment.atom_ops.size(); op_idx++) {
            ss << moment.atom_ops[op_idx];
        }
        logger->debug("{}", ss.str());

        // iterate over all individual operations
        for (auto it_op = moment.atom_ops.begin(); it_op != moment.atom_ops.end(); it_op++) {

            target_qubits.clear();
            op_name                   = it_op->operation;
            target_qubits             = it_op->target_qubits;
            auto number_target_qubits = target_qubits.size();

            if (number_target_qubits == 1) {
                if (op_name.compare("measure") == 0) {  // If a measurement operation
                    measure_qubit(target_qubits[0]);
                    num_measure++;
                    unsigned int measurement_result = run_circuit(num_measure);

                    res_from_qsim.results.push_back(
                      std::make_pair(target_qubits[0], measurement_result));
                    logger->debug("measured qubit: {}, result: {}", target_qubits[0],
                                  measurement_result);
                    msmt_res.write(res_from_qsim);
                } else if (op_name.compare("mock_meas") == 0) {
                    logger->error("QIcircuit do not support mock measurement. Aborts!");

                    exit(EXIT_FAILURE);
                } else {
                    apply_single_qubit_gate(op_name, target_qubits[0]);
                }
            } else if (number_target_qubits == 2) {
                apply_two_qubit_gate(op_name, target_qubits[0], target_qubits[1]);
            } else
                logger->error("Three target qubits are not allowed");
        }
    }
}

void If_QIcircuit::apply_single_qubit_gate(std::string quantum_operation, unsigned int qubit) {
    auto logger = get_logger_or_exit("qsim_logger");

    if (quantum_operation.compare("Null") == 0) {  // skip the quantum nop.
        return;
    }

    // quantum_operation = "H";
    auto pMethod = PyUnicode_FromString("add_single_qubit_operation");
    auto pArgs0  = PyUnicode_FromString(quantum_operation.c_str());
    //  auto pArgs1  = PyUnicode_FromString(std::to_string(qubit).c_str());
    auto pArgs1 = PyLong_FromLong(qubit);

    auto pValue = PyObject_CallMethodObjArgs(interface, pMethod, pArgs0, pArgs1, NULL);
    post_py_process(pValue, pMethod,
                    std::string("Failed to call add_single_qubit_operation for the operation ") +
                      quantum_operation + ". Simulation aborts.");
}

void If_QIcircuit::apply_two_qubit_gate(std::string quantum_operation, unsigned int qubit0,
                                        unsigned int qubit1) {

    auto logger = get_logger_or_exit("qsim_logger");

    // quantum_operation = "CZ";

    auto pMethod = PyUnicode_FromString("add_two_qubit_operation");
    auto pArgs0  = PyUnicode_FromString(quantum_operation.c_str());
    auto pArgs1  = PyLong_FromLong(qubit0);
    auto pArgs2  = PyLong_FromLong(qubit1);
    // auto pArgs1  = PyUnicode_FromString(std::to_string(qubit0).c_str());
    // auto pArgs2  = PyUnicode_FromString(std::to_string(qubit1).c_str());
    auto pValue = PyObject_CallMethodObjArgs(interface, pMethod, pArgs0, pArgs1, pArgs2, NULL);

    Py_DECREF(pArgs0);
    Py_DECREF(pArgs1);
    Py_DECREF(pArgs2);
    post_py_process(pValue, pMethod, "Failed to call add_two_qubit_operation. Simulation aborts.");
}

void If_QIcircuit::measure_qubit(unsigned int qubit) {
    auto logger = get_logger_or_exit("qsim_logger");
    logger->debug("measure on qubit {}", qubit);

    auto pMethod = PyUnicode_FromString("add_measurement");
    auto pArgs   = PyLong_FromLong(qubit);
    logger->debug("calling add_measurement parameter pArgs: {}", PyLong_AsLong(pArgs));
    auto pValue = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    logger->debug("finish call python function add_measurement");
    post_py_process(pValue, pMethod, "Failed to call add_measurement. Simulation aborts.");
}

unsigned int If_QIcircuit::run_circuit(unsigned int num_measure) {

    json         msmt_res;
    auto         logger = get_logger_or_exit("qsim_logger");
    unsigned int measurement_result;
    // Fetch the measurement result which is generated in interface
    auto pMethod = PyUnicode_FromString("return_measurement_result");
    logger->debug("calling return_measurement_result");
    auto pValue = PyObject_CallMethodObjArgs(interface, pMethod, NULL);

    if (pValue != NULL) {

        char* result;

        PyArg_Parse(pValue, "s", &result);

        try {
            json j_result = json::parse(result);
            read_json_object(j_result, msmt_res, "values");
        } catch (std::exception& e) {
            logger->error("{}: {}. Simulation Aborts", this->name(), e.what());
            exit(EXIT_FAILURE);
        }

        if (msmt_res.size() != num_measure) {
            logger->error(
              "{}: Measurement result size doesn't match input measurement qubit number. "
              "Simulation Aborts",
              this->name());
            exit(EXIT_FAILURE);
        }

        measurement_result = msmt_res[num_measure - 1];
    }

    post_py_process(pValue, pMethod,
                    "Failed to call return_measurement_result. Simulation aborts.");

    return measurement_result;
}
}  // namespace cactus
