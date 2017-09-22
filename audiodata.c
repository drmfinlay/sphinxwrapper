/*
 * audiobuffer.c
 *
 *  Created on: 2nd September 2017
 *      Author: Dane Finlay
 */

#include "audiodata.h"

void
AudioDataObj_dealloc(AudioDataObj* self) {
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

void
initaudiodata(PyObject *module) {
    AudioDataType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&AudioDataType) < 0)
        return;

    Py_INCREF(&AudioDataType);
    PyModule_AddObject(module, "AudioData", (PyObject *)&AudioDataType);

    // Define a new Python exception for when an invalid AudioData object is used.
    AudioDataError = PyErr_NewException("AudioData.Error", NULL, NULL);
    Py_INCREF(AudioDataError);
    PyModule_AddObject(module, "AudioDataError", AudioDataError);
}

