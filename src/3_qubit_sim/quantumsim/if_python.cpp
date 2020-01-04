#include "if_python.h"

#include <iostream>

namespace cactus {
void apply_all_pending(PyObject* interface) {
    PyObject *pValue, *pMethod;

    if (interface != NULL) {
        pMethod = PyUnicode_FromString("apply_all_pending");

        // Call the method
        pValue = PyObject_CallMethodObjArgs(interface, pMethod, NULL);
        if (pValue != NULL) {
            std::cout << "All the pending gates are applied to corresponding qubits" << std::endl;
            Py_DECREF(pValue);
            Py_XDECREF(pMethod);
        } else {
            Py_DECREF(pMethod);
            PyErr_Print();
            std::cout << "Call apply_all_pending function failed" << std::endl;
        }
    } else {
        PyErr_Print();
        std::cout << "There is not density matrix instance" << std::endl;
    }
}  // end of config_reader::apply_all_pending()

void print_final_result(PyObject* interface) {
    PyObject *pValue, *pMethod;

    if (interface != NULL) {
        pMethod = PyUnicode_FromString("print_final_result");

        // Call the method
        pValue = PyObject_CallMethodObjArgs(interface, pMethod, NULL);
        if (pValue != NULL) {
            std::cout << "All the pending gates are applied to corresponding qubits" << std::endl;
            Py_DECREF(pValue);
            Py_XDECREF(pMethod);
        } else {
            Py_DECREF(pMethod);
            PyErr_Print();
            std::cout << "Call print_final_result function failed" << std::endl;
        }
    } else {
        PyErr_Print();
        std::cout << "There is not density matrix instance" << std::endl;
    }

    if (interface != NULL) {
        pMethod = PyUnicode_FromString("record_msmt_results");

        // Call the method
        pValue = PyObject_CallMethodObjArgs(interface, pMethod, NULL);
        if (pValue != NULL) {
            std::cout << "All the pending gates are applied to corresponding qubits" << std::endl;
            Py_DECREF(pValue);
            Py_XDECREF(pMethod);
        } else {
            Py_DECREF(pMethod);
            PyErr_Print();
            std::cout << "Call record_msmt_results function failed" << std::endl;
        }
    } else {
        PyErr_Print();
        std::cout << "There is not density matrix instance" << std::endl;
    }

}  // end of config_reader::print_final_result()

}  // end of namespace cactus
