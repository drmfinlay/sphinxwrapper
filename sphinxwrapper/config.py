"""
Useful Pocket Sphinx configuration functions
"""

from pocketsphinx import get_model_path, Config
from os import path, walk


class ConfigError(Exception):
    pass


def set_lm_path(config, model_path=get_model_path()):
    """
    This function will try to find the LM file in model_path and set the '-lm'
    argument for the given Config object. If the LM file couldn't be found, an error
    will be raised.
    :type config: Config
    :type model_path: str
    :raises: ConfigError
    """
    lm_file = None
    for (dir_path, _, file_names) in walk(model_path):
        for f in file_names:
            if f.endswith(".lm.bin"):  # LM found
                lm_file = path.join(dir_path, f)
                break

    if not lm_file:
        raise ConfigError("could not find the language model file in '%s'. Please "
                          "specify the '-lm' argument manually or use a different "
                          "model path" % model_path)

    config.set_string("-lm", lm_file)


def set_hmm_and_dict_paths(config, model_path=get_model_path()):
    """
    This function will try to find the HMM directory and dictionary file paths in
    model_path and set the '-hmm' and '-dict' arguments in the given Config object,
    or raise an error if any of the paths were not found.
    :type config: Config
    :type model_path: str
    :raises: ConfigError
    """
    dict_file = None
    hmm_dir = None
    hmm_required_files = [
        "feat.params", "mdef", "noisedict",
        "sendump", "transition_matrices", "variances"
    ]

    # Find the HMM directory and dictionary file within model_path
    for (dir_path, _, file_names) in walk(model_path):
        for f in file_names:
            if f.endswith(".dict"):  # dictionary found
                dict_file = path.join(dir_path, f)

        # Does this directory contain the HMM?
        valid = True
        for required_file in hmm_required_files:
            if required_file not in file_names:
                valid = False
                break

        if valid:  # HMM directory found
            hmm_dir = dir_path

    if not (hmm_dir and dict_file):
        raise ConfigError("could not find HMM directory and/or dictionary file in "
                          "'%s'. Please specify '-hmm' and '-dict' config arguments"
                          " manually or use a different model path" % model_path)
    config.set_string("-hmm", hmm_dir)
    config.set_string("-dict", dict_file)


search_arguments = ["-lm", "-jsgf", "-kws", "-keyphrase", "-fsg"]


def search_arguments_set(config):
    """
    The arguments in the search_arguments list that are set for a given Config
    object.
    :type config: Config
    :return: list
    """
    result = []
    for arg in search_arguments:
        if config.get_string(arg):
            result.append(arg)
    return result
