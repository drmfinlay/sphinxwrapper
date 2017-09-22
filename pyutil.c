/*
 * pyutil.c
 *
 *  Created on 22 Sept. 2017
 *      Author: Dane Finlay
 */

#include "pyutil.h"

bool
assert_callable_arg_count(PyObject *value, const unsigned int arg_count) {
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
	PyErr_Format(PyExc_TypeError, "callable must have %d %s.",
		     arg_count, arg_or_args);
	result = false;
    }
    
    return result;
}

