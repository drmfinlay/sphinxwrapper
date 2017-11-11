/*
 * audio.h
 *
 *  Created on: 31 August 2017
 *      Author: Dane Finlay
 *
 * Part of this file is based on source code from the CMU Pocket Sphinx project.
 * As such, the below copyright notice and conditions apply IN ADDITION TO the 
 * sphinxwrapper project's LICENSE file.
 *
 * ====================================================================
 * Copyright (c) 1999-2016 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */


#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdbool.h>
#include <Python.h>

// Required for int16 and int32
#include <sphinxbase/prim_type.h>

// Required to use sphinxbase audio device implementations
#include <sphinxbase/ad.h>

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

PyObject *
initaudio(PyObject *module);

#endif /* AUDIO_H_ */
