#
# Copyright (c) 2017-2022 Dane Finlay
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

"""
This module demonstrates how to use sphinxwrapper and PyAudio to listen for and
process a keyword list. If a keyword entry is detected, it will be printed out
and the program will terminate.

See the following webpage for information on keyword-lists and CMU Pocket
Sphinx: https://cmusphinx.github.io/wiki/tutoriallm/#keyword-lists

In this example, the keyword list defined in the *keyword-list.txt* file is
used.
"""

import os
import time

from pyaudio import PyAudio, paInt16

from sphinxwrapper import PocketSphinx, DefaultConfig


#-------------------------------------------------------------------------------
# Define global variables.

listen_for_speech = True


#-------------------------------------------------------------------------------
# Define decoder callbacks.

def speech_start_callback():
    print("Speech started.")


def hypothesis_callback(hyp):
    # Note: strip() is required here to workaround a bug in Pocket Sphinx.
    hypstr = hyp.hypstr.strip() if hyp else None

    # If none of the keywords were spoken, return early.
    if not hypstr:
        print("Didn't hear keyword.")
        print("Listening...")
        return

    # Otherwise, a keyphrase was spoken.
    print("Keyphrase %r spoken. Stopping..." % hypstr)
    global listen_for_speech
    listen_for_speech = False


#-------------------------------------------------------------------------------
# Define the main function.

def main():
    # Set Pocket Sphinx decoder configuration so that audio is scanned for words
    # defined in the *keyword-list.txt* file.
    config = DefaultConfig()
    config.set_string("-kws", "keyword-list.txt")
    config.set_string("-logfn", os.devnull)  # Suppress log output.

    # Initialise the decoder.
    ps = PocketSphinx(config)

    # The *ps.set_kws_list()* method may be used to set a keyword-list search
    # with a dictionary instead of a file. Uncomment the following code to try
    # it out:
    #ps.set_kws_list("kws", {
    #    "computer": 1e-20,
    #    "hello computer": 1e-30,
    #    "hello pocket sphinx": 1e-40
    #})
    #ps.active_search = "kws"

    # Set callback functions.
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hypothesis_callback

    # Open an audio stream on the default input audio device.
    p = PyAudio()
    stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                    frames_per_buffer=2048, input_device_index=None)
    stream.start_stream()

    # Recognise from the microphone in a loop. Stop if interrupted or if the
    # keyphrase is spoken.
    try:
        print("Listening... Press Ctrl+C to exit...")
        while listen_for_speech:
            ps.process_audio(stream.read(2048))
            time.sleep(0.1)
    except KeyboardInterrupt: pass
    stream.stop_stream()
    p.terminate()


if __name__ == "__main__":
    main()
