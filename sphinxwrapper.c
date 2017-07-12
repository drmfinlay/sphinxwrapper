/*
 * sphinxwrapper.c
 *
 *  Created on 8 Jul. 2017
 *      Author: Dane Finlay
 */

#include "sphinxwrapper.h"

static const arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    /* Argument file. */
    {"-argfile",
     ARG_STRING,
     NULL,
     "Argument file giving extra arguments."},
    {"-adcdev",
     ARG_STRING,
     NULL,
     "Name of audio device to use for input."},
    {"-time",
     ARG_BOOLEAN,
     "no",
     "Print word times in file transcription."},
    CMDLN_EMPTY_OPTION
};

static PyObject *PocketSphinxError;
static PyObject *CallbackNotSetError;
static PyObject *hypothesis_callback;
static PyObject *test_callback;

static ps_decoder_t *
create_ps_decoder_c(int argc, char *argv[]) {
    char const *cfg;
    ps_decoder_t *ps;
    cmd_ln_t *config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);
    
    /* Handle argument file as -argfile. */
    if (config && (cfg = cmd_ln_str_r(config, "-argfile")) != NULL) {
        config = cmd_ln_parse_file_r(config, cont_args_def, cfg, FALSE);
    }
    
    if (config == NULL) {
        E_INFO("Specify '-adcdev <device identifier>' to recognize from microphone using the specified device.\n");
        cmd_ln_free_r(config);
        return NULL;
    }
    
    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        E_ERROR("PocketSphinx couldn't be initialised. Is your configuration right?");
        return NULL;
    }
    
    return ps;
}

static PyObject *
create_ps_decoder(PyObject *self, PyObject *args) {
    PyObject *result = NULL;
    PyObject *list;
    Py_ssize_t list_size;
    
    if (PyArg_ParseTuple(args, "O:create_ps_decoder", &list)) {
        if (!PyList_Check(list)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be a list");
	    return NULL;
	}
	Py_XINCREF(list);
	list_size = PyList_Size(list);
	char *strings[list_size];
	for (Py_ssize_t i = 0; i < list_size; i++) {
	    PyObject * item = PyList_GetItem(list, i);
	    // Is this paranoid?
	    Py_XINCREF(item);
	    if (!PyString_Check(item)) {
		// Raise the exception flag and return NULL
		PyErr_SetString(PyExc_TypeError, "all list items must be strings!");
		Py_XDECREF(item);
		Py_XDECREF(list);
		
		return NULL;
	    }
	    char *str = PyString_AsString(item);
	    printf("%s\n", str);
	    strings[i] = str;
	    Py_XDECREF(item);
	}
	
	result = list;
    }
    return result;
}


static PyObject *
set_callback_func(PyObject **callback_pptr, PyObject *args) {
    PyObject *result = NULL;
    PyObject *temp;
    
    // PyArg_ParseTuple will put the result into an obj referenced by temp
    // or return NULL, which is false.
    if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
	
        Py_XINCREF(temp);           /* Add a reference to new callback */
        Py_XDECREF(&(*callback_pptr));  /* Dispose of previous callback */
        *callback_pptr = temp;       /* Remember new callback using callback_ptr */
	
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    
    return result;
}

static PyObject *
call_callback(PyObject *callback_ptr, PyObject *args) {
    int arg;
    PyObject *arglist;
    PyObject *result = NULL;
    arg = 123;

    if (callback_ptr != NULL) {
	/* Time to call the callback */
	arglist = Py_BuildValue("(i)", arg);
	result = PyObject_CallObject(callback_ptr, arglist);
	Py_DECREF(arglist);
    } else {
	// Set a Python exception here so Python knows what's
	// happened
	PyErr_SetString(CallbackNotSetError, "No callback set");
    }
    
    // Don't worry about result being NULL because PyObject_CallObject
    // should only return NULL if the callback function raised an exception
    // somehow. We DO NOT need to call PyErr_SetString because the Python
    // callback function has already done that.
    return result;
}


static PyObject *
set_hypothesis_callback(PyObject *self, PyObject *args) {
    return set_callback_func(&hypothesis_callback, args);
}

static PyObject *
set_test_callback(PyObject *self, PyObject *args) {
    return set_callback_func(&test_callback, args);
}

static PyObject *
call_hypothesis_callback(PyObject *self, PyObject *args) {
    return call_callback(hypothesis_callback, args);
}

static PyObject *
call_test_callback(PyObject *self, PyObject *args) {
    return call_callback(test_callback, args);
}

static PyMethodDef sphinxwrapper_funcs[] = {
    {"create_ps_decoder",  create_ps_decoder, METH_VARARGS,
     "Create a new ps decoder using the arguments passed."},
    {"set_test_callback", set_test_callback, METH_VARARGS,
     "Set a Python callback method for C to call."},
    {"set_hypothesis_callback", set_hypothesis_callback, METH_VARARGS,
     "Set a Python callback method for C to call."},
    {"call_hypothesis_callback", call_hypothesis_callback, METH_VARARGS,
     "Call the set Python callback function if it's set."},
    {"call_test_callback", call_test_callback, METH_VARARGS,
     "Call the set Python callback function if it's set."},
    {NULL, NULL, 0, NULL} // Sentinel so the C to Python API knows when there aren't any more funcs
};

PyMODINIT_FUNC
initsphinxwrapper() {
    PyObject *module = Py_InitModule("sphinxwrapper", sphinxwrapper_funcs);

    // Define a new Python exception
    PocketSphinxError = PyErr_NewException("PocketSphinx.Error", NULL, NULL);
    Py_INCREF(PocketSphinxError);
    PyModule_AddObject(module, "error", PocketSphinxError);

    // Define another one for calling callbacks that aren't set
    CallbackNotSetError = PyErr_NewException("Callback.Error", NULL, NULL);
    Py_INCREF(CallbackNotSetError);
    PyModule_AddObject(module, "callbackerror", CallbackNotSetError);
}

int
main(int argc, char *argv[]) {
    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(argv[0]);

    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();

    /* Add a static module */
    initsphinxwrapper();
}

