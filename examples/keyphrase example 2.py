from pyaudio import PyAudio, paInt16

from sphinxwrapper import *
import time


def main():
    # Initialise a Pocket Sphinx decoder with the default configuration
    ps = PocketSphinx()
    
    # Set up a keyphrase search with the name 'keyphrase'
    # The keyphrase can contain any word in the active dictionary
    keyphrase = "computer"
    ps.set_keyphrase("keyphrase", keyphrase)
    ps.active_search = "keyphrase"

    def speech_start_callback():
        print("Speech started.")
        
    def hyp_callback(hyp):
        s = hyp.hypstr if hyp else None
        if s == keyphrase:
            print("Keyphrase spoken.")

            # Swap back to the default language model search
            ps.active_search = "_default"
        else:
            print("Hypothesis: %s" % s)

            # Swap back to the keyphrase search
            ps.active_search = "keyphrase"

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
