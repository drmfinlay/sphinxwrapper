/*
 * sphinxwrapper.c
 *
 *  Created on 8 Jul. 2017
 *      Author: Dane Finlay
 */

#include "sphinxwrapper.h"

static PyObject *PocketSphinxError;
static PyObject *CallbackNotSetError;

const char* ps_capsule_name = "sphinxwrapper.ps_ptr";
const char* config_capsule_name = "sphinxwrapper.config_ptr";

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


/* Sleep for specified msec */
static void
sleep_msec(int32 ms) {
#if (defined(_WIN32) && !defined(GNUWINCE)) || defined(_WIN32_WCE)
    Sleep(ms);
#else
    /* ------------------- Unix ------------------ */
    struct timeval tmo;
    
    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;
    
    select(0, NULL, NULL, NULL, &tmo);
#endif
}

static PyObject *
PSObj_recognize_from_microphone(PSObj *self) {
    ps_decoder_t * ps = get_ps_decoder_t(self);
    
    if (ps == NULL) {
	E_ERROR("Pocket Sphinx was not initialised. Have you called 'ps_init()'?\n");
	return NULL;
    }
    
    ad_rec_t *ad;
    int16 adbuf[2048];
    uint8 utt_started, in_speech;
    int32 k;
    char const *hyp;
    const char *mic_dev = cmd_ln_str_r(get_cmd_ln_t(self), "-adcdev");

    // Doesn't matter if dev is NULL; ad_open_dev will use the
    // defined default device.
    ad = ad_open_dev(mic_dev, 16000);
    if (ad == NULL)
        E_FATAL("Failed to open audio device\n");
    if (ad_start_rec(ad) < 0)
        E_FATAL("Failed to start recording\n");
    
    if (ps_start_utt(ps) < 0)
        E_FATAL("Failed to start utterance\n");
    utt_started = FALSE;
    printf("READY....\n");
    
    // TODO We need to be able to call ps_reinit from Python
    // so that the recognizer updates itself with new configuration
    // such as an updated acoustic model, dictionary or grammar files.
    for (;;) {
        if ((k = ad_read(ad, adbuf, 2048)) < 0)
            E_FATAL("Failed to read audio\n");
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
            printf("Listening...\n");
        }
        if (!in_speech && utt_started) {
            /* speech -> silence transition, time to start new utterance  */
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL );
            if (hyp != NULL) {
                printf("%s\n", hyp);
		// Call the Python hypothesis_callback function
		PyObject *py_hyp = Py_BuildValue("s", hyp);
		PyObject *callback = self->hypothesis_callback;
		if (PyCallable_Check(callback)) {
		    PyObject_CallObject(callback, py_hyp);
		}
	    }
            
            if (ps_start_utt(ps) < 0)
                E_FATAL("Failed to start utterance\n");
            utt_started = FALSE;
            printf("READY....\n");
        }
        sleep_msec(30);
    }
    ad_close(ad);
}


static PyMethodDef PSObj_methods[] = {
    {"recognize_from_microphone",
     (PyCFunction)PSObj_recognize_from_microphone, METH_NOARGS,
     PyDoc_STR("Recognize speech from the microphone")},
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
        self->test_callback = Py_None;
        Py_INCREF(Py_None);
        self->hypothesis_callback = Py_None;

	// Do the same with capsules
        Py_INCREF(Py_None);
        self->ps_capsule = Py_None;
        Py_INCREF(Py_None);
        self->config_capsule = Py_None;
    }

    return (PyObject *)self;
}

static void
PSObj_dealloc(PSObj* self) {
    Py_XDECREF(self->hypothesis_callback);
    Py_XDECREF(self->test_callback);
    
    // Deallocate the config object and its Py_Capsule
    cmd_ln_t *config = get_cmd_ln_t(self);
    if (config != NULL)
	cmd_ln_free_r(config);
    Py_XDECREF(self->config_capsule);

    // Deallocate the Pocket Sphinx decoder and its Py_Capsule
    ps_decoder_t *ps = get_ps_decoder_t(self);
    if (ps != NULL)
	ps_free(ps);
    Py_XDECREF(self->ps_capsule);

    // Finally free the PSObj itself
    Py_TYPE(self)->tp_free((PyObject*)self);
}


