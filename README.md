# sphinxwrapper
Python C extension for Pocket Sphinx and other CMU Sphinx libraries

This is an simplified alternative to the SWIG Python modules for 
[CMU Pocket Sphinx](https://github.com/cmusphinx/pocketsphinx) and 
[Sphinx base](https://github.com/cmusphinx/sphinxbase). You can read more about the CMU Sphinx speech recognition projects [here](https://cmusphinx.github.io/wiki/).

This extension doesn't have all of the functionality that the offical SWIG Python modules do, but the [examples](examples/) show what it does have. Although most of the `PocketSphinx` type methods are used, some haven't been put into the examples yet.

## Installing dependencies
This project has the following library dependencies:
- [sphinxbase](https://github.com/cmusphinx/sphinxbase)
- [pocketsphinx](https://github.com/cmusphinx/pocketsphinx)

I've tested both *sphinxbase* and *pocketsphinx* using version **0.8+5prealpha-3**. It is best to match the versions for these 2 dependencies.

### Debian GNU/Linux
Install the CMU Sphinx dependencies on Debian using the following:
``` Shell
sudo apt install libpocketsphinx-dev libpocketsphinx3 libsphinxbase-dev libsphinxbase3 pocketsphinx pocketsphinx-en-us
```

**Note**: `pocketsphinx-en-us` contains both models (LM and HMM) and the pronunciation dictionary for US English. Other models and dictionaries are available [from sourceforge](https://sourceforge.net/projects/cmusphinx/files/Acoustic%20and%20Language%20Models/). You can also build your own language model. There is information on that [here](https://cmusphinx.github.io/wiki/tutoriallm/#language-models), although it is rather involved.

You'll also need to install the dev package for the version of Python you want to use.

``` Shell
sudo apt install python-dev
sudo apt install python3-dev
```

### Compiling dependencies from source
If the dependencies aren't available from your system's package management system, you can download and compile the sources from the repositories linked above and follow the instructions there. 

**Note**: both dependencies by default require *swig*, but you shouldn't need to install it at all because the swig modules for the dependencies are not used. You can use `./autogen.sh --without-python` to configure both dependencies to compile without swig.

#### Windows
Compiling CMU Sphinx libraries on Windows is a little different. There are specific instructions for Pocket Sphinx (here)[https://github.com/cmusphinx/pocketsphinx#ms-windows-ms-visual-studio-2010-or-newer---we-test-with-vc-2010-express] and Sphinx base (here)[https://github.com/cmusphinx/sphinxbase#ms-windows-installation].

If you want to use Cygwin, the audio implementation currently doesn't work, so you won't be able to build *sphinxwrapper* without modifications to *sphinxbase*.

## Install the sphinxwrapper Python Extension
Clone or download this repository and run the following:
``` Shell 
python setup.py install
```

## Usage example
The following is a simple usage example showing how to use the `sphinxwrapper` module to make Pocket Sphinx continuously recognise from the default microphone using the default configuration.
``` Python
from sphinxwrapper import AudioDevice, PocketSphinx
import time

# Set up a Pocket Sphinx decoder with the default config
ps = PocketSphinx()

# Set up and register a callback function to print
# Pocket Sphinx's hypothesis for recognised speech
def print_hypothesis(hyp):
    print("Hypothesis: %s" % hyp)

ps.hypothesis_callback = print_hypothesis

# Have it decode from the default audio input device
# continously
ad = AudioDevice()
ad.open()
ad.record()
while True:
    audio = ad.read_audio()
    ps.process_audio(audio)
    time.sleep(0.1)

```

## Notes
### Python versions
This extension module has been written for Python 2.7 and above. For the most part, it should work exactly the same way for all supported versions. If you come across any problems that are specific to the Python version you're using, please file an issue.

### Future CMU Sphinx API changes
There will probably be changes to CMU Sphinx libraries such as Pocket Sphinx and sphinxbase that break this module in the future in some way. I'm happy to fix any such issues, just file an issue.
