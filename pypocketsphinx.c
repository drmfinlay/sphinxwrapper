/*
 * pypocketsphinx.c
 *
 *  Created on: 22 Sept. 2017
 *      Author: Dane Finlay
 */

#include "pypocketsphinx.h"

#define PS_DEFAULT_SEARCH "_default"

static PyObject *PocketSphinxError;

const arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    /* Argument file. */
    {"-argfile",
     ARG_STRING,
     NULL, 
     "Argument file giving extra arguments."},
    CMDLN_EMPTY_OPTION
};

PyObject *
PSObj_start_utterance(PSObj *self) {
    PyObject *result = NULL;
    ps_decoder_t * ps = get_ps_decoder_t(self);

    if (ps == NULL)
	return NULL;
    int r = ps_start_utt(ps);
    if (r == 0) {
	Py_INCREF(Py_None);
	result = Py_None;
    } else
        PyErr_Format(PocketSphinxError,	"Failed to start utterance. "
		     "Got '%d' from ps_start_utt.", r);
    return result;
}

PyObject *
PSObj_end_utterance(PSObj *self) {
    PyObject *result = NULL;
    ps_decoder_t * ps = get_ps_decoder_t(self);

    if (ps == NULL)
	return NULL;
    
    int r = ps_end_utt(ps);
    if (r == 0) {
	Py_INCREF(Py_None);
	result = Py_None;
    } else
        PyErr_Format(PocketSphinxError,	"Failed to end utterance. "
		     "Got '%d' from ps_end_utt.", r);
    return result;
}

