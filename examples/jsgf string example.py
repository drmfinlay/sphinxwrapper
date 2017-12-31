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

    # Set up the decoder with a JSGF grammar string
    ps.set_jsgf_string("jsgf", """
    #JSGF V1.0 UTF-8 en;
    grammar g;
    public <greet> = hi <name>;
    <name> = peter | john | mary | anna;
    """)
    ps.active_search = "jsgf"

    # Recognise from the mic in a loop
    p = PyAudio()
    stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                    output=True, frames_per_buffer=2048)
    stream.start_stream()
    while True:
        ps.process_audio(stream.read(2048))
        time.sleep(0.1)

if __name__ == "__main__":
    main()
