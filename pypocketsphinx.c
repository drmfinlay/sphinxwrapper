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
PSObj_process_audio_internal(PSObj *self, PyObject *audio_data,
			     bool call_callbacks) {
    ps_decoder_t * ps = get_ps_decoder_t(self);

    if (ps == NULL)
        return NULL;

    if (!PyObject_TypeCheck(audio_data, &AudioDataType)) {
        PyErr_SetString(PyExc_TypeError, "argument or item is not an AudioData "
			"object.");
        return NULL;
    }

    AudioDataObj *audio_data_c = (AudioDataObj *)audio_data;

    if (!audio_data_c->is_set) {
        PyErr_SetString(AudioDataError, "AudioData object is not set up properly. "
			"Try using the result from AudioDevice.read_audio()");
        return NULL;
    }

    // Call ps_start_utt if necessary
    if (self->utterance_state == ENDED) {
	ps_start_utt(ps);
        self->utterance_state = IDLE;
    }

    ps_process_raw(ps, audio_data_c->audio_buffer, audio_data_c->n_samples, FALSE, FALSE);

    uint8 in_speech = ps_get_in_speech(ps);
    PyObject *result = Py_None; // incremented at end of function as result

    if (in_speech && self->utterance_state == IDLE) {
        self->utterance_state = STARTED;

        // Call speech_start callback if necessary
        PyObject *callback = self->speech_start_callback;
        if (call_callbacks && PyCallable_Check(callback)) {
	    // NULL args means no args are required.
	    PyObject *cb_result = PyObject_CallObject(callback, NULL);
	    if (cb_result == NULL) {
		result = cb_result;
	    }
        }
    } else if (!in_speech && self->utterance_state == STARTED) {
        /* speech -> silence transition, time to start new utterance  */
        ps_end_utt(ps);

	char const *hyp = ps_get_hyp(ps, NULL);
	
	// Call the Python hypothesis callback if it is callable
	// It should have the correct number of arguments because
	// of the checks in set_hypothesis_callback
	PyObject *callback = self->hypothesis_callback;
	if (call_callbacks && PyCallable_Check(callback)) {
	    PyObject *args;
	    if (hyp != NULL) {
		args = Py_BuildValue("(s)", hyp);
	    } else {
		Py_INCREF(Py_None);
		args = Py_BuildValue("(O)", Py_None);
	    }

	    PyObject *cb_result = PyObject_CallObject(callback, args);
	    if (cb_result == NULL) {
		result = cb_result;
	    }
	} else if (!call_callbacks) {
	    // Return the hypothesis instead
	    result = Py_BuildValue("s", hyp);
	}

        self->utterance_state = ENDED;
    }

    Py_XINCREF(result);
    return result;
}

PyObject *
PSObj_process_audio(PSObj *self, PyObject *audio_data) {
    return PSObj_process_audio_internal(self, audio_data, true);
}

PyObject *
PSObj_batch_process(PSObj *self, PyObject *list) {
    if (list == NULL || !PyList_Check(list)) {
        PyErr_SetString(PyExc_TypeError, "argument must be a list");
	return NULL;
    }

    Py_ssize_t list_size = PyList_Size(list);
    PyObject *result;
    if (list_size == 0) {
	Py_INCREF(Py_None);
	result = Py_None;
    }

    for (Py_ssize_t i = 0; i < list_size; i++) {
        PyObject * item = PyList_GetItem(list, i);
        if (!PyObject_TypeCheck(item, &AudioDataType)) {
            PyErr_SetString(PyExc_TypeError, "all list items must be AudioData objects!");
            return NULL;
        }
	result = PSObj_process_audio_internal(self, item, false);

	// Break on errors so NULL is returned
	if (result == NULL)
	    break;
    }

    return result;
}

