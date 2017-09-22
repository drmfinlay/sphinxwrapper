/*
 * sphinxwrapper.c
 *
 *  Created on 8 Jul. 2017
 *      Author: Dane Finlay
 */

#include "audio.h"
#include "pypocketsphinx.h"


#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

static PyMethodDef sphinxwrapper_funcs[] = {
    {NULL, NULL, 0, NULL} // Sentinel signifying the end of the func declarations
};

PyMODINIT_FUNC
initsphinxwrapper(void) {
    PyObject *module = Py_InitModule("sphinxwrapper", sphinxwrapper_funcs);

    // Set up the 'PocketSphinx' type and anything else it needs
    initpocketsphinx(module);

    // Set up the audio related types
    initaudio(module);
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

