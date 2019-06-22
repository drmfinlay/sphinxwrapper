============
Introduction
============

Alternative Python API for recognising speech with CMU Pocket Sphinx

This package aims to provide a simple API for recognising speech using the
Pocket Sphinx API. Pocket Sphinx is an open source, lightweight speech
recognition engine. You can read more about the CMU Sphinx speech
recognition projects `here <https://cmusphinx.github.io/wiki/>`__.

There are some usage examples in the repository's `examples folder`_.


Installation & dependencies
---------------------------

To install this package via pip, run the following command:

.. code:: shell

   pip install sphinxwrapper

If you are installing in order to *develop* sphinxwrapper, clone/download
the repository, move to the root directory and run:

.. code:: shell

   pip install -e .

Either of the above commands will also install the required
`pocketsphinx-python`_ package.

The usage examples for ``sphinxwrapper`` require the cross-platform
`pyaudio`_ Python package. It can be installed by running the following:


.. code:: shell

   pip install pyaudio

Usage example
-------------

The following is a simple usage example showing how to use the
``sphinxwrapper`` package to make Pocket Sphinx continuously recognise
speech from the microphone using the default decoder configuration.

..  code:: python

    import time

    from pyaudio import PyAudio, paInt16

    from sphinxwrapper import PocketSphinx

    # Set up a Pocket Sphinx decoder with the default config
    ps = PocketSphinx()

    # Set up and register a callback function to print Pocket Sphinx's
    # hypothesis for  recognised speech
    def print_hypothesis(hyp):
        # Get the hypothesis string from the Hypothesis object or None, then
        # print it
        speech = hyp.hypstr if hyp else None
        print("Hypothesis: %s" % speech)

    ps.hypothesis_callback = print_hypothesis

    # Decode from the default audio input device continously.
    p = PyAudio()
    stream = p.open(format=paInt16, channels=1, rate=16000, input=True,
                    frames_per_buffer=2048)
    while True:
        ps.process_audio(stream.read(2048))
        time.sleep(0.1)



Python versions
---------------

This package has been written for Python 2.7 and above. It should work
exactly the same way for each supported version. please file an issue if you
come across any problems that are specific to the Python version you're
using.

Future CMU Sphinx API changes
-----------------------------

As the CMU Sphinx libraries are pre-alpha, there may be future changes that
break this package in some way. I'm happy to fix any such issues, just file
an issue.

Documentation
-------------

The documentation for this project is written in `reStructuredText`_ and
built using the `Sphinx documentation engine`_.

Run the following in the repository folder to build it locally::

  cd docs
  pip install -r requirements.txt
  make html


.. Links.
.. _Sphinx documentation engine: http://www.sphinx-doc.org/en/stable
.. _examples folder: https://github.com/Danesprite/sphinxwrapper/tree/master/examples
.. _pocketsphinx-python: https://github.com/bambocher/pocketsphinx-python
.. _pyaudio: http://people.csail.mit.edu/hubert/pyaudio/
.. _reStructuredText: http://docutils.sourceforge.net/rst.html
