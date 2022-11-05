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
This is an example for using the PocketSphinx end_utterance method:

There are two decoders listening to the same recorded audio: one is using a language
model (LM) search, and the other is using a keyphrase search with the word 'alexa'.

If 'alexa' is heard by the second decoder, the utterance for the first decoder is
ended early using end_utterance(). This has the effect of stopping this decoder from
calling its hypothesis callback.

There are other use cases for this method. For example, if a user is speaking a
command in the context of a focused window on a desktop system and changes to a
different window, then perhaps the command is now invalid in that context and the
utterance should be ended, preventing the hypothesis callback from being reached.
"""


from sphinxwrapper import *
from pyaudio import PyAudio, paInt16
import time, os


def main():
    # Set up a decoder with a default LM search
    cfg = DefaultConfig()

    # Also discard Pocket Sphinx's log output because otherwise it's a bit difficult
    # to see what's going on with two decoders running. Do this using the portable
    # os.devnull path
    cfg.set_string("-logfn", os.devnull)
    ps1 = PocketSphinx(cfg)

    # Set up another decoder with a keyphrase search
    cfg = DefaultConfig()
    cfg.set_string("-keyphrase", "alexa")
    cfg.set_string("-logfn", os.devnull)
    ps2 = PocketSphinx(cfg)

    # Define some callbacks for the decoders
    def speech_start_callback():
        print("Speech started.")

    def hyp_callback_1(hyp):
        if not hyp:
            return
        print("Hypothesis: %s" % hyp.hypstr)

    def hyp_callback_2(hyp):
        if not hyp:
            print("Keyphrase not heard, continuing.")
            return
        s = hyp.hypstr
        if s == "alexa":
            print("Keyphrase heard. Ending utterance for decoder 1.")
            ps1.end_utterance()  # this is a guarded method.

    # Set decoder callbacks
    ps1.speech_start_callback = speech_start_callback
    ps2.speech_start_callback = speech_start_callback
    ps1.hypothesis_callback = hyp_callback_1
    ps2.hypothesis_callback = hyp_callback_2

    # Recognise from the mic in a loop
    p = PyAudio()
    stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                    frames_per_buffer=2048)
    stream.start_stream()
    try:
        while True:
            audio = stream.read(2048)

            # Process audio with both decoders
            # ps2 processes the audio first because it can end the utterance for ps1
            ps2.process_audio(audio)
            ps1.process_audio(audio)
            time.sleep(0.1)
    except KeyboardInterrupt:
        stream.close()


if __name__ == "__main__":
    main()
