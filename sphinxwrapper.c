/*
 * sphinxwrapper.c
 *
 *  Created on 8 Jul. 2017
 *      Author: Dane Finlay
 */

#include "sphinxwrapper.h"

static PyObject *PocketSphinxError;
static PyObject *CallbackNotSetError;

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
    {"-inmic",
     ARG_BOOLEAN,
     "no",
     "Transcribe audio from microphone."},
    {"-time",
     ARG_BOOLEAN,
     "no",
     "Print word times in file transcription."},
    CMDLN_EMPTY_OPTION
};

static PyObject *
PSObj_open_rec_from_audio_device(PSObj *self) {
    ps_decoder_t * ps = get_ps_decoder_t(self);

    if (ps == NULL)
	return NULL;
    
    if (self->ad == NULL) {
	const char *mic_dev = cmd_ln_str_r(get_cmd_ln_t(self), "-adcdev");

	// Doesn't matter if dev is NULL; ad_open_dev will use the
	// defined default device, at least for pulse audio..
	// TODO Make sure other ad.h implementations (alsa, jack, etc)
	// don't seg fault or anything if mic_dev is NULL.
	self->ad = ad_open_dev(mic_dev, 16000);
    }

    // If it's still NULL, then that's an error.
    if (self->ad == NULL) {
	PyErr_SetString(PocketSphinxError,
			"Couldn't open audio device.");
	return NULL;
    }
    
    if (ad_start_rec(self->ad) < 0) {
        PyErr_SetString(PocketSphinxError,
			"Failed to start recording.");
	return NULL;
    }

    self->utterance_started = false;
    printf("READY....\n");
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
PSObj_close_audio_device(PSObj *self) {
    ad_rec_t *ad = self->ad;
    if (ad != NULL) {
	ad_close(ad);
	self->ad = NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
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

static PyObject *
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

static PyObject *
PSObj_read_and_process_audio(PSObj *self) {
    ps_decoder_t * ps = get_ps_decoder_t(self);

    if (ps == NULL)
	return NULL;
    
    // Open and record from audio device if necessary
    if (self->ad == NULL) {
	PSObj_open_rec_from_audio_device(self);
	if (self->ad == NULL)
	    return NULL;
    }
    
    uint8 in_speech;
    int32 k = ad_read(self->ad, self->adbuf, 2048);
    char const *hyp;
    if (k < 0) {
	PyErr_SetString(PocketSphinxError, "Failed to read audio.");
	self->ad = NULL;
	ps_end_utt(ps);
	return NULL;
    }
    
    ps_process_raw(ps, self->adbuf, k, FALSE, FALSE);
    
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

static PyMethodDef PSObj_methods[] = {
    {"open_rec_from_audio_device",
     (PyCFunction)PSObj_open_rec_from_audio_device, METH_NOARGS,
     PyDoc_STR("Open the audio device for recording speech and start recording.")},
    {"close_audio_device",
     (PyCFunction)PSObj_close_audio_device, METH_NOARGS,
     PyDoc_STR("If it's open, close the audio device used to record speech.")},
    {"start_utterance",
     (PyCFunction)PSObj_start_utterance, METH_NOARGS,
     PyDoc_STR("Call this function before passing any utterance data.")},
    {"end_utterance",
     (PyCFunction)PSObj_end_utterance, METH_NOARGS,
     PyDoc_STR("Call this function to end utterance processing.")},
    {"read_and_process_audio",
     (PyCFunction)PSObj_read_and_process_audio, METH_NOARGS,
     PyDoc_STR("Read and process audio.")},
    {NULL}  /* Sentinel */
};


#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

static PyObject *
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

	// Set jsgf_info to (None, None)
        Py_INCREF(Py_None);
        Py_INCREF(Py_None);
	self->jsgf_info = Py_BuildValue("(O,O)", Py_None, Py_None);
	
	// Ensure pointer members are NULL
        self->ps = NULL;
        self->config = NULL;

	// Ensure the 'ad' member used in init_mic_recording
	// and read_and_process_audio in set to NULL for now.
	self->ad = NULL;
	self->utterance_started = false;
    }

    return (PyObject *)self;
}

static void
PSObj_dealloc(PSObj* self) {
    Py_XDECREF(self->hypothesis_callback);
    Py_XDECREF(self->speech_start_callback);
    
    // Deallocate the config object
    cmd_ln_t *config = self->config;
    if (config != NULL)
	cmd_ln_free_r(config);

    // Deallocate the Pocket Sphinx decoder
    ps_decoder_t *ps = self->ps;
    if (ps != NULL)
	ps_free(ps);

    // Close the audio device if it's open
    ad_rec_t *ad = self->ad;
    if (ad != NULL)
	ad_close(ad);

    // Finally free the PSObj itself
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static ps_decoder_t *
get_ps_decoder_t(PSObj *self) {
    ps_decoder_t *ps = self->ps;
    if (ps == NULL)
	PyErr_SetString(PyExc_ValueError, "PocketSphinx instance has no native "
			"decoder reference");
    return ps;
}

static cmd_ln_t *
get_cmd_ln_t(PSObj *self) {
    cmd_ln_t *config = self->config;
    if (config == NULL)
	PyErr_SetString(PyExc_ValueError, "PocketSphinx instance has no native "
			"config reference");
    
    return config;
}

static int
PSObj_init(PSObj *self, PyObject *args, PyObject *kwds) {
    PyObject *ps_args=NULL;
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

static PyObject *
PSObj_get_speech_start_callback(PSObj *self, void *closure) {
    Py_INCREF(self->speech_start_callback);
    return self->speech_start_callback;
}

static PyObject *
PSObj_get_hypothesis_callback(PSObj *self, void *closure) {
    Py_INCREF(self->hypothesis_callback);
    return self->hypothesis_callback;
}

static PyObject *
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

static PyObject *
PSObj_get_jsgf_info(PSObj *self, void *closure) {
    Py_INCREF(self->jsgf_info);
    return self->jsgf_info;
}

static bool
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

static int
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

static int
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

static int
PSObj_set_jsgf_info(PSObj *self, PyObject *value, void *closure) {
    if (value == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Cannot delete the "
			"jsgf_info attribute.");
        return -1;
    }

    if (!PyTuple_Check(value) || PyTuple_Size(value) != (Py_ssize_t)2) {
        PyErr_SetString(PyExc_TypeError, "value must be a tuple of "
			"length 2.");
	return -1;
    }

    PyObject *name = PyTuple_GetItem(value, (Py_ssize_t)0);
    PyObject *jsgf_str = PyTuple_GetItem(value, (Py_ssize_t)1);
    
    if (!PyString_Check(name) || !PyString_Check(jsgf_str)) {
        PyErr_SetString(PyExc_TypeError, "both objects in the tuple "
			"must be strings: a name and a JSGF grammar "
			"string.");
	return -1;
    }
    
    ps_decoder_t * ps = get_ps_decoder_t(self);

    ps_unset_search(ps, PyString_AsString(name));

    // Set the value using ps_set_jsgf_string
    if (ps_set_jsgf_string(ps, PyString_AsString(name),
			   PyString_AsString(jsgf_str)) == -1) {
	PyErr_SetString(PocketSphinxError, "Something went wrong when "
			"setting JSGF grammar name and/or string. "
			"Please check for syntax or semantic errors.");
	return -1;
    }
    
    Py_INCREF(value);
    self->jsgf_info = value;

    return 0;
}

static PyGetSetDef PSObj_getseters[] = {
    {"speech_start_callback",
     (getter)PSObj_get_speech_start_callback,
     (setter)PSObj_set_speech_start_callback,
     "Callable object called when speech started.", NULL},
    {"hypothesis_callback",
     (getter)PSObj_get_hypothesis_callback,
     (setter)PSObj_set_hypothesis_callback,
     "Hypothesis callback called with Pocket Sphinx's hypothesis for "
     "what was said.", NULL},
    {"jsgf_info",
     (getter)PSObj_get_jsgf_info,
     (setter)PSObj_set_jsgf_info,
     "Java Speech Grammar Format information used by Pocket Sphinx "
     "to set up recogniser to recognise JSGF rules in speech.\n"
     "Requires a tuple containing 2 strings: a name and a valid JSGF "
     "grammar.", NULL},
    {"in_speech",
     (getter)PSObj_get_in_speech, NULL, // No setter. AttributeError is thrown on set attempt.
     // From pocketsphinx.h:
     "Checks if the last feed audio buffer contained speech", NULL},
    {NULL}  /* Sentinel */
};

static PyTypeObject PSType = {
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

static bool
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
    
    // Set a pointer to the config for later use
    self->config = config;

    return true;
}

static PyMethodDef sphinxwrapper_funcs[] = {
    {NULL, NULL, 0, NULL} // Sentinel signifying the end of the func declarations
};

PyMODINIT_FUNC
initsphinxwrapper() {
    PyObject *module = Py_InitModule("sphinxwrapper", sphinxwrapper_funcs);

    // Set up the 'PocketSphinx' type
    PSType.tp_new = PSObj_new;
    if (PyType_Ready(&PSType) < 0)
        return;
    
    Py_INCREF(&PSType);
    PyModule_AddObject(module, "PocketSphinx", (PyObject *)&PSType);

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

