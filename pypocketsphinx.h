/*
 * pypocketsphinx.h
 *
 *  Created on: 22 Sept. 2017
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

#ifndef PYPOCKETSPHINX_H_
#define PYPOCKETSPHINX_H_

#include <stdio.h>
#include <stdbool.h>
#include <Python.h>
#include <pocketsphinx.h>
#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/err.h>
#include <sphinxbase/fsg_model.h>
#include <sphinxbase/prim_type.h>

#include "audio.h"
#include "pyutil.h"

typedef enum {
    IDLE,
    STARTED,
    ENDED
} utterance_state_t;

typedef enum {
    JSGF_FILE, // JSpeech Grammar Format search from file
    JSGF_STR,  // JSpeech Grammar Format search from string
    LM_FILE,   // Language model search from file
    FSG_FILE,  // Finite state grammar search from file
    KWS_FILE,  // Key word/phrase search from file
    KWS_STR    // Key word/phrase search from string
} ps_search_type;

typedef struct {
    PyObject_HEAD
    ps_decoder_t *ps; // pocketsphinx decoder pointer
    cmd_ln_t * config; // sphinxbase commandline config struct pointer
    PyObject *hypothesis_callback; // callable or None
    PyObject *speech_start_callback; // callable or None
    PyObject *search_name; // string
    // Utterance state used in processing methods
    utterance_state_t utterance_state;
} PSObj;

PyObject *
PSObj_process_audio_internal(PSObj *self, PyObject *audio_data,
                             bool call_callbacks);

PyObject *
PSObj_process_audio(PSObj *self, PyObject *audio_data);

PyObject *
PSObj_batch_process(PSObj *self, PyObject *list);

PyObject *
PSObj_set_search_internal(PSObj *self, ps_search_type search_type,
                          PyObject *args, PyObject *kwds);

PyObject *
PSObj_set_jsgf_file_search(PSObj *self, PyObject *args, PyObject *kwds);

PyObject *
PSObj_set_jsgf_str_search(PSObj *self, PyObject *args, PyObject *kwds);

PyObject *
PSObj_set_lm_search(PSObj *self, PyObject *args, PyObject *kwds);

PyObject *
PSObj_set_fsg_search(PSObj *self, PyObject *args, PyObject *kwds);

PyObject *
PSObj_set_keyphrase_search(PSObj *self, PyObject *args, PyObject *kwds);

PyObject *
PSObj_set_keyphrases_search(PSObj *self, PyObject *args, PyObject *kwds);

PyObject *
PSObj_set_config_argument(PSObj *self, PyObject *args, PyObject *kwds);

PyObject *
PSObj_get_config_argument(PSObj *self, PyObject *args, PyObject *kwds);

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
PSObj_get_active_search(PSObj *self, void *closure);

int
PSObj_set_speech_start_callback(PSObj *self, PyObject *value, void *closure);

int
PSObj_set_hypothesis_callback(PSObj *self, PyObject *value, void *closure);

int
PSObj_set_active_search(PSObj *self, PyObject *value, void *closure);

PyTypeObject PSType;

/*
 * Initialise a Pocket Sphinx decoder with arguments.
 * @return true on success, false on failure
 */
bool
init_ps_decoder_with_args(PSObj *self, int argc, char *argv[]);

PyObject *
initpocketsphinx(PyObject *module);

#endif /* PYPOCKETSPHINX_H_ */
