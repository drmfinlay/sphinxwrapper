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
#include <python2.7/ceval.h>
#include <sphinxbase/err.h>
#include <sphinxbase/cmd_ln.h>
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>

static void
sleep_msec(int32 ms);

typedef struct {
    PyObject_HEAD
    PyObject *ps_args; // list containing only strings and at least 1
    PyObject *ps_capsule; // hidden Py_Capsule object containing a ref to ps_decoder_t
    PyObject *config_capsule; // hidden Py_Capsule object containing a ref to cmd_ln_t
    PyObject *hypothesis_callback; // callable
    PyObject *test_callback; // callable
} PSObj;

static PyObject *
PSObj_start_recognizing_from_mic(PSObj *self);

static PyObject *
PSObj_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

static void
PSObj_dealloc(PSObj* self);

/* Used to get the pointer stored in a PSObj instance's capsule,
 * or NULL if there isn't one and also call PyErr_SetString.
 */
static ps_decoder_t *
get_ps_decoder_t(PSObj *self);

static cmd_ln_t *
get_cmd_ln_t(PSObj *self);

static int
PSObj_init(PSObj *self, PyObject *args, PyObject *kwds);

static PyObject *
PSObj_get_test_callback(PSObj *self, void *closure);

static PyObject *
PSObj_get_hypothesis_callback(PSObj *self, void *closure);

static int
PSObj_set_test_callback(PSObj *self, PyObject *value, void *closure);

static int
PSObj_set_hypothesis_callback(PSObj *self, PyObject *value, void *closure);

/*
 * Initialise a Pocket Sphinx decoder with arguments.
 * @return true on success, false on failure
 */
static bool
init_ps_decoder_with_args(PSObj *self, int argc, char *argv[]);

#endif /* SPHINXWRAPPER_H_ */

