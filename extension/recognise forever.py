# Recognise forever example using the C extension version of sphinxwrapper.
#
# Apart from the decoder's callback functions, this is very similar to the
# pocketsphinx_continuous program:
# https://github.com/cmusphinx/pocketsphinx/blob/master/src/programs/continuous.c

import os
import time

from sphinxwrapper import PocketSphinx, AudioDevice


def speech_start_callback():
    print("Speech started.")


def hyp_callback(s):
    print("Hypothesis: %s" % s)


def main():
    # Initialise a decoder with the default configuration.
    # ps = PocketSphinx()

    # The decoder optionally accepts an command-line argument list for
    # decoder configuration. The following will suppress log output.
    ps = PocketSphinx(["-logfn", os.devnull])

    # Set up callback functions.
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hyp_callback

    # Recognise from the mic in a loop.
    ad = AudioDevice()
    ad.open()
    ad.record()
    while True:
        audio = ad.read_audio()
        ps.process_audio(audio)
        time.sleep(0.02)

if __name__ == "__main__":
    main()
