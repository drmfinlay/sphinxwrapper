from sphinxwrapper import *
# import time


def speech_start_callback():
    print("Speech started.")


def hyp_callback(s):
    print("Hypothesis: %s" % s)


def main():
    ps = PocketSphinx()
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hyp_callback

    # Set up the decoder with a JSGF grammar
    ps.set_jsgf_search("""
    #JSGF V1.0 UTF-8 en;
    grammar g;
    public <greet> = hi <name>;
    <name> = peter | john | mary | anna;
    """)
    
    # Recognise from the mic in a loop
    ad = AudioDevice()
    ad.open()
    ad.record()
    while True:
        audio = ad.read_audio()
        ps.process_audio(audio)
        # time.sleep(0.1)

if __name__ == "__main__":
    main()
