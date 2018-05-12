/*
 * PythonCompat.h
 *
 * Header with definitions for writing C/C++ extension modules compatible with
 * Python 2.x and Python 3.x.
 *
 *  Created on 12 May. 2018
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

#ifndef PYTHONCOMPAT_H_
#define PYTHONCOMPAT_H_

#include <Python.h>

#if PY_MAJOR_VERSION >= 3
#define IS_PY3
#else
#define IS_PY2
#endif

// Definitions for Python 2.x
#ifdef IS_PY2

// Checks
#define PYCOMPAT_STRING_CHECK PyString_Check
#define PYCOMPAT_INT_CHECK PyInt_Check

// Conversions
#define PYCOMPAT_STRING_AS_STRING PyString_AsString
#define PYCOMPAT_INT_AS_LONG PyInt_AsLong

// Modules
#define PYCOMPAT_INIT_ERROR return

// Definitons for Python 3.x and above
#else

// Checks
#define PYCOMPAT_STRING_CHECK PyUnicode_Check
#define PYCOMPAT_INT_CHECK PyLong_Check

// Conversions
#define PYCOMPAT_STRING_AS_STRING PyUnicode_AsUTF8
#define PYCOMPAT_INT_AS_LONG PyLong_AsLong

// Modules
#define PYCOMPAT_INIT_ERROR return NULL
#endif

// Define the return type of module init functions if necessary
#ifndef PyMODINIT_FUNC /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif


#endif /* PYTHONCOMPAT_H_ */
