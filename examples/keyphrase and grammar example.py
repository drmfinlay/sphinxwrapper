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
This module demonstrates how to use sphinxwrapper and PyAudio to listen for
and process a keyphrase followed by a separately spoken command.

In this example, commands are defined in the *commands.jsgf* grammar file.
"""

import datetime
import os
import platform
import time

from pyaudio import PyAudio, paInt16

from sphinxwrapper import PocketSphinx, DefaultConfig


#-------------------------------------------------------------------------------
# Define global variables.

ps = None
keyphrase = "computer"
listen_for_speech = True


#-------------------------------------------------------------------------------
# Define decoder callbacks.

def speech_start_callback():
    print("Speech started.")


def hypothesis_callback_keyphrase(hyp):
    hypstr = hyp.hypstr if hyp else None
    if hypstr == keyphrase:
        # Activate the JSGF grammar search and callback.
        ps.active_search = "grammar"
        ps.hypothesis_callback = hypothesis_callback_grammar
        print("Keyphrase spoken. Waiting for command.")
    else:
        print("Didn't hear keyphrase.")


def hypothesis_callback_grammar(hyp):
    hypstr = hyp.hypstr if hyp else None
    print("Hypothesis: %r" % hypstr)

    # If nothing was spoken, return early.
    if not hypstr:
        print("Waiting for command.")
        return

    # Handle the spoken command.
    if "date" in hypstr or "time" in hypstr or "day" in hypstr:
        now = datetime.datetime.now()
        message = {
            "what is the time": now.strftime("%I:%M%p"),
            "what day is it": now.strftime("%A"),
            "what is the date": now.strftime("%d %B %Y"),
        }[hypstr]
        print(message)
    elif hypstr == "what operating system is running":
        print(platform.system())
    elif hypstr == "stop listening":
        global listen_for_speech
        listen_for_speech = False
        print("Goodbye!")

    # Reactivate the keyphrase search and callback.
    ps.active_search = "_default"
    ps.hypothesis_callback = hypothesis_callback_keyphrase


#-------------------------------------------------------------------------------
# Define the main function.

def main():
    global ps

    # Set Pocket Sphinx decoder configuration so that audio is scanned for the
    # keyphrase "computer".
    # Note: Keyphrases may contain any word in the active dictionary.
    config = DefaultConfig()
    config.set_string("-keyphrase", keyphrase)
    config.set_string("-logfn", os.devnull)  # Suppress log output.

    # Initialise the decoder.
    ps = PocketSphinx(config)

    # Set a JSGF search which, when active, scans audio for commands defined in
    # the *commands.jsgf* grammar file.
    # Note: The above '-logfn' option will suppress any JSGF compilation errors.
    ps.set_jsgf_file("grammar", "commands.jsgf")

    # Set callback functions.
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hypothesis_callback_keyphrase

    # Open an audio stream on the default input audio device.
    p = PyAudio()
    stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                    frames_per_buffer=2048, input_device_index=None)
    stream.start_stream()

    # Recognise from the microphone in a loop until interrupted.
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
