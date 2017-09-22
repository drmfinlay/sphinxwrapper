/*
 * audiodata.h
 *
 *  Created on: 31 August 2017
 *      Author: Dane Finlay
 */


#ifndef AUDIODATA_H_
#define AUDIODATA_H_

#include <stdbool.h>
#include <python2.7/Python.h>
#include <python2.7/structmember.h>

// Required for int16 and int32
#include <sphinxbase/prim_type.h>

typedef struct {
    PyObject_HEAD
    int16 audio_buffer[2048]; // array used to store audio data
    int32 n_samples;
    bool is_set; // used to check if the object is set up correctly
} AudioDataObj;

PyTypeObject AudioDataType;

void
AudioDataObj_dealloc(AudioDataObj* self);

PyObject *
AudioDataObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

int
AudioDataObj_init(AudioDataObj *self, PyObject *args, PyObject *kwds);

PyObject *AudioDataError;

void
initaudiodata(PyObject *module);

#endif /* AUDIODATA_H_ */
