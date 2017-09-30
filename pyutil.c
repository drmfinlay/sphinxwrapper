/*
 * pyutil.c
 *
 *  Created on 22 Sept. 2017
 *      Author: Dane Finlay
 */

#include "pyutil.h"

bool
assert_callable_arg_count(PyObject *value, const unsigned int arg_count) {
#if PY_MAJOR_VERSION >= 3
    // This doesn't work in Python 3+. See header.
    PyErr_Format(PyExc_NotImplementedError,
		 "internal C function '%s' is not yet "
		 "implemented for this version of Python.",
		 __func__);
    return false;
#else
    int count = -1;
    bool result = true;
    PyObject* fc = PyObject_GetAttrString(value, "func_code");
    if(fc) {
	PyObject* ac = PyObject_GetAttrString(fc, "co_argcount");
	if(ac) {
	    count = PyInt_AsLong(ac);
	    Py_DECREF(ac);
	}
	Py_DECREF(fc);
    }

    const char *arg_or_args;
    if (arg_count == 1) arg_or_args = "argument";
    else arg_or_args = "arguments";

    if (count != arg_count) {
	PyErr_Format(PyExc_TypeError, "callable must have %d %s, found %d %s",
		     arg_count, arg_or_args, count, arg_or_args);
	result = false;
    }
    
    return result;
#endif
}