static ps_decoder_t *
get_ps_decoder_t(PSObj *self) {
    ps_decoder_t *result = NULL;
    if (PyCapsule_IsValid(self->ps_capsule, ps_capsule_name)) {
	result = PyCapsule_GetPointer(self->ps_capsule, ps_capsule_name);
    } else {
	PyErr_SetString(PyExc_ValueError, "PocketSphinx instance has no native "
			"decoder reference");
    }

    return result;
}

static cmd_ln_t *
get_cmd_ln_t(PSObj *self) {
    cmd_ln_t *result = NULL;
    if (PyCapsule_IsValid(self->config_capsule, config_capsule_name)) {
	result = PyCapsule_GetPointer(self->config_capsule, config_capsule_name);
    } else {
	PyErr_SetString(PyExc_ValueError, "PocketSphinx instance has no native "
			"config reference");
    }

    return result;
}

static int
PSObj_init(PSObj *self, PyObject *args, PyObject *kwds) {
    PyObject *ps_args=NULL, *tmp;
    Py_ssize_t list_size;

    static char *kwlist[] = {"ps_args", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &ps_args))
        return -1;

    if (ps_args) {
	if (!PyList_Check(ps_args)) {
	    // Raise the exception flag and return -1
            PyErr_SetString(PyExc_TypeError, "parameter must be a list");
	    return -1;
	} else if (PyList_Size(ps_args) < 1) {
            PyErr_SetString(PyExc_IndexError, "list must have at least 1 item");
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
    }

    return 0;
}

static PyObject *
PSObj_get_test_callback(PSObj *self, void *closure) {
    Py_INCREF(self->test_callback);
    return self->test_callback;
}

static PyObject *
PSObj_get_hypothesis_callback(PSObj *self, void *closure) {
    Py_INCREF(self->hypothesis_callback);
    return self->hypothesis_callback;
}

static int
PSObj_set_test_callback(PSObj *self, PyObject *value, void *closure) {
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the test_callback attribute");
        return -1;
    }

    if (!PyCallable_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "value must be callable");
	return -1;
    }
    
    Py_DECREF(self->test_callback);
    Py_INCREF(value);
    self->test_callback = value;

    return 0;
}

static int
PSObj_set_hypothesis_callback(PSObj *self, PyObject *value, void *closure) {
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the hypothesis_callback attribute");
        return -1;
    }

    if (!PyCallable_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "value must be callable");
	return -1;
    }
    
    Py_DECREF(self->hypothesis_callback);
    Py_INCREF(value);
    self->hypothesis_callback = value;

    return 0;
}

static PyGetSetDef PSObj_getseters[] = {
    {"test_callback",
     (getter)PSObj_get_test_callback, (setter)PSObj_set_test_callback,
     "Test callback", NULL},
    {"hypothesis_callback",
     (getter)PSObj_get_hypothesis_callback, (setter)PSObj_set_hypothesis_callback,
     "Hypothesis callback", NULL},
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
    
    PyObject *tmp;
    
    // Create a capsule containing a pointer to the new decoder used only in C.
    PyObject *new_capsule1 = PyCapsule_New(ps, ps_capsule_name, NULL);
    tmp = self->ps_capsule;
    Py_INCREF(new_capsule1);
    self->ps_capsule = new_capsule1;
    Py_XDECREF(tmp);
    
    // Create another one to store the config pointer for later use
    PyObject *new_capsule2 = PyCapsule_New(config, config_capsule_name, NULL);
    tmp = self->config_capsule;
    Py_INCREF(new_capsule2);
    self->config_capsule = new_capsule2;
    Py_XDECREF(tmp);

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

