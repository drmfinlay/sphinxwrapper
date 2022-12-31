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
Decoder Configuration
----------------------------------------------------------------------------

"""

from os import path, walk

from pocketsphinx import get_model_path, Config


class ConfigError(Exception):
    """
    Error raised if something is wrong with the decoder configuration.
    """

#: List of acceptable file extensions for language model files.
lm_file_extensions = [".lm", ".lm.bin"]

#: List of acceptable file extensions for pronunciation dictionary files.
dict_file_extensions = [".dic", ".dict"]

#: List of files that must be present in a hidden Markov model directory.
hmm_dir_files = ["feat.params", "mdef", "noisedict", "transition_matrices",
                 "variances"]

search_arguments = ["-lm", "-fsg", "-jsgf", "-keyphrase", "-kws"]


def set_lm_path(config, model_path=None):
    """
    This function will try to find the language model file under *model_path*,
    set the '-lm' argument for the given ``Config`` object and return ``True``.
    If the argument is already set, its value is not changed and the function
    returns ``False``.

    By default, only files ending with *.lm* or *.lm.bin* will be considered.

    An error will be raised if the path was not found.

    :param config: Decoder configuration object
    :param model_path: Path to search for the language model file.
        The Pocket Sphinx :meth:`get_model_path()` function is used if this
        parameter is unspecified.
    :type config: Config
    :type model_path: str
    :raises: ConfigError
    :returns: Whether any configuration argument was changed.
    :rtype: bool
    """
    lm_file = config.get_string("-lm")
    if lm_file: return False

    # Use get_model_path() if *model_path* was not specified.
    if model_path is None: model_path = get_model_path()

    # Convert *model_path* to an absolute path, if necessary.
    if model_path and not path.isabs(model_path):
        model_path = path.abspath(model_path)

    # Find a language model file within *model_dir*.
    for (dir_path, _, file_names) in walk(model_path):
        for f in file_names:
            if any([f.endswith(ext) for ext in lm_file_extensions]):
                # Language model file found.
                lm_file = path.join(dir_path, f)
                break

    if not lm_file:
        raise ConfigError("Could not find the language model file under %r. "
                          " Please specify the '-lm' argument manually or use"
                          "  a different model path" % model_path)

    config.set_string("-lm", lm_file)
    return True


def set_hmm_and_dict_paths(config, model_path=None):
    """
    This function will try to find a hidden Markov model (HMM) directory and
    a pronunciation dictionary file under *model_path*.  If found, the function
    sets the '-hmm' and '-dict' arguments in the given ``Config`` object
    appropriately and returns ``True``.  If an argument is already set, its
    value is not changed.  If both arguments were already set, the function
    returns ``False``.

    By default, this function requires an HMM directory to contain each of the
    following files:

     * *feat.params*
     * *mdef*
     * *noisedict*
     * *transition_matrices*
     * *variances*

    Likewise, the pronunciation dictionary file must, by default, have the file
    extension *.dic* or *.dict* in order for this function to use it.

    An error will be raised if the function finishes without both arguments set
    in the ``Config`` object.

    :param config: Decoder configuration object
    :param model_path: Path to search for the HMM directory and pronunciation
        dictionary file.
        The Pocket Sphinx :meth:`get_model_path()` function is used if this
        parameter is unspecified.
    :type config: Config
    :type model_path: str
    :raises: ConfigError
    :returns: Whether any configuration argument was changed.
    :rtype: bool
    """
    dict_file = config.get_string("-dict")
    hmm_dir = config.get_string("-hmm")
    if dict_file and hmm_dir: return False

    # Use get_model_path() if *model_path* was not specified.
    if model_path is None:  model_path = get_model_path()

    # Convert *model_path* to an absolute path, if necessary.
    if model_path and not path.isabs(model_path):
        model_path = path.abspath(model_path)

    # Find the HMM directory and pronunciation dictionary file within
    # *model_path*.
    for (dir_path, _, file_names) in walk(model_path):
        if dict_file is None:
            for f in file_names:
                if any([f.endswith(ext) for ext in dict_file_extensions]):
                    # Dictionary file found.
                    dict_file = path.join(dir_path, f)

        if hmm_dir is None:
            # Does this directory contain the HMM?
            valid_hmm = True
            for required_file in hmm_dir_files:
                if required_file not in file_names:
                    valid_hmm = False
                    break
            if valid_hmm: hmm_dir = dir_path

    if not (hmm_dir and dict_file):
        raise ConfigError("Could not find HMM directory and/or dictionary file"
                          " under %r.  Please specify '-hmm' and '-dict' config"
                          " arguments manually or use a different model path"
                          % model_path)

    config.set_string("-hmm", hmm_dir)
    config.set_string("-dict", dict_file)
    return True


def search_arguments_set(config):
    """
    Get a list of search arguments set for a given ``Config`` object.

    Search arguments include:

     * '-lm' (default)
     * '-fsg'
     * '-jsgf'
     * '-keyphrase'
     * '-kws'

    :param config: Decoder configuration object
    :type config: Config
    :return: list
    """
    result = []
    for arg in search_arguments:
        if config.get_string(arg): result.append(arg)
    return result