PyObject *
PSObj_process_audio(PSObj *self, PyObject *audio_data) {
    ps_decoder_t * ps = get_ps_decoder_t(self);

    if (ps == NULL)
	return NULL;

    if (! PyObject_TypeCheck(audio_data, &AudioDataType)) {
	PyErr_SetString(PyExc_TypeError, "audio_data argument is not an AudioData object.");
	return NULL;
    }

    AudioDataObj *audio_data_c = (AudioDataObj *)audio_data;

    if (!audio_data_c->is_set) {
	PyErr_SetString(AudioDataError, "audio_data object is not set up properly. Try using "
			"PocketSphinx.read_audio()");
	return NULL;
    }

    uint8 in_speech;
    char const *hyp;
    ps_process_raw(ps, audio_data_c->audio_buffer, audio_data_c->n_samples, FALSE, FALSE);

    in_speech = ps_get_in_speech(ps);
    bool utt_started = self->utterance_started;

    if (in_speech && !utt_started) {
	self->utterance_started = true;
	
	// Call speech_start callback
	PyObject *callback = self->speech_start_callback;
	if (PyCallable_Check(callback)) {
	    PyObject_CallObject(callback, NULL); // no args required.
	}
    }

    if (!in_speech && utt_started) {
	/* speech -> silence transition, time to start new utterance  */
	ps_end_utt(ps);
	hyp = ps_get_hyp(ps, NULL);
	
	// Call the Python hypothesis callback if it is callable
	// It should have the correct number of arguments because
	// of the checks in set_hypothesis_callback
	PyObject *callback = self->hypothesis_callback;
	if (PyCallable_Check(callback)) {
	    PyObject *args;
	    if (hyp != NULL) {
		args = Py_BuildValue("(s)", hyp);
	    } else {
		Py_INCREF(Py_None);
		args = Py_BuildValue("(O)", Py_None);
	    }
	    
	    PyObject_CallObject(callback, args);
	}
	
	if (ps_start_utt(ps) < 0) {
	    PyErr_SetString(PocketSphinxError, "Failed to start "
			    "utterance.");
	    return NULL;
	}
	self->utterance_started = false;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PSObj_set_jsgf_search(PSObj *self, PyObject *args, PyObject *kwds) {
    const char *name = NULL;
    const char *jsgf_str = NULL;
    const char *jsgf_file = NULL;
    static char *kwlist[] = {"jsgf_str", "jsgf_file", "name", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|sss", kwlist, &jsgf_str, &jsgf_file, &name))
        return NULL;

    if (!name)
	name = PS_DEFAULT_SEARCH;

    if (!(jsgf_str || jsgf_file) || (jsgf_str && jsgf_file)) {
	PyErr_SetString(PocketSphinxError, "either the jsgf_str OR the jsgf_file argument must be "
			"used when calling this method");
	return NULL;
    }
    
    ps_decoder_t * ps = get_ps_decoder_t(self);
    if (ps == NULL)
	return NULL;

    const char *err_msg = "Something went wrong while setting the JSGF grammar. Please check for "
        "syntax or semantic errors.";
    
    // Set the value using ps_set_jsgf_string
    if (jsgf_str && (ps_set_jsgf_string(ps, name, jsgf_str) < 0 ||
		     ps_set_search(ps, PS_DEFAULT_SEARCH) < 0)) {
	PyErr_SetString(PocketSphinxError, err_msg);
	return NULL;
    }

    if (jsgf_file && (ps_set_jsgf_file(ps, name, jsgf_file) < 0 ||
		      ps_set_search(ps, name) < 0)) {
        PyErr_SetString(PocketSphinxError, err_msg);
        return NULL;
     }

    Py_XDECREF(self->search_name);
    self->search_name = Py_BuildValue("s", name);
    Py_INCREF(self->search_name);

    Py_INCREF(Py_None);
    return Py_None;
}

PyMethodDef PSObj_methods[] = {
    {"start_utterance",
     (PyCFunction)PSObj_start_utterance, METH_NOARGS,
     PyDoc_STR("Call this function before passing any utterance data.")},
    {"end_utterance",
     (PyCFunction)PSObj_end_utterance, METH_NOARGS,
     PyDoc_STR("Call this function to end utterance processing.")},
    {"process_audio",
     (PyCFunction)PSObj_process_audio, METH_O,  // takes self + one argument
     PyDoc_STR("Process audio from an AudioBuffer object.")},
    {"set_jsgf_search",
     (PyCFunction)PSObj_set_jsgf_search, METH_KEYWORDS,
     PyDoc_STR("Set the JSpeech Grammar Format grammar string or file path and "
	       "optionally the name to use for the Pocket Sphinx search.\n "
	       "Setting an old search name will replace that search.")},
    {NULL}  /* Sentinel */
};

PyObject *
PSObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PSObj *self;

    self = (PSObj *)type->tp_alloc(type, 0);
    if (self != NULL) {
	// Set the callbacks to None initially
	// TODO Set to a lambda like 'lambda x : None' instead?
        Py_INCREF(Py_None);
        self->speech_start_callback = Py_None;
        Py_INCREF(Py_None);
        self->hypothesis_callback = Py_None;

        Py_INCREF(Py_None);
        self->search_name = Py_None;

	// Ensure pointer members are NULL
        self->ps = NULL;
        self->config = NULL;

	self->utterance_started = false;
    }

    return (PyObject *)self;
}

void
PSObj_dealloc(PSObj* self) {
    Py_XDECREF(self->hypothesis_callback);
    Py_XDECREF(self->speech_start_callback);
    Py_XDECREF(self->search_name);
    
    // Deallocate the config object
    cmd_ln_t *config = self->config;
    if (config != NULL)
	cmd_ln_free_r(config);

    // Deallocate the Pocket Sphinx decoder
    ps_decoder_t *ps = self->ps;
    if (ps != NULL)
	ps_free(ps);

    // Finally free the PSObj itself
    Py_TYPE(self)->tp_free((PyObject*)self);
}

ps_decoder_t *
get_ps_decoder_t(PSObj *self) {
    ps_decoder_t *ps = self->ps;
    if (ps == NULL)
	PyErr_SetString(PyExc_ValueError, "PocketSphinx instance has no native "
			"decoder reference");
    return ps;
}

cmd_ln_t *
get_cmd_ln_t(PSObj *self) {
    cmd_ln_t *config = self->config;
    if (config == NULL)
	PyErr_SetString(PyExc_ValueError, "PocketSphinx instance has no native "
			"config reference");
    
    return config;
}

int
PSObj_init(PSObj *self, PyObject *args, PyObject *kwds) {
    PyObject *ps_args = NULL;
    Py_ssize_t list_size;

    static char *kwlist[] = {"ps_args", NULL};
    
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|O", kwlist, &ps_args))
        return -1;

    if (ps_args && ps_args != Py_None) {
	if (!PyList_Check(ps_args)) {
	    // Raise the exception flag and return -1
            PyErr_SetString(PyExc_TypeError, "parameter must be a list");
	    return -1;
	}
        
	// Extract strings from Python list into a C string array and use that
	// to call init_ps_decoder_with_args
	list_size = PyList_Size(ps_args);
	char *strings[list_size];
	for (Py_ssize_t i = 0; i < list_size; i++) {
	    PyObject * item = PyList_GetItem(ps_args, i);
	    if (!PyString_Check(item)) {
		PyErr_SetString(PyExc_TypeError, "all list items must be strings!");
		return -1;
	    }
		
	    strings[i] = PyString_AsString(item);
	}

	// Init a new pocket sphinx decoder or raise a PocketSphinxError and return -1
	if (!init_ps_decoder_with_args(self, list_size, strings)) {
	    PyErr_SetString(PocketSphinxError, "PocketSphinx couldn't be initialised. "
			    "Is your configuration right?");
	    return -1;
	}
    } else {
	// Let Pocket Sphinx use the default configuration if there aren't any arguments
	char *strings[0];
	if (!init_ps_decoder_with_args(self, 0, strings)) {
	    PyErr_SetString(PocketSphinxError, "PocketSphinx couldn't be initialised "
			    "using the default configuration. Is it installed properly?");
	    return -1;
	}
    }

    return 0;
}

PyObject *
PSObj_get_speech_start_callback(PSObj *self, void *closure) {
    Py_INCREF(self->speech_start_callback);
    return self->speech_start_callback;
}

PyObject *
PSObj_get_hypothesis_callback(PSObj *self, void *closure) {
    Py_INCREF(self->hypothesis_callback);
    return self->hypothesis_callback;
}

PyObject *
PSObj_get_in_speech(PSObj *self, void *closure) {
    PyObject * result = NULL;
    ps_decoder_t * ps = get_ps_decoder_t(self);
    if (ps != NULL) {
	uint8 in_speech = ps_get_in_speech(ps);
	if (in_speech) {
	    Py_INCREF(Py_True);
	    result = Py_True;
	} else {
	    Py_INCREF(Py_False);
	    result = Py_False;
	}
    }

    return result;
}

PyObject *
PSObj_get_search_name(PSObj *self, void *closure) {
    Py_INCREF(self->search_name);
    return self->search_name;
}

int
PSObj_set_speech_start_callback(PSObj *self, PyObject *value, void *closure) {
    if (value == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Cannot delete the speech_start_callback "
			"attribute.");
        return -1;
    }

    if (!PyCallable_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "value must be callable.");
	return -1;
    }

    if (!assert_callable_arg_count(value, 0))
	return -1;

    Py_DECREF(self->speech_start_callback);
    Py_INCREF(value);
    self->speech_start_callback = value;

    return 0;
}

