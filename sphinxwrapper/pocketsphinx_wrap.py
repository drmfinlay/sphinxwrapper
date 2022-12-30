#
# Copyright (c) 2017-2022 Dane Finlay
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

"""
CMU Pocket Sphinx Decoder Class
--------------------------------------------------------------------------------

"""

import os
import tempfile

from pocketsphinx import Decoder, Config

from .config import (set_hmm_and_dict_paths, search_arguments_set,
                     set_lm_path, ConfigError)


class PocketSphinx(Decoder):
    """
    Pocket Sphinx decoder subclass with processing methods providing callback
    functionality, as well as some other things.

    This class will try to set the '-hmm' and '-dict' configuration arguments
    automatically if they are not set prior to initialisation.

    If no search argument is present, the class will also try to set the '-lm'
    argument so that the default language model is used.  Search arguments
    include the following:

     * '-lm'
     * '-jsgf'
     * '-fsg'
     * '-keyphrase'
     * '-kws'

    Construct arguments:

     - *config* -- Decoder configuration object.  Will be initialised using
       :meth:`default_config` if unspecified.

    .. note::

       An error will be raised if the configuration object specifies more than
       search argument.  In this event, the decoder class will not be
       initialised.

    """

    # Internal values used in process_audio to keep track of the utterance
    # state between method calls.
    # This is similar to how Pocket Sphinx handles utterance state in C.
    _UTT_IDLE = object()
    _UTT_STARTED = object()
    _UTT_ENDED = object()

    def __init__(self, config=None):
        if config is None:
            config=Decoder.default_config()
        assert isinstance(config, Config)

        search_args_set = search_arguments_set(config)

        if len(search_args_set) == 0:
            # Use the language model by default if nothing else is set
            set_lm_path(config)
        elif len(search_args_set) > 1:
            raise ConfigError("more than one search argument was set in the"
                              " Config object")

        # Set the required config paths if they aren't already set
        if not (config.get_string("-hmm") and config.get_string("-dict")):
            set_hmm_and_dict_paths(config)

        self._speech_start_callback = None
        self._hypothesis_callback = None
        self._utterance_state = self._UTT_ENDED

        super(PocketSphinx, self).__init__(config)

    def process_audio(self, buf, no_search=False, full_utterance=False,
                      use_callbacks=True):
        """
        Process audio from an audio buffer using the :meth:`process_raw`
        decoder method.

        This method processes the given buffer with the :meth:`process_raw`
        decoder method, invoking :attr:`speech_start_callback` and
        :attr:`hypothesis_callback` when appropriate.

        :param buf: Audio buffer
        :param no_search: Whether to perform feature extraction, but no
            recognition yet (default: *False*).
        :param full_utterance: Whether this block of data contains a full
            utterance worth of data (default: *False*).  This may produce
            more accurate results.
        :param use_callbacks: Whether speech start and hypothesis callbacks
            should be called (default: *True*).
        :type buf: str
        :type no_search: bool
        :type full_utterance: bool
        :type use_callbacks: bool
        """
        if self.utt_ended:
            self.start_utt()

        self.process_raw(buf, no_search, full_utterance)

        # Note that get_in_speech moves idle -> started if returning True, so check
        # utt_idle before calling that method.
        was_idle = self.utt_idle

        # Check if we're in speech.
        in_speech = self.get_in_speech()

        # In speech and idle -> started transition just occurred.
        if in_speech and was_idle and self.utt_started:
            # Call speech start callback if it is set
            if use_callbacks and self.speech_start_callback:
                self.speech_start_callback()

        elif not in_speech and self.utt_started:
            # We're not in speech any more; utterance is over.
            self.end_utt()
            hyp = self.hyp()

            # Call the hypothesis callback if using callbacks and if it is set
            if use_callbacks and self.hypothesis_callback:
                self.hypothesis_callback(hyp)
            elif not use_callbacks:
                return hyp

    def batch_process(self, buffers, no_search=False, full_utterance=False,
                      use_callbacks=True):
        """
        Process a list of audio buffers and return the speech hypothesis or use the
        decoder callbacks if use_callbacks is True.
        """
        result = None
        for buf in buffers:
            if use_callbacks:
                self.process_audio(buf, no_search, full_utterance, use_callbacks)
            else:
                processing_result = self.process_audio(
                    buf, no_search, full_utterance, use_callbacks)
                if processing_result:  # this'll be the hypothesis
                    result = processing_result

        return result

    def get_in_speech(self):
        """
        Check if the last audio buffer contained speech.

        :returns: Whether the last audio buffer contained speech.
        :rtype: bool
        """
        in_speech = super(PocketSphinx, self).get_in_speech()

        # Move idle -> started to make utterance properties compatible with using
        # methods like process_raw instead of process_audio.
        if in_speech and self.utt_idle:
            self._utterance_state = self._UTT_STARTED
        return in_speech

    def start_utt(self):
        """
        Starts a new utterance if one is not already in progress.

        Does nothing if an utterance is already in progress.
        """
        if self.utt_ended:
            super(PocketSphinx, self).start_utt()
            self._utterance_state = self._UTT_IDLE

    def end_utt(self):
        """
        Ends the current utterance if one was in progress.

        Does nothing if no utterance is in progress.
        """
        if not self.utt_ended:
            super(PocketSphinx, self).end_utt()
            self._utterance_state = self._UTT_ENDED

    @property
    def utt_idle(self):
        """
        Whether an utterance is in progress, but no speech has been detected yet.

        :rtype: bool
        """
        # This property is True if get_in_speech() returns False.
        return self._utterance_state == self._UTT_IDLE

    @property
    def utt_started(self):
        """
        Whether an utterance is in progress and speech has been detected.

        :rtype: bool
        """
        # This property is True if get_in_speech() returns True.
        return self._utterance_state == self._UTT_STARTED

    @property
    def utt_ended(self):
        """
        Whether there is no utterance in progress.

        :rtype: bool
        """
        return self._utterance_state == self._UTT_ENDED

    # Alias utterance methods and properties
    end_utterance = end_utt
    start_utterance = start_utt
    utterance_started = utt_started
    utterance_idle = utt_idle
    utterance_ended = utt_ended

    def set_kws_list(self, name, kws_list):
        """
        Set a keyword-list search which, when active, scans input audio for
        keywords defined in the specified list or dictionary.

        :param name: Search name
        :param kws_list: Dictionary of words to threshold value.  Can also be a
            list of 2-tuples.
        :type name: str
        :type kws_list: list | dict
        """
        if not kws_list:
            return

        # If we get a list or tuple, turn it into a dict.
        if isinstance(kws_list, (list, tuple)):
            kws_list = dict(kws_list)

        # Get a new temporary file and write each words string and threshold value
        # on separate lines with the threshold value escaped with forward slashes.
        tf = tempfile.NamedTemporaryFile(mode="a", delete=False)
        for words, threshold in kws_list.items():
            tf.write("%s /%s/\n" % (words, float(threshold)))

        # Close the file and then set the search using the file's path.
        tf.close()
        self.set_kws(name, tf.name)

        # Delete the file manually.
        os.remove(tf.name)

    @property
    def active_search(self):
        """
        The name of the currently active Pocket Sphinx search.

        If the setter is passed a name with no matching Pocket Sphinx search,
        a ``RuntimeError`` will be raised.

        :rtype: str
        """
        return self.get_search()

    @active_search.setter
    def active_search(self, value):
        self.set_search(value)

    @property
    def speech_start_callback(self):
        """
        Function invoked when speech is first detected.

        To use this callback, set it to a callable that takes no arguments: ::

            ps = PocketSphinx()

            def callback():
                print("Speech started.")

            ps.speech_start_callback = callback

        To disable this callback, set it to ``None`` (default).
        """
        return self._speech_start_callback

    @speech_start_callback.setter
    def speech_start_callback(self, value):
        if not callable(value) and value is not None:
            raise TypeError("value must be callable or None")
        self._speech_start_callback = value

    @property
    def hypothesis_callback(self):
        """
        Function invoked when the decoder has finished processing speech.

        To use this callback, set it to a callable that takes one positional
        argument, the decoder's hypothesis: ::

            ps = PocketSphinx()

            def callback(hyp):
                print(hyp)

            ps.hypothesis_callback = callback

        To disable this callback, set it to ``None`` (default).
        """
        return self._hypothesis_callback

    @hypothesis_callback.setter
    def hypothesis_callback(self, value):
        if not callable(value) and value is not None:
            raise TypeError("value must be callable or None")
        self._hypothesis_callback = value
