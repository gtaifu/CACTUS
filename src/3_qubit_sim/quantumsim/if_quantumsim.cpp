#include "if_quantumsim.h"

#include <ostream>
#include <sstream>
#include <string>

#include "global_counter.h"
#include "logger_wrapper.h"
#include "num_util.h"

namespace cactus {

using std::pair;
using std::stringstream;
using std::vector;

If_QuantumSim::If_QuantumSim(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("qsim_logger");

    logger->trace("The logger for the If_QuantumSim has been initialized.");

    config();

    init_python_api();

    SC_CTHREAD(log_telf, clock_50MHz.pos());
    SC_CTHREAD(apply_quantum_operation, clock_50MHz.pos());
}

void If_QuantumSim::config() {

    Global_config& global_config = Global_config::get_instance();

    num_qubits              = global_config.num_qubits;
    num_msmt_devices        = global_config.num_msmt_devices;
    qubits_in_each_feedline = global_config.qubits_in_each_feedline;
    operation_time          = global_config.single_qubit_gate_time;
    cycle_time              = global_config.cycle_time;
    m_instruction_type      = global_config.instruction_type;
    mock_msmt_res_fn        = global_config.mock_msmt_res_fn;

    operation_time.insert(global_config.two_qubit_gate_time.begin(),
                          global_config.two_qubit_gate_time.end());
}

void If_QuantumSim::add_telf_header() {}

void If_QuantumSim::log_telf() {
    while (true) {

        wait();

        if (is_telf_on) {
            add_telf_line();
        }
    }
}

void If_QuantumSim::add_telf_line() {

    auto         counter_50MHz = global_counter::get("cycle_counter_50MHz");
    stringstream ss;
    bool         msmt_result_valid = false;

    // TODO: add telf for measurement results

    // ss << std::setfill(' ') << std::setw(11) << counter_50MHz->get_cur_cycle_num() << ",";

    // for (size_t i = 0; i < vec_msmt_result.size(); i++) {

    //     if (vec_msmt_result[i].read() != 0) {
    //         msmt_result_valid = true;
    //     }

    //     ss << std::setfill(' ') << std::setw(14) << vec_msmt_result[i].read() << ", ";
    // }
    // ss << std::endl;

    // if (msmt_result_valid == true) {
    //     telf_os << ss.str();
    // }
}

void If_QuantumSim::init_python_api() {

    auto logger = get_logger_or_exit("qsim_logger");

    // Create the interface_quantumsim instance and initialize the sparse density matrix with
    // the number of qubits
    logger->trace("Start initializing the interface to QuantumSim.");

    // Initialize the Python interpreter.
    try {
        Py_Initialize();
    } catch (...) {
        PyErr_Print();
        logger->error("Failed to initialized Python interpreter. Please try to fix it. Aborts.");
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
    auto pName = PyUnicode_DecodeFSDefault("interface");
    if (pName == NULL) {
        PyErr_Print();
        logger->error("Failed to decode the string 'interface'.");
    }

    // Import the file as a Python module.
    auto pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
        PyErr_Print();
        logger->error("Failed to import the Python module 'interface'. Aborts!");
        exit(EXIT_FAILURE);
    } else {
        logger->trace("Successfully imported the Python module 'interface'.");
    }

    // Create a dictionary for the contents of the module.
    auto pDict = PyModule_GetDict(pModule);

    // Create an instance of the Python class interface_quantumsim
    auto pClass = PyDict_GetItemString(pDict, "interface_quantumsim");

    if (pClass == NULL) {
        PyErr_Print();
        logger->error("The class interface_quantumsim is undefined in the module. Aborts.");
        exit(EXIT_FAILURE);
    } else {
        logger->trace(
          "Successfully created an instance of the Python class "
          "'interface_quantumsim'.");
    }

    if (PyCallable_Check(pClass)) {
        interface = PyObject_CallObject(pClass, NULL);
        if (interface == NULL) {
            logger->error(
              "Failed to call the python object to create the interface "
              "to the qubit state simulator. Aborts.");
            exit(EXIT_FAILURE);
        } else {
            logger->trace("Successfully created the callable object 'interface'.");
        }
    } else {
        logger->error("Given object is not callable. Aborts.");
        exit(EXIT_FAILURE);
    }

    // Initialize the spare density matrix with the number of qubits
    auto pMethod = PyUnicode_FromString("init_dm");
    // the parameter is the number of qubits
    auto pArgs  = PyLong_FromLong(num_qubits);
    auto pValue = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    Py_DECREF(pArgs);

    if (pValue != NULL) {

        logger->trace("Successfully initialized the density matrix in QuantumSim for {} qubits.",
                      num_qubits);
        Py_DECREF(pValue);
        Py_XDECREF(pMethod);
        Py_DECREF(pModule);

    } else {

        Py_DECREF(pMethod);
        Py_DECREF(pModule);
        PyErr_Print();
        logger->error(
          "Failed to initialize the density matrix in QuantumSim (function init_dm). "
          "Aborts.");
        exit(EXIT_FAILURE);
    }

    // Print out the classical state of this density matrix
    logger->trace("Printing the classical state of the qubits ...");
    pMethod = PyUnicode_FromString("print_classical_state");
    pValue  = PyObject_CallMethodObjArgs(interface, pMethod, NULL);

    if (pValue != NULL) {

        Py_DECREF(pValue);
        Py_DECREF(pMethod);

    } else {

        Py_DECREF(pMethod);
        PyErr_Print();
        logger->error("Failed to print the classical state of qubits after initialization.");
    }
}

// This function sends quantum operations received from ADI to QuantumSim.
// It works in the following way:
// In each cycle, check whether there is any quantum operation.
//   - If not, wait until the next cycle
//   - If yes, perform the "idle and apply" procedure.
//
//   The "idle and apply" procedure first apply idling gates on the target qubit(s),
// and then apply this operation on the target qubit(s). Inserting idling gates is
// required by QuantumSim to handle timing, which is used to simulate qubit decoherence.
// For more reasoning, please refer to Mengyu Zhang's master thesis.
//
// starting point
//        v               _____                  _____                       _____
//        |              |__H__|                |__X__|                     |__Y__|
//        |<---- idle0 ---->|<------ idle1 ------->|<--------- idle2 --------->|
//
//
// The "idle and apply" procedure  consists of the following steps:
// 1. Calculate the duration of the idling gate
//    - In general, the duration is the interval between the middle points of two gates on this
//    qubit
//    - See the figure above for illustration.
// 2. Apply the idling gate on this qubit
// 3. Apply this gate on this qubit
//
// It is important to clarify some terms.
// - When a cycle number is used to indicate a timing point, it corresponds to the starting
//    point of this cycle.
// - We say an operation starts at cycle k, it means the starting point of this operation is
//    at the starting point of cycle k.
// - starting_cycle corresponds to the cycle when the first operation of the entire circuit starts.
// - current_cycle  corresponds to the cycle of the current operation being processed.
void If_QuantumSim::apply_quantum_operation() {

    auto         logger = get_logger_or_exit("qsim_logger");
    stringstream ss;

    std::string          op_name;
    std::string          op_name_prefix;
    bool                 is_1st_op = true;
    vector<unsigned int> last_gate_durations(num_qubits, 0);
    vector<int>          pre_gate_start_point(num_qubits, 0);
    unsigned int         current_cycle = 0;
    unsigned int         idle_duration = 0, cur_gate_duration = 0, pre_gate_duration = 0;

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

        // ------------------------------------------------------------
        // If there comes at lease one quantum operation
        // ------------------------------------------------------------
        op_name        = "Null";
        op_name_prefix = "Null";
        res_from_qsim.reset();

        // If this is the first operation of entire circuit, record this clock cycle
        if (is_1st_op) {
            starting_cycle = moment.cycle;
        }

        current_cycle = moment.cycle;

        ss.str("");
        ss << "The following operations arrive at cycle: " << current_cycle << std::endl;
        for (size_t op_idx = 0; op_idx < moment.atom_ops.size(); op_idx++) {
            ss << moment.atom_ops[op_idx];
        }
        logger->debug("{}", ss.str());

        moment.trim_qnops();

        target_qubits.clear();

        // iterate over all individual operations
        for (auto it_op = moment.atom_ops.begin(); it_op != moment.atom_ops.end(); it_op++) {
            op_name       = it_op->operation;
            target_qubits = it_op->target_qubits;

            size_t number_target_qubits = target_qubits.size();

            if (number_target_qubits > 2) {

                logger->error(
                  "Currently support at most two-qubit operations. But found operation {} "
                  "operates on {} qubits. Aborts!",
                  op_name, target_qubits.size());

                exit(EXIT_FAILURE);
            }

            // if instruction type is asm,op_name_prefix is used
            if (m_instruction_type == Instruction_type::ASM && op_name.find_first_of("rxyz") == 0) {
                if (op_name.size() > 2 && ((op_name[1] == 'm') || (op_name[0] == 'r'))) {
                    op_name_prefix = op_name.substr(0, 2);
                } else {
                    op_name_prefix = op_name.substr(0, 1);
                }
            } else {
                op_name_prefix = op_name;
            }

            // Get the duration of the current gate
            auto it = operation_time.find(op_name_prefix);

            if (it != operation_time.end())

                cur_gate_duration = it->second;

            else {
                logger->error("If_QuantumSim: found undefined operation ({}). Aborts!", op_name);
                exit(EXIT_FAILURE);
            }

            // ============================== idle ==============================
            for (size_t i = 0; i < number_target_qubits; i++) {

                unsigned int qubit = target_qubits[i];

                // Get the duration of the previous gate
                pre_gate_duration = last_gate_durations[qubit];

                idle_duration = get_idle_duration(is_1st_op, cur_gate_duration, current_cycle,
                                                  pre_gate_start_point[qubit], pre_gate_duration);

                logger->debug(
                  "is_1st_op: {}, cur_gate_duration: {}, current_cycle: {}, "
                  "pre_gate_start_point[{}]: {}, pre_gate_duration: {}",
                  is_1st_op, cur_gate_duration, current_cycle, qubit, pre_gate_start_point[qubit],
                  pre_gate_duration);

                logger->debug("The duration of this idling gate is {} ns.", idle_duration);

                // Apply an idling gate before the quantum operation
                if (idle_duration > 0) {  // the idle_duration will be 0 for the first operaiton.
                    apply_idle_gate(idle_duration, static_cast<unsigned int>(qubit));
                }
            }

            // ============================== apply ==============================
            if (number_target_qubits == 1) {
                unsigned int qubit = target_qubits[0];

                // If a measurement
                if (op_name.compare("measure") == 0) {

                    unsigned int result = measure_qubit(qubit);
                    res_from_qsim.results.push_back(std::make_pair(qubit, result));

                } else if (op_name.compare("mock_meas") == 0) {
                    // if a mock measurement, only execute once
                    if (qubit == 0) {
                        mock_measure(mock_msmt_res_fn);
                    }

                } else {  // If not a measurement operation
                    apply_single_qubit_gate(op_name, qubit);

                    // Print out the single_ptms_to_do for this qubit
                    print_ptms_to_do(qubit);
                }

                // After applying this quantum operation, it is recorded as the previous operation
                pre_gate_start_point[qubit] = current_cycle;
                last_gate_durations[qubit]  = cur_gate_duration;
            }

            if (number_target_qubits == 2) {
                unsigned int qubit0 = target_qubits[0];
                unsigned int qubit1 = target_qubits[1];

                apply_two_qubit_gate(std::string(""), qubit0, qubit1);

                pre_gate_start_point[qubit0] = current_cycle;
                pre_gate_start_point[qubit1] = current_cycle;
                last_gate_durations[qubit0]  = cur_gate_duration;
                last_gate_durations[qubit1]  = cur_gate_duration;
            }
        }

        // After the quantum operations are applied, we can print out the full density matrix.
        print_full_dm();

        msmt_res.write(res_from_qsim);

        // Since there is already operations happened, it is no longer the first operation
        is_1st_op = false;
    }
}

