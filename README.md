# sphinxwrapper
Simplified Python API for using Pocket Sphinx with pyaudio

This package aims to provide a simplified way of interacting with the CMU Pocket Sphinx Python API. Pocket Sphinx is an open source, lightweight speech recognition engine. You can read more about the CMU Sphinx speech recognition projects [here](https://cmusphinx.github.io/wiki/).

The `pyaudio` package provides Python bindings for [portaudio](http://www.portaudio.com/), a cross-platform audio I/O library, and can be used to allow portable interaction with Pocket Sphinx if you want to use a microphone for continuous speech recognition.

There are examples [here](examples/) showing a few different use cases for `sphinxwrapper`.

## Installing dependencies
This project has the following dependencies:
- [pocketsphinx](https://github.com/cmusphinx/pocketsphinx)
- [sphinxbase](https://github.com/cmusphinx/sphinxbase) (required for *pocketsphinx*)
- [pyaudio](http://people.csail.mit.edu/hubert/pyaudio/) (if you want to use a microphone)

I've tested both *pocketsphinx* and *sphinxbase* using version **0.8+5prealpha-3**. It is best to match the versions for these 2 dependencies.

### Debian GNU/Linux
Install the CMU Sphinx dependencies on Debian using the following:
``` Shell
sudo apt install libpocketsphinx3 libsphinxbase3 pocketsphinx pocketsphinx-en-us
```

**Note**: `pocketsphinx-en-us` contains both models (LM and HMM) and the pronunciation dictionary for US English. Other models and dictionaries are available [from sourceforge](https://sourceforge.net/projects/cmusphinx/files/Acoustic%20and%20Language%20Models/). You can also build your own language model. There is information on that [here](https://cmusphinx.github.io/wiki/tutoriallm/#language-models), although it is rather involved.

### Compiling dependencies from source
If the dependencies aren't available from your system's package management system, you can download and compile the sources from the repositories linked above and follow the instructions there. 

#### Windows
Compiling CMU Sphinx libraries on Windows is a little different. There are specific instructions for Pocket Sphinx [here](https://github.com/cmusphinx/pocketsphinx#ms-windows-ms-visual-studio-2010-or-newer---we-test-with-vc-2010-express) and Sphinx base [here](https://github.com/cmusphinx/sphinxbase#ms-windows-installation).

## Install the sphinxwrapper Python package
Clone or download this repository and run the following:
``` Shell 
python setup.py install
```

## Usage example
The following is a simple usage example showing how to use the `sphinxwrapper` package to make Pocket Sphinx continuously recognise using the default configuration from the default microphone using `pyaudio`.
``` Python
from sphinxwrapper import PocketSphinx
from pyaudio import PyAudio, paInt16
import time

# Set up a Pocket Sphinx decoder with the default config
ps = PocketSphinx()

# Set up and register a callback function to print Pocket Sphinx's hypothesis for 
# recognised speech
def print_hypothesis(hyp):
    # Get the hypothesis string from the Hypothesis object or None, then print it
    speech = hyp.hypstr if hyp else None
    print("Hypothesis: %s" % speech)

ps.hypothesis_callback = print_hypothesis

# Have it decode from the default audio input device continously
p = PyAudio()
stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                frames_per_buffer=2048)
while True:
    ps.process_audio(stream.read(2048))
    time.sleep(0.1)

```

## Notes
### Python versions
This package has been written for Python 2.7 and above. For the most part, it should work exactly the same way for all supported versions. If you come across any problems that are specific to the Python version you're using, please file an issue.

### Future CMU Sphinx API changes
There might be changes to CMU Sphinx libraries in the future that break this package in some way. I'm happy to fix any such issues, just file an issue.

### Python C Extension module
This project used to be in the form of a Python C extension module. That version of `sphinxwrapper` is in the [extension](extension/) folder.

At the time of writing, the simplistic Python audio API that the extension module uses is not available from the *sphinxbase* Python Swig modules - I used the *sphinxbase* C audio API directly. The only reason to use the C extension version is in the case that `pyaudio` doesn't work for you.