PyObject *
PSObj_set_search_internal(PSObj *self, ps_search_type search_type,
			  PyObject *args,PyObject *kwds) {
    // Set up the keyword list
    char *req_kw;
    switch (search_type) {
    case JSGF_STR:
	req_kw = "str";
	break;
    case KWS_STR:
	req_kw = "keyphrase";
	break;
    default: // everything else requires a file path
	req_kw = "path";
    }
    char *kwlist[] = {req_kw, "name", NULL};

    const char *value = NULL;
    const char *name = NULL;
    PyObject *result = Py_None; // incremented at end of function

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|s", kwlist, &value, &name))
        return NULL;
    
    ps_decoder_t * ps = get_ps_decoder_t(self);
    if (ps == NULL)
	return NULL;

    if (name == NULL)
	name = PS_DEFAULT_SEARCH;

    // TODO Do dictionary and LM checks for missing words - maybe add them using 
    // ps_add_word

    int set_result = -1;
    switch (search_type) {
    case JSGF_FILE:
	set_result = ps_set_jsgf_file(ps, name, value);
	break;
    case JSGF_STR:
	set_result = ps_set_jsgf_string(ps, name, value);
	break;
    case LM_FILE:
	set_result = ps_set_lm_file(ps, name, value);
	break;
    case FSG_FILE:
	; // required because you cannot declare immediately after a label in C
	// Get the config used to initialise the decoder
	cmd_ln_t *config = get_cmd_ln_t(self);
	// Create a fsg model from the file and set it using the search name
	fsg_model_t *fsg = fsg_model_readfile(value, ps_get_logmath(ps),
					      cmd_ln_float32_r(config, "-lw"));
	if (!fsg) {
	    set_result = -1;
	    break;
	}
	
	set_result = ps_set_fsg(ps, name, fsg);

	// This should be done whether or not ps_set_fsg fails, apparently..
	fsg_model_free(fsg);
	break;
    case KWS_FILE:
	// TODO Allow use of a Python list of keyword arguments rather than a file
	set_result = ps_set_kws(ps, name, value);
	break;
    case KWS_STR:
	set_result = ps_set_keyphrase(ps, name, value);
	break;
    }

    // Set the search if set_result is fine or set an error
    if (set_result < 0 || (ps_set_search(ps, name) < 0)) {
	PyErr_Format(PocketSphinxError, "something went wrong whilst setting up the "
		     "Pocket Sphinx search with name '%s'.", name);
	result = NULL;
    }

    // Keep the current search name up to date
    Py_XDECREF(self->search_name);
    self->search_name = Py_BuildValue("s", name);
    Py_INCREF(self->search_name);
    
    Py_XINCREF(result);
    return result;
}


PyObject *
PSObj_set_jsgf_file_search(PSObj *self, PyObject *args, PyObject *kwds) {
    return PSObj_set_search_internal(self, JSGF_FILE, args, kwds);
}

PyObject *
PSObj_set_jsgf_str_search(PSObj *self, PyObject *args, PyObject *kwds) {
    return PSObj_set_search_internal(self, JSGF_STR, args, kwds);
}

PyObject *
PSObj_set_lm_search(PSObj *self, PyObject *args, PyObject *kwds) {
    return PSObj_set_search_internal(self, LM_FILE, args, kwds);
}

PyObject *
PSObj_set_fsg_search(PSObj *self, PyObject *args, PyObject *kwds) {
    return PSObj_set_search_internal(self, FSG_FILE, args, kwds);
}

PyObject *
PSObj_set_keyphrase_search(PSObj *self, PyObject *args, PyObject *kwds) {
    return PSObj_set_search_internal(self, KWS_STR, args, kwds);
}

PyObject *
PSObj_set_keyphrases_search(PSObj *self, PyObject *args, PyObject *kwds) {
    return PSObj_set_search_internal(self, KWS_FILE, args, kwds);
}

// Define a macro for documenting multiple search methods
#define PS_SEARCH_DOC_FOOTER(first_keyword_docstring)			\
    "Setting an already used search name will replace that "		\
    "Pocket Sphinx search.\n\n"						\
    "Keyword arguments:\n"						\
    first_keyword_docstring "\n"					\
    "name -- name of the Pocket Sphinx search to set (default '"	\
    PS_DEFAULT_SEARCH "')\n"						\

