from sphinxwrapper import *
import time


def speech_start_callback():
    print("Speech started.")


def hyp_callback(s):
    print("Hypothesis: %s" % s)


def main():
    ps = PocketSphinx()
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hyp_callback

    # Set up the decoder with a JSGF grammar loaded from a file
    ps.set_jsgf_file_search("grammar.jsgf")
    
    # Recognise from the mic in a loop
    ad = AudioDevice()
    ad.open()
    ad.record()
    while True:
        ps.process_audio(ad.read_audio())
        time.sleep(0.1)

if __name__ == "__main__":
    main()
