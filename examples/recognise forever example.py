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
This module demonstrates how to use sphinxwrapper and PyAudio to scan and
interpret audio from the microphone.

In this example, the default language model is used to interpret speech.
"""

import os
import time

from pyaudio import PyAudio, paInt16

from sphinxwrapper import PocketSphinx, DefaultConfig


#-------------------------------------------------------------------------------
# Define decoder callbacks.

def speech_start_callback():
    print("Speech started.")


def hypothesis_callback(hyp):
    hypstr = hyp.hypstr if hyp else None
    print("Hypothesis: %r" % hypstr)


#-------------------------------------------------------------------------------
# Define the main function.

def main():
    # Set Pocket Sphinx decoder configuration so that audio is scanned and
    # interpreted using the default language model and dictionary.  This is the
    # default behaviour.
    config = DefaultConfig()
    config.set_string("-logfn", os.devnull)  # Suppress log output.

    # Initialise the decoder.
    ps = PocketSphinx(config)

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
            ps.process_audio(stream.read(2048))
            time.sleep(0.1)
    except KeyboardInterrupt: pass
    stream.stop_stream()
    p.terminate()

if __name__ == "__main__":
    main()
