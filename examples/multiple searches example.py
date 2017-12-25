"""
This example demonstrates the use of multiple Pocket Sphinx decoder searches and the
batch_process method.
"""

from sphinxwrapper import *
import time


class PSDecoder(object):
    def __init__(self):
        self.audio_list = []
        self.ps = PocketSphinx()

        # Set up the decoder with a JSGF grammar search.
        grammar = """
        #JSGF V1.0 UTF-8 en;
        grammar g;
        public <greet> = hi <name>;
        <name> = peter | john | mary | anna;
        """
        self.ps.set_jsgf_str_search(grammar, "jsgf")

        # Switch back to the default language model (LM) search
        self.ps.active_search = "_default"

        def speech_start_callback():
            print("Speech started.")

        # Set up callbacks.
        self.ps.hypothesis_callback = lambda s: self.hyp_callback(s)
        self.ps.speech_start_callback = speech_start_callback

    def hyp_callback(self, s):
        print("LM Hypothesis: %s" % s)

        # Don't continue if the LM search didn't hypothesise anything; nothing was
        # spoken
        if not s:
            return

        # Set the active search to the JSGF search.
        self.ps.active_search = "jsgf"

        # Batch process the audio list and get a hypothesis back.
        # Note: using True for the second parameter will use the callbacks instead.
        print("JSGF hypothesis: '%s'" % self.ps.batch_process(
            self.audio_list, False))

        # Clear audio list and switch back to the LM search.
        self.audio_list = []
        self.ps.active_search = "_default"

    def start(self):
        # Recognise from the mic in a loop.
        ad = AudioDevice()
        ad.open()
        ad.record()
        while True:
            audio = ad.read_audio()
            self.audio_list.append(audio)
            self.ps.process_audio(audio)
            time.sleep(0.1)


if __name__ == "__main__":
    d = PSDecoder()
    d.start()
