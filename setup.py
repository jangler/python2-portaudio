from distutils.core import setup, Extension

module1 = Extension('portaudio', sources=['portaudio.c'],
                    libraries=['portaudio'])

setup(name='portaudio', ext_modules=[module1])
