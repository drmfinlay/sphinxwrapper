/*
 * sphinxwrapper.h
 *
 *  Created on: 7 Jul. 2017
 *      Author: Dane Finlay
 */

#ifndef SPHINXWRAPPER_H_
#define SPHINXWRAPPER_H_

/*
 * Include Python API so we can give Python objects which represent pocket sphinx's structs
 * 
 */
#include <python2.7/Python.h>
#include <stdio.h>

#include <sphinxbase/err.h>
#include <sphinxbase/cmd_ln.h>
#include <pocketsphinx.h>

// Wrapper to parse passed arguments and initialise a pocket sphinx decoder.
static ps_decoder_t *
create_ps_decoder_c(int argc, char *argv[]);

static PyObject *
create_ps_decoder(PyObject *self, PyObject *args);


// High-level interface funcs to pocket sphinx based on dragonfly natlink
// engine funcs.

// This function can do stuff with the "sphinxbase/ad.h" functions to direct
static PyObject *
recognize_from_mic(PyObject *self, PyObject *args);

// This shouldn't be exposed to Python
// It is a generic function for setting multiple callbacks.
static PyObject *
set_callback_func(PyObject **callback_pptr, PyObject *args);

// General C function to call a Python callback function
static PyObject *
call_callback(PyObject *callback_ptr, PyObject *args);

static PyObject *
set_hypothesis_callback(PyObject *self, PyObject *args);

static PyObject *
set_recognition_results_callback(PyObject *self, PyObject *args);

// JSGF or FSG? Or both?
static PyObject *
create_grammar_obj(PyObject *self, PyObject *args);

static PyObject *
start_decoders(PyObject *self, PyObject *args);

// Required Python module init function
PyMODINIT_FUNC
initsphinxwrapper();

#endif /* SPHINXWRAPPER_H_ */

