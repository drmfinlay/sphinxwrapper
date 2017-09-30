/*
 * audio.c
 *
 *  Created on: 2nd September 2017
 *      Author: Dane Finlay
 */

#include "audio.h"

void
AudioDataObj_dealloc(AudioDataObj* self) {
    // Free the Python type object
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *
AudioDataObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    AudioDataObj *self;

    self = (AudioDataObj *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->is_set = false;
    }

    return (PyObject *)self;
}

int
AudioDataObj_init(AudioDataObj *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {NULL};

    // Accept no arguments.
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist))
        return -1;

    return 0;
}

PyMethodDef AudioDataObj_methods[] = {
    {NULL}  /* Sentinel */
};

PyGetSetDef AudioDataObj_getseters[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject AudioDataType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sphinxwrapper.AudioData",        /* tp_name */
    sizeof(AudioDataObj),             /* tp_basicsize */
    0,                                /* tp_itemsize */
    (destructor)AudioDataObj_dealloc, /* tp_dealloc */
    0,                                /* tp_print */
    0,                                /* tp_getattr */
    0,                                /* tp_setattr */
    0,                                /* tp_compare */
    0,                                /* tp_repr */
    0,                                /* tp_as_number */
    0,                                /* tp_as_sequence */
    0,                                /* tp_as_mapping */
    0,                                /* tp_hash */
    0,                                /* tp_call */
    0,                                /* tp_str */
    0,                                /* tp_getattro */
    0,                                /* tp_setattro */
    0,                                /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,          /* tp_flags */
    "Audio data objects containing "
    "an audio buffer to process.",    /* tp_doc */
    0,                                /* tp_traverse */
    0,                                /* tp_clear */
    0,                                /* tp_richcompare */
    0,                                /* tp_weaklistoffset */
    0,                                /* tp_iter */
    0,                                /* tp_iternext */
    AudioDataObj_methods,             /* tp_methods */
    0,                                /* tp_members */
    AudioDataObj_getseters,           /* tp_getset */
    0,                                /* tp_base */
    0,                                /* tp_dict */
    0,                                /* tp_descr_get */
    0,                                /* tp_descr_set */
    0,                                /* tp_dictoffset */
    (initproc)AudioDataObj_init,      /* tp_init */
    0,                                /* tp_alloc */
    AudioDataObj_new,                 /* tp_new */
};

