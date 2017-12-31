from setuptools import setup

setup(name='sphinxwrapper',
      version='1.0',
      description='Simplified Python API for using Pocket Sphinx with pyaudio',
      author='Dane Finlay',
      author_email='Danesprite@gmail.com',
      packages=['sphinxwrapper'],
      install_requires=['pyaudio', 'pocketsphinx']
      )
