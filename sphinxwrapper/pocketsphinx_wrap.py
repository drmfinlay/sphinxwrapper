"""
This module contains a Pocket Sphinx Decoder subclass with some simplified methods
and properties.
"""

from pocketsphinx import Decoder, Config
from .config import set_hmm_and_dict_paths, search_arguments_set, set_lm_path,\
    ConfigError


class PocketSphinxError(Exception):
    pass


class PocketSphinx(Decoder):
    """
    Pocket Sphinx decoder subclass with processing methods that provide callback
    functionality, amongst other things.

    This class will try to set required config options, such as '-hmm', '-dict'
    and/or '-lm', in the config object automatically if they are not set.
    """
    # Internal values used in process_audio to keep track of the utterance state
    # between method calls
    _UTT_IDLE = object()
    _UTT_STARTED = object()
    _UTT_ENDED = object()

    def __init__(self, config=Decoder.default_config()):
        assert isinstance(config, Config)

        search_args_set = search_arguments_set(config)

        if len(search_args_set) == 0:
            # Use the language model by default if nothing else is set
            set_lm_path(config)
        elif len(search_args_set) > 1:
            raise ConfigError("more than one search argument was set in the Config "
                              "object")

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
        Process audio from an audio buffer using the decoder's process_raw method and
        call the speech start and hypothesis callbacks if and when necessary.
        """
        if self._utterance_state == self._UTT_ENDED:
            self.start_utt()
            self._utterance_state = self._UTT_IDLE

        self.process_raw(buf, no_search, full_utterance)
        in_speech = self.get_in_speech()

        if in_speech and self._utterance_state == self._UTT_IDLE:
            # Utterance has now started
            self._utterance_state = self._UTT_STARTED

            # Call speech start callback if it is set
            if use_callbacks and self.speech_start_callback:
                self.speech_start_callback()

        elif not in_speech and self._utterance_state == self._UTT_STARTED:
            # We're not in speech any more; utterance is over.
            self.end_utt()
            self._utterance_state = self._UTT_ENDED
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

    def end_utterance(self):
        """
        Ends the current utterance if one was in progress.
        This method is useful for resetting processing of audio via the
        process_audio method. It will *not* raise an error if no utterance was in
        progress.
        """
        if self._utterance_state != self._UTT_ENDED:
            self.end_utt()
            self._utterance_state = self._UTT_ENDED

    @property
    def active_search(self):
        """
        The name of the currently active Pocket Sphinx search.
        If the setter is passed a name with no matching Pocket Sphinx search, an
        error will be raised.
        :return: str
        """
        return self.get_search()

    @active_search.setter
    def active_search(self, value):
        self.set_search(value)

    @property
    def speech_start_callback(self):
        """
        Callback for when speech starts.
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
        Callback called with Pocket Sphinx's hypothesis for what was said.
        """
        return self._hypothesis_callback

    @hypothesis_callback.setter
    def hypothesis_callback(self, value):
        if not callable(value) and value is not None:
            raise TypeError("value must be callable or None")
        self._hypothesis_callback = value
