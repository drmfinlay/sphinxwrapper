"""
This is an example for using the PocketSphinx end_utterance method:

There are two decoders listening to the same recorded audio: one is using a language
model (LM) search, and the other is using a keyphrase search with the word 'alexa'.

If 'alexa' is heard by the second decoder, the utterance for the first decoder is
ended early using end_utterance(). This has the effect of stopping this decoder from
calling its hypothesis callback.

There are other use cases for this method. For example, if a user is speaking a
command in the context of a focused window on a desktop system and changes to a
different window, then perhaps the command is now invalid in that context and the
utterance should be ended, preventing the hypothesis callback from being reached.

Note: the Pocket Sphinx INFO output for this example is a bit spammy because there
are two decoders, so it's a bit difficult to see what's going on.
"""


from sphinxwrapper import *
import time


def main():
    # Set up a decoder with a default LM search
    ps1 = PocketSphinx()

    # And one with a keyphrase search
    ps2 = PocketSphinx(["-keyphrase", "alexa"])

    # Define some callbacks for the decoders
    def speech_start_callback():
        print("Speech started.")

    def hyp_callback_1(s):
        # Put some newlines in to make this more visible in the output
        print("\n\nHypothesis: %s\n\n" % s)

    def hyp_callback_2(s):
        if s == "alexa":
            print("\n\nKeyphrase heard. Ending utterance for decoder 1.\n\n")
            ps1.end_utterance()

    # Set decoder callbacks
    ps1.speech_start_callback = speech_start_callback
    ps2.speech_start_callback = speech_start_callback
    ps1.hypothesis_callback = hyp_callback_1
    ps2.hypothesis_callback = hyp_callback_2

    # Recognise from the mic in a loop
    ad = AudioDevice()
    ad.open()
    ad.record()
    while True:
        audio = ad.read_audio()

        # Process audio with both decoders
        # ps2 processes the audio first because it can end the utterance for ps1
        ps2.process_audio(audio)
        ps1.process_audio(audio)
        time.sleep(0.1)


if __name__ == "__main__":
    main()
