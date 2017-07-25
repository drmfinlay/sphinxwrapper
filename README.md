# pocketsphinx-c-python
Python C extension for Pocket Sphinx and other CMU Sphinx libraries for interaction with [Dragonfly](https://github.com/t4ngo/dragonfly)

This is a work in progress, but this project aims to create a backend for using Pocket Sphinx with 
[Dragonfly](https://github.com/t4ngo/dragonfly) for the excellent macroing support it offers. 
The Pocket Sphinx engine backend for dragonfly is in 
[my fork of Dragonfly](https://github.com/Danesprite/dragonfly). 


## Installing dependencies
This project has the following library dependencies:
- [sphinxbase](https://github.com/cmusphinx/sphinxbase)
- [pocketsphinx](https://github.com/cmusphinx/pocketsphinx)

### Debian GNU/Linux
Install the dependencies on Debian using the following:
``` Shell
$ sudo apt install pocketsphinx sphinxbase-utils
```

### Compiling dependencies from source
If the dependencies aren't available from your system's package management system, you can download and 
compile the sources from repositories linked above and follow the instructions there.


## Install the sphinxwrapper Python Extension
Clone or download this repository and run the following:
``` Shell 
python setup.py install
```


## Usage example
The following is a simple usage example showing how to use the `sphinxwrapper` module to make 
Pocket Sphinx continuously recognize from the default microphone using the default configuration.
``` Python
from sphinxwrapper import PocketSphinx
import time

# Set up a Pocket Sphinx decoder with the default config
ps = PocketSphinx()

# Set up and register a callback function to print
# Pocket Sphinx's hypothesis for recognized speech
def print_hypothesis(hyp):
    print("Hypothesis: %s" % hyp)

ps.hypothesis_callback = print_hypothesis

# Have it decode from the default audio input device
# continously
ps.open_rec_from_audio_device()
ps.start_utterance()
while True:
    ps.read_and_process_audio()
    time.sleep(0.1)

```
