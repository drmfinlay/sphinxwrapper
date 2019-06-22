from setuptools import setup


def get_long_description():
    with open('README.rst') as f:
        return f.read()


setup(
    name='sphinxwrapper',
    version='1.1.1',
    description='Alternative Python API for recognising speech with CMU '
                'Pocket Sphinx',
    long_description=get_long_description(),
    url='https://github.com/Danesprite/sphinxwrapper',
    author='Dane Finlay',
    author_email='Danesprite@posteo.net',
    license='MIT',
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Operating System :: OS Independent',
        'Topic :: Multimedia :: Sound/Audio :: Speech',
        'Topic :: Software Development :: Libraries',
    ],
    packages=['sphinxwrapper'],
    install_requires=['pocketsphinx']
)
