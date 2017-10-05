from sphinxwrapper import *
import time


def main():
    # Initialise a Pocket Sphinx decoder that listens for a keyphrase
    keyphrase = "oh mighty computer"
    ps = PocketSphinx(["-keyphrase", keyphrase])

    def speech_start_callback():
        print("Speech started.")
    
    def hyp_callback(s):
        if s == keyphrase:
            print("Keyphrase spoken. Stopping.")
            exit(0)
        else:
            print("Didn't hear keyphrase. Continuing.")

    # Set up callbacks
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hyp_callback
    
    # Recognise from the mic in a loop
    ad = AudioDevice()
    ad.open()
    ad.record()
    while True:
        ps.process_audio(ad.read_audio())
        time.sleep(0.1)

if __name__ == "__main__":
    main()