PyObject *
AudioDeviceObj_open(AudioDeviceObj *self) {
    if (self->open) {
	PyErr_SetString(AudioDeviceError,
                        "Audio device is already open.");
	return NULL;
    }
    const char *dev = NULL;

    // Get a C representation of the audio device name from self->name
    // This is operation is different in Python 2.7 and 3+
#if PY_MAJOR_VERSION >= 3
    if (PyUnicode_Check(self->name)) {
	dev = PyUnicode_AsUTF8(self->name);
#else
    if (PyString_Check(self->name)) {
	dev = PyString_AsString(self->name);
#endif
    }

    if (self->name != Py_None && dev == NULL) {
	// There was an error in the PyString_AsString or PyUnicode_AsUTF8 functions
	// so assume a Python error message was set and return NULL
	return NULL;
    }

    // Doesn't matter if dev is NULL; ad_open_dev will use the
    // defined default device, at least for pulse audio..
    self->ad = ad_open_dev(dev, 16000);

    // If it's still NULL, then that's an error.
    if (self->ad == NULL) {
        PyErr_SetString(AudioDeviceError,
                        "Failed to open audio device");
        return NULL;
    }

    self->open = true;

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
AudioDeviceObj_record(AudioDeviceObj *self) {
    if (self->recording) {
        PyErr_SetString(AudioDeviceError,
                        "Audio device is already recording.");
        return NULL;
    }

    if (self->ad == NULL) {
        PyErr_SetString(AudioDeviceError,
			"Failed to start recording: device is not open.");
	return NULL;
    }

    if (ad_start_rec(self->ad) < 0) {
        PyErr_SetString(AudioDeviceError,
			"Failed to start recording.");
	return NULL;
    }

    self->recording = true;

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
AudioDeviceObj_stop_recording(AudioDeviceObj *self) {
    if (!self->recording) {
        PyErr_SetString(AudioDeviceError,
                        "Audio device is not recording.");
        return NULL;
    }

    if (self->ad != NULL) {
        if (ad_stop_rec(self->ad) < 0) {
            PyErr_SetString(AudioDeviceError,
                            "Failed to stop recording.");
            return NULL;
        }
    }

    self->recording = false;

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
AudioDeviceObj_close(AudioDeviceObj *self) {
    ad_rec_t *ad = self->ad;
    if (ad != NULL) {
        if (self->recording) {
            PyObject *stop_rec = AudioDeviceObj_stop_recording(self);
            if (stop_rec == NULL) {
                return NULL;
            } else {
                // Decrease ref count of stop_recording's result.
                Py_DECREF(stop_rec);
            }
        }

        if (!self->open) {
	    PyErr_SetString(AudioDeviceError,
                            "Audio device is already closed.");
            return NULL;
	}

        if (ad_close(ad) < 0) {
            PyErr_SetString(AudioDeviceError,
                            "Failed to close audio device.");
            return NULL;
        }
    }

    self->ad = NULL;
    self->open = false;

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
AudioDeviceObj_read_audio(AudioDeviceObj *self) {
    if (self->ad == NULL) {
        PyErr_SetString(AudioDeviceError,
                        "Failed to read audio. Have you called open() and "
			"record()?");
        return NULL;
    }

    // Create a new audio buffer to use
    PyObject *audio_data = PyObject_CallObject((PyObject *)&AudioDataType, NULL);
    AudioDataObj *audio_data_c = (AudioDataObj *)audio_data;

    int32 n_samples = ad_read(self->ad, audio_data_c->audio_buffer, 2048);
    if (n_samples < 0) {
	PyErr_SetString(AudioDeviceError, "Failed to read audio.");
	return NULL;
    }

    audio_data_c->n_samples = n_samples;
    audio_data_c->is_set = true;
    return audio_data;
}

void
AudioDeviceObj_dealloc(AudioDeviceObj* self) {
    // Close the audio device if it's open
    // and stop recording
    ad_rec_t *ad = self->ad;
    if (ad != NULL) {
        ad_stop_rec(ad);
        ad_close(ad);
    }

    Py_XDECREF(self->name);

    // Free the Python type object
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *
AudioDeviceObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    AudioDeviceObj *self;

    self = (AudioDeviceObj *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // Ensure the 'ad' member used when opening and recording
        // from audio devices is set to NULL for now.
	self->ad = NULL;

        Py_INCREF(Py_None);
        self->name = Py_None;

        self->open = false;
        self->recording = false;
    }

    return (PyObject *)self;
}

int
AudioDeviceObj_init(AudioDeviceObj *self, PyObject *args, PyObject *kwds) {
    char* name = NULL;
    static char *kwlist[] = {"name", NULL};

    // Accept one optional argument
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &name)) {
        return -1;
    }

    if (name != NULL) {
        self->name = Py_BuildValue("s", name);
    }

    return 0;
}

int
AudioDeviceObj_set_name(AudioDeviceObj *self, PyObject *value, void *closure) {
    if (value == NULL) {
	PyErr_SetString(PyExc_AttributeError, "cannot delete the name attribute.");
        return -1;
    }
#if PY_MAJOR_VERSION >= 3
    if (PyUnicode_Check(value) || value == Py_None) {
#else
    if (PyString_Check(value) || value == Py_None) {
#endif
	Py_DECREF(self->name);
        self->name = value;
	Py_INCREF(self->name);
    } else {
         PyErr_SetString(PyExc_TypeError, "value must be a string or None.");
        return -1;
    }

    return 0;
}

PyObject *
AudioDeviceObj_get_name(AudioDeviceObj *self, void *closure) {
    Py_INCREF(self->name);
    return self->name;
}

PyMethodDef AudioDeviceObj_methods[] = {
    {"open",
     (PyCFunction)AudioDeviceObj_open, METH_NOARGS,
     PyDoc_STR("Open the audio device.")},
    {"record",
     (PyCFunction)AudioDeviceObj_record, METH_NOARGS,
     PyDoc_STR("Start recording from the audio device.")},
    {"stop_recording",
     (PyCFunction)AudioDeviceObj_stop_recording, METH_NOARGS,
     PyDoc_STR("Start recording from the audio device.")},
    {"read_audio",
     (PyCFunction)AudioDeviceObj_read_audio, METH_NOARGS,
     PyDoc_STR("Read audio from the audio device if it is open and recording.\n"
	       ":rtype: AudioData")},
    {"close",
     (PyCFunction)AudioDeviceObj_close, METH_NOARGS,
     PyDoc_STR("If it's open, close the audio device.")},
    {NULL}  /* Sentinel */
};

PyGetSetDef AudioDeviceObj_getseters[] = {
    {"name",
     (getter)AudioDeviceObj_get_name,
     (setter)AudioDeviceObj_set_name,
     "The name of this audio device.", NULL},
    {NULL}  /* Sentinel */
};

PyTypeObject AudioDeviceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sphinxwrapper.AudioDevice",        /* tp_name */
    sizeof(AudioDeviceObj),             /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)AudioDeviceObj_dealloc, /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_compare */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,            /* tp_flags */
    "Audio device object for reading "
    "audio from an audio device.",      /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    AudioDeviceObj_methods,             /* tp_methods */
    0,                                  /* tp_members */
    AudioDeviceObj_getseters,           /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)AudioDeviceObj_init,      /* tp_init */
    0,                                  /* tp_alloc */
    AudioDeviceObj_new,                 /* tp_new */
};

PyObject *
initaudio(PyObject *module) {
    AudioDataType.tp_new = AudioDataObj_new;
    if (PyType_Ready(&AudioDataType) < 0) {
        return NULL;
    }

    Py_INCREF(&AudioDataType);
    PyModule_AddObject(module, "AudioData", (PyObject *)&AudioDataType);

    // Define a new Python exception for when an invalid AudioData object is used.
    AudioDataError = PyErr_NewException("sphinxwrapper.AudioDataError", NULL, NULL);
    Py_INCREF(AudioDataError);
    PyModule_AddObject(module, "AudioDataError", AudioDataError);

    // Set up the AudioDevice type using a non-generic new method
    AudioDeviceType.tp_new = AudioDeviceObj_new;
    if (PyType_Ready(&AudioDeviceType) < 0) {
	return NULL;
    }

    Py_INCREF(&AudioDeviceType);
    PyModule_AddObject(module, "AudioDevice", (PyObject *)&AudioDeviceType);

    // Define another new exception for failing to open the audio device or read
    // audio from it
    AudioDeviceError = PyErr_NewException("sphinxwrapper.AudioDeviceError", NULL, NULL);
    Py_INCREF(AudioDeviceError);
    PyModule_AddObject(module, "AudioDeviceError", AudioDeviceError);

    return module;
}

