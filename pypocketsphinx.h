/*
 * pypocketsphinx.h
 *
 *  Created on: 22 Sept. 2017
 *      Author: Dane Finlay
 */

#ifndef PYPOCKETSPHINX_H_
#define PYPOCKETSPHINX_H_

#include <stdio.h>
#include <stdbool.h>
#include <python2.7/Python.h>
#include <python2.7/structmember.h>
#include <pocketsphinx.h>
#include <sphinxbase/err.h>
#include <sphinxbase/cmd_ln.h>

#include "audio.h"
#include "pyutil.h"

typedef struct {
    PyObject_HEAD
    ps_decoder_t *ps; // pocketsphinx decoder pointer
    cmd_ln_t * config; // sphinxbase commandline config struct pointer
    PyObject *hypothesis_callback; // callable
    PyObject *speech_start_callback; // callable
    PyObject *search_name;
    // Whether an utterance has been started for recognising audio input.
    // used in 'process_audio' method
    bool utterance_started;
} PSObj;

PyObject *
PSObj_start_utterance(PSObj *self);

PyObject *
PSObj_end_utterance(PSObj *self);

PyObject *
PSObj_process_audio(PSObj *self, PyObject *audio_data);

PyObject *
PSObj_set_jsgf_search(PSObj *self, PyObject *args, PyObject *kwds);

PyObject *
PSObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

void
PSObj_dealloc(PSObj* self);

/* Used to get the ps_decoder_t pointer stored in a PSObj instance,
 * or if it's NULL, return NULL and call PyErr_SetString.
 */
ps_decoder_t *
get_ps_decoder_t(PSObj *self);

/* Used to get the cmd_ln_t pointer stored in a PSObj instance,
 * or if it's NULL, return NULL and call PyErr_SetString.
 */
cmd_ln_t *
get_cmd_ln_t(PSObj *self);

int
PSObj_init(PSObj *self, PyObject *args, PyObject *kwds);

PyObject *
PSObj_get_speech_start_callback(PSObj *self, void *closure);

PyObject *
PSObj_get_hypothesis_callback(PSObj *self, void *closure);

PyObject *
PSObj_get_in_speech(PSObj *self, void *closure);

PyObject *
PSObj_get_search_name(PSObj *self, void *closure);

int
PSObj_set_speech_start_callback(PSObj *self, PyObject *value, void *closure);

int
PSObj_set_hypothesis_callback(PSObj *self, PyObject *value, void *closure);

PyTypeObject PSType;

/*
 * Initialise a Pocket Sphinx decoder with arguments.
 * @return true on success, false on failure
 */
bool
init_ps_decoder_with_args(PSObj *self, int argc, char *argv[]);

void
initpocketsphinx(PyObject *module);

#endif /* PYPOCKETSPHINX_H_ */
