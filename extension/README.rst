Old C Extension module
============================================================================

This project used to be in the form of a Python module written in C
(i.e. a `Python C extension`_). The reason for this is that the CMU Pocket
Sphinx and sphinxbase libraries are both written in C and I developed
*sphinxwrapper* to abstract some of functionality provided by the
`pocketsphinx_continuous`_ program so I could use it in my
`Pocket Sphinx dragonfly engine`_.

The *sphinxwrapper* package now interacts with CMU Pocket Sphinx via the
`pocketsphinx-python`_ package instead, so compiling isn't necessary any
more.

The C extension version still exists in the repository's *extension* folder
for historical reasons. It is **not recommended** that you use this version
for various reasons.

The API is similar to the current version with some differences:

 * It uses the platform's *sphinxbase* audio device implementation for
   reading data from the microphone and cannot be used with *pyaudio*
   without some hacking.

 * Some functions and properties behave differently or just don't exist.

 * Most of the classes, functions and methods provided by the
   `pocketsphinx-python`_ package are not present.

Writing the C extension was mostly a learning experience for me. Python
modules written in C are dreadfully difficult to write and maintain, plus
the CMU Sphinx libraries used already have very good Python compatibility
through `Swig`_.

Like the current version of *sphinxwrapper*, the C extension should work
correctly with Python versions 2.7.x and 3.x.

I might update it at some point to use the same API as the current version.

Compiling & Installing
----------------------

To compile this version of *sphinxwrapper*, you need to have the following
dependencies installed:

 * `sphinxbase`_
 * `pocketsphinx`_
 * Python C headers

You can install the dependencies using the following commands on Debian
GNU/Linux. They might also work on Debian-derived systems such as Ubuntu and
will be similar on other systems.

I've tested this with *pocketsphinx* and *sphinxbase* versions
**0.8+5prealpha-3**.

.. code:: shell

   # Install the required library headers
   sudo apt install libpocketsphinx-dev libpocketsphinx3 libsphinxbase-dev libsphinxbase3

   # Install the Python headers for your version of Python
   sudo apt install python-dev
   sudo apt install python3-dev

You can compile and install the required CMU Sphinx libraries from source
instead. Have a look at the project for information

You probably also want to install the ``pocketsphinx-en-us`` package for the
default models and dictionary, unless you specify the *-hmm*, *-dict* and
other arguments (e.g. *-lm*) yourself.

You will also need the C/C++ compiler that your version of Python was
compiled with. Normally this is GCC on Linux or Clang on Mac OS/\*BSD.
You can identify the required compiler by starting an interpreter.
For example, my Debian 9 machine has Python 2.7.13 comiled with GCC v6.3.0:

.. code:: shell

  Python 2.7.13 (default, Sep 26 2018, 18:42:22) 
  [GCC 6.3.0 20170516] on linux2

You should be able to compile and install it by running the following in
the repository's *extension* folder:

.. code:: shell

   cd extension
   python setup.py install

I will probably not distribute this version of *sphinxwrapper* on PyPI.

Microsoft Windows
^^^^^^^^^^^^^^^^^

I would not recommend using this version of *sphinxwrapper* on Windows
because compiling Python C/C++ extensions on that platform is not easy. It
should work using ``python setup.py install``, but you'll probably need to
download and install a few things first. The Python wiki `page on Windows
Compilers <https://wiki.python.org/moin/WindowsCompilers>`__ should help.

Please note that you will likely have no luck getting this working on
*Cygwin* because *libsphinxbasead* won't compile in that environment, at
least it didn't the last time I tried. You're better off using a virtual
machine with a Linux distribution if you're really determined to use this on
Windows.

Usage example
-------------

As mentioned above, there are a few differences with this version of the
package. The below usage example should work. It is similar to the
`pocketsphinx_continuous`_ program.

..  code:: python

    import os
    import time

    from sphinxwrapper import PocketSphinx, AudioDevice


    def speech_start_callback():
        print("Speech started.")


    def hyp_callback(s):
        print("Hypothesis: %s" % s)


    # Initialise a decoder with the default configuration.
    # ps = PocketSphinx()

    # The decoder optionally accepts an command-line argument list for
    # decoder configuration. The following will suppress log output.
    ps = PocketSphinx(["-logfn", os.devnull])

    # Set up callback functions.
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hyp_callback

    # Recognise from the mic in a loop.
    ad = AudioDevice()
    ad.open()
    ad.record()
    while True:
        audio = ad.read_audio()
        ps.process_audio(audio)
        time.sleep(0.1)


.. Links.
.. _Pocket Sphinx dragonfly engine: https://dragonfly2.readthedocs.io/en/latest/sphinx_engine.html
.. _Python C extension: https://docs.python.org/3/extending/extending.html
.. _Swig: http://http://www.swig.org
.. _pocketsphinx-python: https://github.com/bambocher/pocketsphinx-python
.. _pocketsphinx: https://github.com/cmusphinx/pocketsphinx
.. _pocketsphinx_continuous: https://github.com/cmusphinx/pocketsphinx/blob/master/src/programs/continuous.c
.. _sphinxbase: https://github.com/cmusphinx/sphinxbase

