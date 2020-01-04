#ifndef _IF_PYTHON_H_
#define _IF_PYTHON_H_

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

namespace cactus {

void apply_all_pending(PyObject* interface);

void print_final_result(PyObject* interface);

}  // end of namespace cactus

#endif  // _IF_PYTHON_H_
