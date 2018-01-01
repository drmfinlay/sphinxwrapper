from pyaudio import PyAudio, paInt16

from sphinxwrapper import *
import time


def speech_start_callback():
    print("Speech started.")


def hyp_callback(hyp):
    s = hyp.hypstr if hyp else None
    print("Hypothesis: %s" % s)


def main():
    ps = PocketSphinx()
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
