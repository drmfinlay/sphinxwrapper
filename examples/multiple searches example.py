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
This example demonstrates the use of multiple Pocket Sphinx decoder searches and the
batch_process method.
"""
from pyaudio import PyAudio, paInt16

from sphinxwrapper import *
import time


class PSDecoder(object):
    def __init__(self):
        self.buffer_list = []
        self.ps = PocketSphinx()

        # Set up the decoder with a JSGF grammar search.
        grammar = """
        #JSGF V1.0 UTF-8 en;
        grammar g;
        public <greet> = hi <name>;
        <name> = peter | john | mary | anna;
        """
        self.ps.set_jsgf_string("jsgf", grammar)
        self.ps.active_search = "jsgf"

        # Switch back to the default language model (LM) search
        self.ps.active_search = "_default"

        def speech_start_callback():
            print("Speech started.")

        # Set up callbacks.
        self.ps.hypothesis_callback = lambda hyp: self.hyp_callback(hyp)
        self.ps.speech_start_callback = speech_start_callback

    def hyp_callback(self, hyp):
        # Don't continue if the LM search didn't hypothesise anything; nothing was
        # spoken
        if not hyp:
            return

        print("LM Hypothesis: %s" % hyp.hypstr)

        # Set the active search to the JSGF search.
        self.ps.active_search = "jsgf"

        # Batch process the audio list and get a hypothesis back.
        jsgf_hyp = self.ps.batch_process(self.buffer_list, use_callbacks=False)
        if jsgf_hyp:
            jsgf_hyp = jsgf_hyp.hypstr
        print("JSGF hypothesis: %s" % jsgf_hyp)

        # Clear buffer list and switch back to the LM search.
        self.buffer_list = []
        self.ps.end_utterance()  # ensure we're not processing
        self.ps.active_search = "_default"

    def start(self):
        # Recognise from the mic in a loop.
        p = PyAudio()
        stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                        frames_per_buffer=2048)
        stream.start_stream()
        while True:
            buf = stream.read(2048)
            self.buffer_list.append(buf)
            self.ps.process_audio(buf)
            time.sleep(0.1)


if __name__ == "__main__":
    d = PSDecoder()
    d.start()
