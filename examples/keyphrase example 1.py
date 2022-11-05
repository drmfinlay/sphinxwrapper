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

from pyaudio import PyAudio, paInt16

from sphinxwrapper import *
import time


def main():
    # Initialise a Pocket Sphinx decoder that listens for a keyphrase
    keyphrase = "oh mighty computer"
    cfg = DefaultConfig()
    cfg.set_string("-keyphrase", keyphrase)
    ps = PocketSphinx(cfg)

    def speech_start_callback():
        print("Speech started.")

    def hyp_callback(hyp):
        s = hyp.hypstr if hyp else None
        if s == keyphrase:
            print("Keyphrase spoken. Stopping.")
            exit(0)
        else:
            print("Didn't hear keyphrase. Continuing.")

    # Set up callbacks
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hyp_callback

    # Recognise from the mic in a loop
    p = PyAudio()
    stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                    frames_per_buffer=2048)
    stream.start_stream()
    while True:
        ps.process_audio(stream.read(2048))
        time.sleep(0.1)


if __name__ == "__main__":
    main()
