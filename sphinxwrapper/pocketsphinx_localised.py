"""
This module contains a `PocketSphinx` subclass that localises input and output words
between English language variants so that the CMU Sphinx US English models can be
used easily with non-US English.

One use of this is using the US English language model to output localised English.
"""

from .pocketsphinx_wrap import PocketSphinx
from pocketsphinx import Hypothesis, Config
from .localiser import *


class LocalisedPocketSphinx(PocketSphinx):
    """
    Pocket Sphinx decoder with custom localising for input and output words.

    `in_lang` is the English variant that words will localised to internally.
    `out_lang` is the variant that hypothesis strings will be in.
    """
    def __init__(self, in_lang, out_lang, cfg=PocketSphinx.default_config()):
        """
        in_lang and out_lang should be English language identifiers.
        :param in_lang: input language identifier - e.g. en_US, en_UK, etc
        :param out_lang: output language identifier
        :param cfg: PS config object
        """
        # Create localisers for the specified languages
        self.in_localiser = Localiser(in_lang)
        self.out_localiser = Localiser(out_lang)

        # TODO Localise any search that is set by the cfg: -kws, -keyphrase, -jsgf
        super(LocalisedPocketSphinx, self).__init__(cfg)

    def localise_from_config(self, cfg):
        """
        Create localised versions of grammar files and other specified search
        settings in a PS config object and modify file names as necessary.
        :param cfg: PS config object
        """
        assert isinstance(cfg, Config)
        key = "-keyphrase"
        if cfg.exists(key):
            localised = self.in_localiser.localise_words(cfg.get_string(key))
            cfg.set_string(key, localised)

        key = "-jsgf"
        if cfg.exists(key):
            # TODO This requires some parsing of grammar files to localise only the tokens.
            pass

        # TODO -fsg, -kws

    def hyp(self):
        result = super(LocalisedPocketSphinx, self).hyp()
        assert isinstance(result, Hypothesis)

        # If there is one, localise the hypothesis string with `out_localiser`.
        if result and result.hypstr:
            result.hypstr = self.out_localiser.localise_words(result.hypstr)
        return result
