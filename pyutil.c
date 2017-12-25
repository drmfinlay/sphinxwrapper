/*
 * pyutil.c
 *
 *  Created on 22 Sept. 2017
 *      Author: Dane Finlay
 *
 * ==============================================================================
 * MIT License
 *
 * Copyright (c) 2017 Dane Finlay
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ==============================================================================
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
    PyObject *fc = PyObject_GetAttrString(value, "func_code");
    if(fc) {
        PyObject *ac = PyObject_GetAttrString(fc, "co_argcount");
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

