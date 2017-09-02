/*
 * sphinxwrapper.h
 *
 *  Created on: 7 Jul. 2017
 *      Author: Dane Finlay
 */

#ifndef SPHINXWRAPPER_H_
#define SPHINXWRAPPER_H_

#include <stdio.h>
#include <stdbool.h>
#include <python2.7/Python.h>
#include <python2.7/structmember.h>
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <sphinxbase/cmd_ln.h>
#include "audiodata.h"

typedef struct {
    PyObject_HEAD
    ps_decoder_t *ps; // pocketsphinx decoder pointer
    cmd_ln_t * config; // sphinxbase commandline config struct pointer
    PyObject *hypothesis_callback; // callable
    PyObject *speech_start_callback; // callable
    PyObject *search_name;
    ad_rec_t *ad; // Used for recording audio
    // Whether an utterance has been started for recognising audio input.
    // used in 'process_audio' method
    bool utterance_started;
} PSObj;

static PyObject *
PSObj_open_rec_from_audio_device(PSObj *self);

static PyObject *
PSObj_close_audio_device(PSObj *self);

static PyObject *
PSObj_start_utterance(PSObj *self);

static PyObject *
PSObj_end_utterance(PSObj *self);

static PyObject *
PSObj_read_audio(PSObj *self);

static PyObject *
PSObj_process_audio(PSObj *self, PyObject *audio_buffer);

static PyObject *
PSObj_set_jsgf_search(PSObj *self, PyObject *args, PyObject *kwds);

static PyObject *
PSObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

static void
PSObj_dealloc(PSObj* self);

/* Used to get the ps_decoder_t pointer stored in a PSObj instance,
 * or if it's NULL, return NULL and call PyErr_SetString.
 */
static ps_decoder_t *
get_ps_decoder_t(PSObj *self);

/* Used to get the cmd_ln_t pointer stored in a PSObj instance,
 * or if it's NULL, return NULL and call PyErr_SetString.
 */
static cmd_ln_t *
get_cmd_ln_t(PSObj *self);

static int
PSObj_init(PSObj *self, PyObject *args, PyObject *kwds);

static PyObject *
PSObj_get_speech_start_callback(PSObj *self, void *closure);

static PyObject *
PSObj_get_hypothesis_callback(PSObj *self, void *closure);

static PyObject *
PSObj_get_in_speech(PSObj *self, void *closure);

static PyObject *
PSObj_get_search_name(PSObj *self, void *closure);

/* Checks the argument count of a callable Python object and returns true
 * if it matches arg_count, or returns false if it doesn't and sets a 
 * Python exception.
 */
static bool
assert_callable_arg_count(PyObject *value, const unsigned int arg_count);

static int
PSObj_set_speech_start_callback(PSObj *self, PyObject *value, void *closure);

static int
PSObj_set_hypothesis_callback(PSObj *self, PyObject *value, void *closure);

/*
 * Initialise a Pocket Sphinx decoder with arguments.
 * @return true on success, false on failure
 */
static bool
init_ps_decoder_with_args(PSObj *self, int argc, char *argv[]);

#endif /* SPHINXWRAPPER_H_ */

