from distutils.core import setup, Extension

module1 = Extension('sphinxwrapper',
                    sources = [
                        'sphinxwrapper.c',
                        'pypocketsphinx.c',
                        'audio.c',
                        'pyutil.c'
                    ],
                    include_dirs = [
                        'include',
                        '/usr/local/include',
                        '/usr/local/include/sphinxbase',
                        '/usr/local/include/pocketsphinx',
                        '/usr/include',
                        '/usr/include/sphinxbase',
                        '/usr/include/pocketsphinx'

                    ],
                    libraries = [
                        'pocketsphinx',
                        'sphinxbase',
                        'sphinxad'
                    ],
                    library_dirs = ['/usr/local/lib']
)

setup (name = 'SphinxWrapper',
       version = '1.1',
       description = 'Python C extension for Pocket Sphinx and '
       'other CMU Sphinx libraries',
       ext_modules = [module1])