int
PSObj_set_hypothesis_callback(PSObj *self, PyObject *value, void *closure) {
    if (value == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Cannot delete the "
			"hypothesis_callback attribute.");
        return -1;
    }

    if (!PyCallable_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "value must be callable.");
	return -1;
    }

    if (!assert_callable_arg_count(value, 1))
	return -1;
    
    Py_DECREF(self->hypothesis_callback);
    Py_INCREF(value);
    self->hypothesis_callback = value;

    return 0;
}

PyGetSetDef PSObj_getseters[] = {
    {"speech_start_callback",
     (getter)PSObj_get_speech_start_callback,
     (setter)PSObj_set_speech_start_callback,
     "Callable object called when speech started.", NULL},
    {"hypothesis_callback",
     (getter)PSObj_get_hypothesis_callback,
     (setter)PSObj_set_hypothesis_callback,
     "Hypothesis callback called with Pocket Sphinx's hypothesis for "
     "what was said.", NULL},
    {"in_speech",
     (getter)PSObj_get_in_speech, NULL, // No setter. AttributeError is thrown on set attempt.
     // From pocketsphinx.h:
     "Checks if the last feed audio buffer contained speech.", NULL},
    {"search_name",
     (getter)PSObj_get_search_name, NULL,
     "Get the current pocket sphinx search name.", NULL},
    {NULL}  /* Sentinel */
};

PyTypeObject PSType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sphinxwrapper.PocketSphinx",             /* tp_name */
    sizeof(PSObj),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PSObj_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "Pocket Sphinx decoder objects",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PSObj_methods,             /* tp_methods */
    0,                         /* tp_members */
    PSObj_getseters,           /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PSObj_init,      /* tp_init */
    0,                         /* tp_alloc */
    PSObj_new,                 /* tp_new */
};

bool
init_ps_decoder_with_args(PSObj *self, int argc, char *argv[]) {
    char const *cfg; 
    ps_decoder_t *ps;
    cmd_ln_t *config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);
    
    /* Handle argument file as -argfile. */
    if (config && (cfg = cmd_ln_str_r(config, "-argfile")) != NULL) {
        config = cmd_ln_parse_file_r(config, cont_args_def, cfg, FALSE);
    }

    if (config == NULL) {
	return false;
    }
    
    ps_default_search_args(config);
    ps = ps_init(config);

    if (ps == NULL) {
	return false;
    }
    
    // Set a pointer to the new decoder used only in C.
    self->ps = ps;

    // Retain the config for later use.
    // This claims ownership of the config struct.
    config = cmd_ln_retain(config);
    
    // Set a pointer to the config
    self->config = config;

    // Set self->search_name
    const char *name = ps_get_search(ps);
    Py_XDECREF(self->search_name);
    if (!name) {
	self->search_name = Py_None;
    } else {
	self->search_name = Py_BuildValue("s", name);
    }
    Py_INCREF(self->search_name);

    return true;
}

void
initpocketsphinx(PyObject *module) {
    // Set up the 'PocketSphinx' type
    PSType.tp_new = PSObj_new;
    if (PyType_Ready(&PSType) < 0) {
        return;
    }

    Py_INCREF(&PSType);
    PyModule_AddObject(module, "PocketSphinx", (PyObject *)&PSType);

    // Define a new Python exception
    PocketSphinxError = PyErr_NewException("sphinxwrapper.PocketSphinxError",
					   NULL, NULL);
    Py_INCREF(PocketSphinxError);

    PyModule_AddObject(module, "PocketSphinxError", PocketSphinxError);
}

