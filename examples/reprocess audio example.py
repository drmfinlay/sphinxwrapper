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
This example demonstrates how to use sphinxwrapper and PyAudio to scan and
interpret audio from the microphone multiple times.

In this example, the default language model and the keyword list defined in the
*keyword-list.txt* file are used to interpret speech one after another.
"""

import os
import time

from pyaudio import PyAudio, paInt16

from sphinxwrapper import PocketSphinx, DefaultConfig


#-------------------------------------------------------------------------------
# Define global variables.

ps = None
buffers = []


#-------------------------------------------------------------------------------
# Define decoder callbacks.

def speech_start_callback():
    global speech_start_buffers_size

    print("Speech started.")

    # Trim unneeded buffers from before speech was started.
    # We will keep at most 10 pre-speech buffers.  If the default audio
    # parameters are used, this amounts to keeping a little more than one second
    # of audio.  This much is needed for voice activity detection (VAD).
    #
    # This is an optimisation that decreases batch reprocessing time in the
    # event of a long pause between the last hypothesis and the next word
    # spoken.  It is only needed if batch processing is required.
    while len(buffers) > 11: buffers.pop(0)


def hypothesis_callback(hyp1):
    hypstr1 = hyp1.hypstr if hyp1 else None
    print("Hypothesis 1: %r" % hypstr1)

    # Set the active search to the keyword-list search.
    ps.active_search = "kws"

    # Batch process each audio buffer and get a hypothesis back.
    hyp2 = ps.batch_process(buffers, use_callbacks=False)
    hypstr2 = hyp2.hypstr if hyp2 else None
    print("Hypothesis 2: %r" % hypstr2)

    # Clear the buffer list for next time.
    while len(buffers) > 0: buffers.pop()

    # Ensure the decoder is in the idle state and switch back to the default
    # language model search.
    ps.end_utterance()
    ps.active_search = "_default"


#-------------------------------------------------------------------------------
# Define the main function.

def main():
    global ps

    # Set Pocket Sphinx decoder configuration so that audio is scanned and
    # interpreted using the default language model and dictionary.
    config = DefaultConfig()
    config.set_string("-logfn", os.devnull)  # Suppress log output.

    # Initialise the decoder.
    ps = PocketSphinx(config)

    # Set a keyword-list search which, when active, scans audio for words
    # defined in the *keyword-list.txt* file.
    ps.set_kws("kws", "keyword-list.txt")

    # Set callback functions.
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hypothesis_callback

    # Open an audio stream on the default input audio device.
    p = PyAudio()
    stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                    frames_per_buffer=2048, input_device_index=None)
    stream.start_stream()

    # Recognise from the microphone in a loop until interrupted.
    try:
        print("Listening... Press Ctrl+C to exit...")
        while True:
            buf = stream.read(2048)
            buffers.append(buf)
            ps.process_audio(buf)
            time.sleep(0.1)
    except KeyboardInterrupt: pass
    stream.stop_stream()
    p.terminate()


if __name__ == "__main__":
    main()
