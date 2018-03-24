from setuptools import setup

setup(name='sphinxwrapper',
      version='1.0',
      description='Simplified Python API for using Pocket Sphinx with pyaudio',
      author='Dane Finlay',
      author_email='Danesprite@gmail.com',
      packages=['sphinxwrapper'],
      install_requires=['pyaudio', 'pocketsphinx'],
      extras_require={
            "localise": ["aspell-python-py2;python_version<'3.0'",
                         "aspell-python-py3;python_version>='3.0'"]
      },
      )
