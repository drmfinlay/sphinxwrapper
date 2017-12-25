/*
 * pyutil.h
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

#ifndef PYUTIL_H_
#define PYUTIL_H_

#include <stdbool.h>
#include <Python.h>

// Define some useful macros for compiling an extension module for Python 2.7 and 3+
#if PY_MAJOR_VERSION >= 3
#define INITERROR return NULL
#else
#define INITERROR return
#endif

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

/* Checks the argument count of a callable Python object and returns true
 * if it matches arg_count, or returns false if it doesn't and sets a 
 * Python exception.
 *
 * For Python 3 and above, this method returns false with a Python
 * NotImplementedError set; the attributes it depends on are not present in
 * Python 3+. It can be done differently for both Python 2 and 3 using Python's
 * built-in inspect module, however I'm not sure that module can be used from C.
 */
bool
assert_callable_arg_count(PyObject *value, const unsigned int arg_count);

#endif /* PYUTIL_H_ */
