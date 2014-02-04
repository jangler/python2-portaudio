from distutils.core import setup, Extension

module = Extension('portaudio', sources=['portaudio.c'],
                    libraries=['portaudio'])

setup(name='portaudio', ext_modules=[module])
