"""
Setup file for the sphinxwrapper C extension module.

This is here mostly for historical reasons. You probably want to install the pure
Python version that uses the CMU Sphinx Swig modules instead.
"""

from setuptools import setup, Extension

module1 = Extension('sphinxwrapper',
                    sources=[
                        'src/sphinxwrapper.c',
                        'src/pypocketsphinx.c',
                        'src/audio.c',
                        'src/pyutil.c'
                    ],
                    include_dirs=[
                         'include',
                         '/usr/local/include',
                         '/usr/local/include/sphinxbase',
                         '/usr/local/include/pocketsphinx',
                         '/usr/include',
                         '/usr/include/sphinxbase',
                         '/usr/include/pocketsphinx'
                    ],
                    libraries=[
                         'pocketsphinx',
                         'sphinxbase',
                         'sphinxad'
                    ],
                    library_dirs=['/usr/local/lib']
                    )

setup(name='sphinxwrapper',
      version='0.1',
      description='Simplified Python API for using Pocket Sphinx',
      author='Dane Finlay',
      author_email='Danesprite@gmail.com',
      ext_modules=[module1]
      )
