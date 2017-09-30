/*
 * audio.h
 *
 *  Created on: 31 August 2017
 *      Author: Dane Finlay
 */


#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdbool.h>
#include <Python.h>

// Required for int16 and int32
#include <sphinxbase/prim_type.h>

// Required to use sphinxbase audio device implementations
#include <sphinxbase/ad.h>

#include "pyutil.h"

typedef struct {
    PyObject_HEAD
    int16 audio_buffer[2048]; // array used to store audio data
    int32 n_samples;
    bool is_set; // used to check if the object is set up correctly
} AudioDataObj;

PyTypeObject AudioDataType;

PyObject *AudioDataError;

void
AudioDataObj_dealloc(AudioDataObj* self);

PyObject *
AudioDataObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

int
AudioDataObj_init(AudioDataObj *self, PyObject *args, PyObject *kwds);

typedef struct {
    PyObject_HEAD
    ad_rec_t *ad; // Used for recording audio
    PyObject *name;
    bool open;
    bool recording;
} AudioDeviceObj;

PyObject *
AudioDeviceObj_open(AudioDeviceObj *self);

PyObject *
AudioDeviceObj_record(AudioDeviceObj *self);

PyObject *
AudioDeviceObj_stop_recording(AudioDeviceObj *self);

PyObject *
AudioDeviceObj_close(AudioDeviceObj *self);

PyObject *
AudioDeviceObj_read_audio(AudioDeviceObj *self);

void
AudioDeviceObj_dealloc(AudioDeviceObj* self);

PyObject *
AudioDeviceObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

int
AudioDeviceObj_init(AudioDeviceObj *self, PyObject *args, PyObject *kwds);

int
AudioDeviceObj_set_name(AudioDeviceObj *self, PyObject *value, void *closure);

PyObject *
AudioDeviceObj_get_name(AudioDeviceObj *self, void *closure);

PyTypeObject AudioDeviceType;

PyObject *AudioDeviceError;

void
initaudio(PyObject *module);

#endif /* AUDIO_H_ */
