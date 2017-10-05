from sphinxwrapper import *
import time


def main():
    # Initialise a Pocket Sphinx decoder with the default configuration
    ps = PocketSphinx()
    
    # Set up a keyphrase search with the name 'keyphrase'
    # The keyphrase can contain any word in the active dictionary
    keyphrase = "oh mighty computer"
    ps.set_keyphrase_search(keyphrase, "keyphrase")

    def speech_start_callback():
        print("Speech started.")
        
    def hyp_callback(s):
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
    ad = AudioDevice()
    ad.open()
    ad.record()
    while True:
        ps.process_audio(ad.read_audio())
        time.sleep(0.1)

if __name__ == "__main__":
    main()
