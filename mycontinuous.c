/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2010 Carnegie Mellon University.  All rights
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
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
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

/*
 * Modified by Dane Finlay. I don't hold copyright on this file. Use it 
 * in accordance within the terms above and don't hold me liable or anything
 * either.
 */

#include <sphinxbase/err.h>
#include <ad.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pocketsphinx.h>
#include <sys/select.h>

static cmd_ln_t *config;

static const arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    /* Argument file. */
    {"-argfile",
     ARG_STRING,
     NULL, // Default argument.
     "Argument file giving extra arguments."},
    {"-adcdev",
     ARG_STRING,
     NULL,
     "Name of audio device to use for input."},
    {"-time",
     ARG_BOOLEAN,
     "no",
     "Print word times in file transcription."},
    CMDLN_EMPTY_OPTION
};

static ps_decoder_t *
create_ps_decoder(int argc, char *argv[]) {
    char const *cfg;
    ps_decoder_t *ps;
    config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);
    
    /* Handle argument file as -argfile. */
    if (config && (cfg = cmd_ln_str_r(config, "-argfile")) != NULL) {
        config = cmd_ln_parse_file_r(config, cont_args_def, cfg, FALSE);
    }
    
    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        E_ERROR("PocketSphinx couldn't be initialised. Is your configuration right?");
        return NULL;
    }
    
    return ps;
}

/* Sleep for specified msec */
static void
sleep_msec(int32 ms) {
#if (defined(_WIN32) && !defined(GNUWINCE)) || defined(_WIN32_WCE)
    Sleep(ms);
#else
    /* ------------------- Unix ------------------ */
    struct timeval tmo;
    
    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;
    
    select(0, NULL, NULL, NULL, &tmo);
#endif
}

static void
recognize_from_microphone(ps_decoder_t *ps) {
    // Don't proceed if pocket sphinx (ps) isn't initialised.
    if (ps == NULL) {
	E_ERROR("Pocket Sphinx was not initialised. Have you called 'ps_init()'?\n");
	return;
    }
    

    ad_rec_t *ad;
    int16 adbuf[2048];
    uint8 utt_started, in_speech;
    int32 k;
    char const *hyp;
    const char *mic_dev = cmd_ln_str_r(config, "-adcdev");
    cmd_ln_free_r(config);

    // Doesn't matter if dev is NULL; ad_open_dev will use the
    // defined default device.
    ad = ad_open_dev(mic_dev, 16000);
    if (ad == NULL)
        E_FATAL("Failed to open audio device\n");
    if (ad_start_rec(ad) < 0)
        E_FATAL("Failed to start recording\n");
    
    if (ps_start_utt(ps) < 0)
        E_FATAL("Failed to start utterance\n");
    utt_started = FALSE;
    printf("READY....\n");
    
    // TODO We need to be able to call ps_reinit from Python
    // so that the recognizer updates itself with new configuration
    // such as an updated acoustic model, dictionary or grammar files.
    // This way we don't need to init a new recognizer for EVERY single
    // dragonfly context.
    // C++ would give us an easier way to do all this by encapsulating
    // everything in a class instance accessable from Python through
    // ctypes: https://stackoverflow.com/questions/145270/calling-c-c-from-python#145649
    for (;;) {
        if ((k = ad_read(ad, adbuf, 2048)) < 0)
            E_FATAL("Failed to read audio\n");
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
            printf("Listening...\n");
        }
        if (!in_speech && utt_started) {
            /* speech -> silence transition, time to start new utterance  */
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL );
            if (hyp != NULL)
                printf("%s\n", hyp);
            
            if (ps_start_utt(ps) < 0)
                E_FATAL("Failed to start utterance\n");
            utt_started = FALSE;
            printf("READY....\n");
        }
        // I don't understand why this is necessary.
        // sleep_msec(100);
    }
    ad_close(ad);
}


int
main(int argc, char *argv[]) {
    // Add a recognizer using the program arguments.
    ps_decoder_t *ps = create_ps_decoder(argc, argv);
    
    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);

    if (ps != NULL) {
        recognize_from_microphone(ps);
    } else {
        return -1;
    }
    
    return 0;
}
