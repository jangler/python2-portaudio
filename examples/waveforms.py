#!/usr/bin/env python2

from atexit import register
from math import sin, pi
from random import random
from time import sleep

import portaudio

SAMPLE_RATE = 44100
BUFFER_SIZE = 256
NOTE_FREQUENCY = 440.0

saw_wave = lambda x: x % 2 - 1
sine_wave = lambda x: sin(x * pi)
square_wave = lambda x: int(x % 2) * 2 - 1
white_noise = lambda x: random()

def callback(in_list, out_list, time_info, user_data):
    if user_data[0] < NOTE_FREQUENCY:
        func = saw_wave
    elif user_data[0] < NOTE_FREQUENCY * 2:
        func = sine_wave
    elif user_data[0] < NOTE_FREQUENCY * 3:
        func = square_wave
    else:
        func = white_noise

    for i in range(0, len(out_list), 2):
        user_data[0] += NOTE_FREQUENCY / SAMPLE_RATE
        out_list[i] = func(user_data[0])
        out_list[i+1] = func(user_data[0])

    return portaudio.CONTINUE

portaudio.initialize()
register(portaudio.terminate)
stream = portaudio.open_default_stream(0, 2, portaudio.FLOAT32, SAMPLE_RATE,
                                       BUFFER_SIZE, callback, [0.0])
stream.start()
sleep(4)
stream.abort()
