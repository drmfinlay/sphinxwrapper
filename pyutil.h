/*
 * pyutil.h
 *
 *  Created on 22 Sept. 2017
 *      Author: Dane Finlay
 */

#ifndef PYUTIL_H_
#define PYUTIL_H_

#include <stdbool.h>
#include <Python.h>

/* Checks the argument count of a callable Python object and returns true
 * if it matches arg_count, or returns false if it doesn't and sets a 
 * Python exception.
 */
bool
assert_callable_arg_count(PyObject *value, const unsigned int arg_count);

#endif /* PYUTIL_H_ */
