#!/usr/bin/env python2

import atexit
import struct
import sys
import time
import wave

import portaudio

BUFFER_SIZE = 1024

wf = wave.open(sys.argv[1], 'rb')
atexit.register(wf.close)
nchannels, sampwidth, framerate, nframes, comptype, compname = wf.getparams()

def callback(in_list, out_list, time_info, user_data):
    frames = wf.readframes(len(out_list) / nchannels)
    if len(frames) < len(out_list):
        return portaudio.COMPLETE

    if sampwidth == 1:
        values = struct.unpack('%dB' % (len(frames) / nchannels), frames)
    elif sampwidth == 2:
        values = struct.unpack('%dh' % (len(frames) / nchannels), frames)

    for i in range(len(out_list)):
        out_list[i] = values[i]

    return portaudio.CONTINUE

portaudio.initialize()
atexit.register(portaudio.terminate)
stream = portaudio.open_default_stream(0, nchannels, portaudio.INT16,
                                       framerate, BUFFER_SIZE, callback, None)
stream.start()
while stream.is_active():
    time.sleep(1)