PyMethodDef PSObj_methods[] = {
    {"process_audio",
     (PyCFunction)PSObj_process_audio, METH_O,  // takes self + one argument
     PyDoc_STR("Process audio from an AudioData object and call the speech_start "
	       "and hypothesis callbacks where necessary.")},
    {"batch_process",
     (PyCFunction)PSObj_batch_process, METH_O,  // takes self + one argument
     PyDoc_STR("Process a list of AudioData objects and return a speech hypothesis "
	       "or None.\n"
	       "This method doesn't call speech_start or hypothesis callbacks.")},
    {"set_jsgf_file_search",
     (PyCFunction)PSObj_set_jsgf_file_search, METH_KEYWORDS | METH_VARARGS,
     PyDoc_STR("Set a Pocket Sphinx search using a JSpeech Grammar Format grammar "
	       "file.\n"
	       PS_SEARCH_DOC_FOOTER("path -- file path to the JSGF file to use."))},
    {"set_jsgf_str_search",
     (PyCFunction)PSObj_set_jsgf_str_search, METH_KEYWORDS | METH_VARARGS,
     PyDoc_STR("Set a Pocket Sphinx search using a JSpeech Grammar Format grammar "
	       "string.\n"
	       PS_SEARCH_DOC_FOOTER("str -- the JSGF string to use."))},
    {"set_lm_search",
     (PyCFunction)PSObj_set_lm_search, METH_KEYWORDS | METH_VARARGS,
     PyDoc_STR("Set a Pocket Sphinx search using a language model file.\n"
	       PS_SEARCH_DOC_FOOTER("path -- file path to the LM file to use."))},
    {"set_fsg_search",
     (PyCFunction)PSObj_set_fsg_search, METH_KEYWORDS | METH_VARARGS,
     PyDoc_STR("Set a Pocket Sphinx search using a finite state grammar file.\n"
	       PS_SEARCH_DOC_FOOTER("path -- file path to the FSG file to use."))},
    {"set_keyphrase_search",
     (PyCFunction)PSObj_set_keyphrase_search, METH_KEYWORDS | METH_VARARGS,
     PyDoc_STR("Set a Pocket Sphinx search using a single keyphrase to listen for.\n"
	       PS_SEARCH_DOC_FOOTER("keyphrase -- the keyphrase to listen for."))},
    {"set_keyphrases_search",
     (PyCFunction)PSObj_set_keyphrases_search, METH_KEYWORDS | METH_VARARGS,
     PyDoc_STR("Set a Pocket Sphinx search using a file containing keyphrases to "
	       "listen for.\n"
	       PS_SEARCH_DOC_FOOTER("path -- file path to the keyphrases file to "
				    "use."))},
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

	self->utterance_state = ENDED;
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
	    char *err_msg = "all list items must be strings!";
#if PY_MAJOR_VERSION >= 3
	    if (!PyUnicode_Check(item)) {
		PyErr_SetString(PyExc_TypeError, err_msg);
	    }

	    strings[i] = PyUnicode_AsUTF8(item);
#else
	    if (!PyString_Check(item)) {
		PyErr_SetString(PyExc_TypeError, err_msg);
		return -1;
	    }
		
	    strings[i] = PyString_AsString(item);
#endif
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

#if PY_MAJOR_VERSION >= 2 && PY_MAJOR_VERSION < 3
    if (!assert_callable_arg_count(value, 0))
	return -1;
#endif

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

#if PY_MAJOR_VERSION >= 2 && PY_MAJOR_VERSION < 3
    if (!assert_callable_arg_count(value, 1))
	return -1;
#endif

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

PyObject *
initpocketsphinx(PyObject *module) {
    // Set up the 'PocketSphinx' type
    PSType.tp_new = PSObj_new;
    if (PyType_Ready(&PSType) < 0) {
	return NULL;
    }

    Py_INCREF(&PSType);
    PyModule_AddObject(module, "PocketSphinx", (PyObject *)&PSType);

    // Define a new Python exception
    PocketSphinxError = PyErr_NewException("sphinxwrapper.PocketSphinxError",
					   NULL, NULL);
    Py_INCREF(PocketSphinxError);

    PyModule_AddObject(module, "PocketSphinxError", PocketSphinxError);
    return module;
}

