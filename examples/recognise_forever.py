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
    
    # Recognise from the mic in a loop
    ps.open_rec_from_audio_device()
    ps.start_utterance()  # must do this before processing audio
    while True:
        audio = ps.read_audio()
        ps.process_audio(audio)
        time.sleep(0.1)

if __name__ == "__main__":
    main()