unsigned int If_QuantumSim::get_idle_duration(bool is_1st_op, unsigned int cur_gate_duration,
                                              unsigned int current_cycle,
                                              unsigned int pre_gate_start_point,
                                              unsigned int pre_gate_duration) {
    unsigned int idle_duration = 0;

    if (is_1st_op == true) {  // If this is the first operation of the entire circuit

        // the idling time is half gate time
        idle_duration = cur_gate_duration / 2;

    } else {

        if (pre_gate_duration == 0) {  // no previous gates applied on this qubit

            // the interval between the starting point of the entire circuit
            // and the middle point of the current operation
            idle_duration = (current_cycle - starting_cycle) * cycle_time + (cur_gate_duration / 2);

        } else {  // there was an previous operation applied on this qubit

            // the interval between the middle points of the curret gate and the previous gate
            int gate_duration_interval = cur_gate_duration / 2 - pre_gate_duration / 2;
            idle_duration =
              (current_cycle - pre_gate_start_point) * cycle_time + gate_duration_interval;
        }
    }
    return idle_duration;
}

void If_QuantumSim::post_py_process(PyObject* pValue, PyObject* pMethod,
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

void If_QuantumSim::apply_idle_gate(unsigned int idle_duration, unsigned int qubit) {

    auto logger = get_logger_or_exit("qsim_logger");

    logger->debug("An idling gate of {}ns is applied on qubit {}.", idle_duration, qubit);

    // calculate_gamma_lamda
    auto pArgs   = PyLong_FromLong(idle_duration);
    auto pMethod = PyUnicode_FromString("calculate_gamma_lamda");
    auto pValue  = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    Py_DECREF(pArgs);
    post_py_process(pValue, pMethod, "Failed to call calculate_gamma_lamda. Aborts.");

    // Prepare the idling gate ptm
    pMethod = PyUnicode_FromString("prepare_idling_ptm");
    pValue  = PyObject_CallMethodObjArgs(interface, pMethod, NULL);
    post_py_process(pValue, pMethod, "Failed to call prepare_idling_ptm. Aborts.");

    // Apply this idling gate
    pMethod = PyUnicode_FromString("apply_ptm");
    pArgs   = PyUnicode_FromString(std::to_string(qubit).c_str());
    pValue  = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    post_py_process(pValue, pMethod, "Failed to call apply_ptm. Aborts.");
}

void If_QuantumSim::apply_single_qubit_gate(std::string quantum_operation, unsigned int qubit) {

    auto logger = get_logger_or_exit("qsim_logger");

    logger->debug("To apply single-qubit-gate {} on qubit {}.", quantum_operation, qubit);

    if (quantum_operation.compare("Null") == 0) {  // skip the quantum nop.
        return;
    }

    // Prepare the ptm for the quantum gate
    auto pMethod = PyUnicode_FromString("prepare_ptm");
    auto pArgs   = PyUnicode_FromString(quantum_operation.c_str());
    auto pValue  = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    post_py_process(pValue, pMethod,
                    std::string("Failed to call prepare_ptm for the operation ") +
                      quantum_operation + ". Aborts.");

    // Apply the ptm to density matrix
    pMethod = PyUnicode_FromString("apply_ptm");
    pArgs   = PyUnicode_FromString(std::to_string(qubit).c_str());
    pValue  = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    post_py_process(
      pValue, pMethod,
      std::string("Failed to call apply_ptm for the operation ") + quantum_operation + ". Aborts.");
}

unsigned int If_QuantumSim::measure_qubit(unsigned int qubit) {

    auto logger = get_logger_or_exit("qsim_logger");
    logger->debug("To measure the qubit {}.", qubit);

    unsigned int single_msmt_result = 0;

    auto pMethod = PyUnicode_FromString("apply_measurement");
    auto pArgs   = PyUnicode_FromString(std::to_string(qubit).c_str());
    auto pValue  = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    post_py_process(pValue, pMethod, "Failed to call apply_measurement. Aborts.");

    // Fetch the measurement result which is generated in interface
    pMethod = PyUnicode_FromString("return_measurement_result");
    pValue  = PyObject_CallMethodObjArgs(interface, pMethod, NULL);

    if (pValue != NULL) {
        single_msmt_result = PyLong_AsLong(pValue);
    }

    post_py_process(pValue, pMethod, "Failed to call return_measurement_result. Aborts.");

    return single_msmt_result;
}

void If_QuantumSim::mock_measure(std::string mock_msmt_res_fn) {

    auto logger = get_logger_or_exit("qsim_logger");
    logger->debug("To apply a mock measurement");

    auto pMethod = PyUnicode_FromString("apply_mock_meas");
    auto pArgs   = PyUnicode_FromString(mock_msmt_res_fn.c_str());
    auto pValue  = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    post_py_process(pValue, pMethod, "Failed to call apply_mock_meas. Aborts.");
}

// Currently, it only supports the CPhase gate and the parameter quantum_operation is ignored.
// TODO: More two-qubit gates should be supported.
void If_QuantumSim::apply_two_qubit_gate(std::string quantum_operation, unsigned int qubit0,
                                         unsigned int qubit1) {

    auto logger = get_logger_or_exit("qsim_logger");

    logger->debug("To apply two-qubit-gate {} on qubit {} and {}.", quantum_operation, qubit0,
                  qubit1);

    // Prepare the active two-qubit ptm
    auto pMethod = PyUnicode_FromString("prepare_two_ptm");
    auto pValue  = PyObject_CallMethodObjArgs(interface, pMethod, NULL);
    post_py_process(pValue, pMethod, "Failed to call prepare_two_ptm. Aborts.");

    // Apply the two_qubit ptm
    pMethod     = PyUnicode_FromString("apply_two_ptm");
    auto pArgs0 = PyUnicode_FromString(std::to_string(qubit0).c_str());
    auto pArgs1 = PyUnicode_FromString(std::to_string(qubit1).c_str());
    pValue      = PyObject_CallMethodObjArgs(interface, pMethod, pArgs0, pArgs1, NULL);

    Py_DECREF(pArgs0);
    Py_DECREF(pArgs1);
    post_py_process(pValue, pMethod, "Failed to call apply_two_ptm. Aborts.");
}

void If_QuantumSim::print_ptms_to_do(unsigned int qubit) {

    auto logger = get_logger_or_exit("qsim_logger");

    logger->debug("To print the appending Pauli Transfer Matrix (PTM) ...");
    auto pMethod = PyUnicode_FromString("print_ptm_to_do");
    auto pArgs   = PyUnicode_FromString(std::to_string(qubit).c_str());
    auto pValue  = PyObject_CallMethodObjArgs(interface, pMethod, pArgs, NULL);
    post_py_process(pValue, pMethod, "Failed to call print_ptm_to_do. Aborts.");
}

void If_QuantumSim::print_full_dm() {

    auto logger = get_logger_or_exit("qsim_logger");

    logger->debug("Printing the full density matrix of the qubits ...");
    auto pMethod = PyUnicode_FromString("print_full_dm");
    auto pValue  = PyObject_CallMethodObjArgs(interface, pMethod, NULL);
    post_py_process(pValue, pMethod, "Failed to call print_full_dm. Aborts.");
}

}  // namespace cactus
