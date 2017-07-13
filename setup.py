from distutils.core import setup, Extension

module1 = Extension('sphinxwrapper',
                    sources = [
                        'sphinxwrapper.c'
                    ],
                    include_dirs = [
                        '/usr/local/include',
                        '/usr/local/include/sphinxbase',
                        '/usr/local/include/pocketsphinx'
                    ],
                    libraries = [
                        'pocketsphinx',
                        'sphinxbase',
                        'sphinxad'
                    ],
                    library_dirs = ['/usr/local/lib']
)

setup (name = 'SphinxWrapper',
       version = '0.1',
       description = 'C extension for Pocket Sphinx',
       ext_modules = [module1])
