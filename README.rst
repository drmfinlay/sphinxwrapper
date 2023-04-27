=============================
Introduction to sphinxwrapper
=============================

|Docs Status|

Simplified Python API for the CMU Pocket Sphinx speech recogniser

This package provides a simple API for recognising speech using CMU Pocket
Sphinx, an open source, lightweight speech recognition engine.  More information
on CMU Pocket Sphinx, and other CMU speech recognition libraries, may be found
at `cmusphinx.github.io <https://cmusphinx.github.io>`__.

There are some usage examples in the repository's `examples folder`_
demonstrating how to use this library to scan and process speech audio from
a microphone.  Each of these examples require the `PyAudio`_ package, which
may be installed by running the following command:

.. code:: shell

   pip install pyaudio


Installation & dependencies
---------------------------

To install this package via pip, run the following command:

.. code:: shell

   pip install sphinxwrapper

If you are installing in order to *develop* sphinxwrapper, clone/download
the repository, move to the root directory and run:

.. code:: shell

   pip install -e .

Either of the above commands will also install version 0.1.15 of the required
`pocketsphinx-python`_ package.

This library does not currently support Python version 3.10 or
above on Windows.


Usage example
-------------

The following example demonstrates how to use ``sphinxwrapper`` and `PyAudio`_
to scan and interpret audio from the microphone using the default language model
and dictionary.

..  code:: python

    import os
    import time

    from pyaudio import PyAudio, paInt16

    from sphinxwrapper import PocketSphinx, DefaultConfig

    # Initialise a Pocket Sphinx decoder with the default configuration.
    config = DefaultConfig()
    config.set_string("-logfn", os.devnull)  # Suppress log output.
    ps = PocketSphinx(config)

    # Define decoder callback functions.
    def speech_start_callback():
        print("Speech started.")

    def hypothesis_callback(hyp):
        hypstr = hyp.hypstr if hyp else None
        print("Hypothesis: %r" % hypstr)

    # Set decoder callback functions.
    ps.speech_start_callback = speech_start_callback
    ps.hypothesis_callback = hypothesis_callback

    # Open an audio stream on the default input audio device.
    p = PyAudio()
    stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                    frames_per_buffer=2048, input_device_index=None)
    stream.start_stream()

    # Recognise from the microphone in a loop until interrupted.
    try:
        print("Listening... Press Ctrl+C to exit...")
        while True:
            ps.process_audio(stream.read(2048))
            time.sleep(0.1)
    except KeyboardInterrupt:
        stream.stop_stream()
        p.terminate()


Python versions
---------------

This package has been written for Python 2.7 and above.  It should work the same
way for each supported version.  Please file an issue if you encounter a problem
specific to the Python version you're using.


Documentation
-------------

The documentation for this project is written in `reStructuredText`_ and
built using the `Sphinx documentation engine`_.

Run the following commands in the repository folder to build it locally::

  cd docs
  pip install -r requirements.txt
  make html


.. Links.
.. _Sphinx documentation engine: http://www.sphinx-doc.org/en/stable
.. _examples folder: https://github.com/Danesprite/sphinxwrapper/tree/master/examples
.. _pocketsphinx-python: https://github.com/bambocher/pocketsphinx-python
.. _PyAudio: http://people.csail.mit.edu/hubert/pyaudio/
.. _reStructuredText: http://docutils.sourceforge.net/rst.html
.. |Docs Status| image::
   https://readthedocs.org/projects/sphinxwrapper/badge/?version=latest&style=flat
   :target: https://sphinxwrapper.readthedocs.io
