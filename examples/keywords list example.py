"""
This is an example for using the PocketSphinx set_kws_list method to recognise a
list of keyphrases. Once a keyphrase is heard, it will be printed and the program
will terminate.

Credit the example words and threshold values are from the following CMU Sphinx
tutorial:
https://cmusphinx.github.io/wiki/tutoriallm/#keyword-lists
"""

from pyaudio import PyAudio, paInt16

from sphinxwrapper import *
import time


def main():
    ps = PocketSphinx()
    keywords_list = {
        "oh mighty computer": 1e-40,
        "hello world": 1e-30,
        "other phrase": 1e-20
    }

    # Note: this method is more suited to configurations where multiple searches are
    # used, such as an LM search.
    # Set a keywords search using a dictionary
    ps.set_kws_list("kws", keywords_list)
    ps.active_search = "kws"

    def speech_start_callback():
        print("Speech started.")

    def hyp_callback(hyp):
        s = hyp.hypstr if hyp else None
        if s:
            print("Keyphrase '%s' spoken. Stopping." % s)
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
