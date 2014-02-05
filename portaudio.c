#include <Python.h>
#include <structmember.h>
#include <portaudio.h>

static PyObject *PortAudioError;

/* Stream (PaStream) */

typedef struct {
    PyObject_HEAD
    PaStream *stream;
} Stream;

static void Stream_dealloc(Stream* self) {
    Py_XDECREF(self->stream);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *Stream_close(Stream *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaError err;
    err = Pa_CloseStream(self->stream);
    if (err != paNoError) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *Stream_start(Stream *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaError err;
    err = Pa_StartStream(self->stream);
    if (err != paNoError) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *Stream_stop(Stream *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaError err;
    err = Pa_StopStream(self->stream);
    if (err != paNoError) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *Stream_abort(Stream *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaError err;
    err = Pa_AbortStream(self->stream);
    if (err != paNoError) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *Stream_is_active(Stream *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaError err = Pa_IsStreamActive(self->stream);
    if (err == 1) {
        Py_INCREF(Py_True);
        return Py_True;
    } else if (err == paNoError) {
        Py_INCREF(Py_False);
        return Py_False;
    } else {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }
}

static PyObject *Stream_is_stopped(Stream *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaError err = Pa_IsStreamStopped(self->stream);
    if (err == 1) {
        Py_INCREF(Py_True);
        return Py_True;
    } else if (err == paNoError) {
        Py_INCREF(Py_False);
        return Py_False;
    } else {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }
}

static PyObject *Stream_get_time(Stream *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PyObject *result = PyFloat_FromDouble(Pa_GetStreamTime(self->stream));
    Py_INCREF(result);
    return result;
}

static PyObject *Stream_get_cpu_load(Stream *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PyObject *result = PyFloat_FromDouble(Pa_GetStreamCpuLoad(self->stream));
    Py_INCREF(result);
    return result;
}

static PyObject *Stream_get_info(Stream *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    const PaStreamInfo *info = Pa_GetStreamInfo(self->stream);
    PyObject *result = Py_BuildValue("fff", info->inputLatency,
                                     info->outputLatency, info->sampleRate);
    Py_INCREF(result);
    return result;
}

static PyMethodDef Stream_methods[] = {
    {"close", (PyCFunction)Stream_close, METH_VARARGS,
     "stream.close()\n\n"
     "Close the audio stream. If the audio stream is active, it discards\n"
     "any pending buffers as if stream.abort() had been called. May\n"
     "raise portaudio.Error."},
    {"start", (PyCFunction)Stream_start, METH_VARARGS,
     "stream.start()\n\n"
     "Commence audio processing. May raise portaudio.Error."},
    {"stop", (PyCFunction)Stream_stop, METH_VARARGS,
     "stream.stop()\n\n"
     "Terminate audio processing, waiting until all pending audio\n"
     "buffers have been played before returning. May raise\n"
     "portaudio.Error."},
    {"abort", (PyCFunction)Stream_abort, METH_VARARGS,
     "stream.abort()\n\n"
     "Terminate audio processing immediately without waiting for pending\n"
     "buffers to complete. May raise portaudio.Error."},
    {"is_active", (PyCFunction)Stream_is_active, METH_VARARGS,
     "stream.is_active() -> bool\n\n"
     "Determine whether the stream is active. A stream is active after a\n"
     "successful call to stream.start() until it becomes inactive either\n"
     "as a result of a call to stream.stop() or stream.abort(), or as a\n"
     "result of a return value other than pulseaudio.CONTINUE from the\n"
     "stream callback. In the latter case, the stream is considered\n"
     "inactive after the last buffer has finished playing. May raise\n"
     "portaudio.Error."},
    {"get_time", (PyCFunction)Stream_get_time, METH_VARARGS,
     "stream.get_time() -> float\n\n"
     "Return the current time in seconds for a stream according to the\n"
     "same clock used to generate callback timestamps. The time values\n"
     "are monotonically increasing and have unspecified origin.\n"
     "stream.get_time() returns valid time values for the entire life of\n"
     "the stream, from when the stream is opened until it is closed.\n"
     "Starting and stopping the stream does not affect the passage of\n"
     "time returned by stream.get_time(). This time may be used for\n"
     "synchronizing other events to the audio stream; for example,\n"
     "synchronizing audio to MIDI. Always returns 0.0 if there is an\n"
     "error."},
    {"get_cpu_load", (PyCFunction)Stream_get_cpu_load, METH_VARARGS,
     "stream.get_cpu_load() -> float\n\n"
     "Retrieve CPU usage information for the stream. The \"CPU load\" is\n"
     "a fraction of total CPU time consumed by a callback stream's audio\n"
     "processing routines including, but not limited to, the\n"
     "client-supplied stream callback. This function returns a value,\n"
     "typically between 0.0 and 1.0, where 1.0 indicates that the stream\n"
     "callback is consuming the maximum number of CPU cycles possible to\n"
     "maintain real-time operation. The return value may exceed 1.0. A\n"
     "value of 0.0 will always be returned for a blocking read/write\n"
     "stream, or if an error occurs."},
    {"is_stopped", (PyCFunction)Stream_is_stopped, METH_VARARGS,
     "stream.is_stopped() -> bool\n\n"
     "Determine whether the stream is stopped. A stream is considered to\n"
     "be stopped prior to a successful call to stream.start() and after\n"
     "a successful call to stream.stop() or stream.abort(). If a stream\n"
     "callback returns a value other than portaudio.CONTINUE the stream\n"
     "is NOT considered to be stopped. May raise portaudio.Error."},
    {"get_info", (PyCFunction)Stream_get_info, METH_VARARGS,
     "stream.get_info() -> (float, float, float)\n\n"
     "Retrieve a tuple containing information about the stream.\n\n"
     "    stream.get_info()[0] : The input latency of the stream in\n"
     "    seconds. This value provides the most accurate estimate of\n"
     "    input latency available to the implementation. It may differ\n"
     "    significantly from the suggested latency value passed to\n"
     "    open_stream(). The value of this field will be 0.0 for\n"
     "    output-only streams.\n\n"
     "    stream.get_info()[1] : The output latency of the stream in\n"
     "    seconds. This value provides the most accurate estimate of\n"
     "    output latency available to the implementation. It may differ\n"
     "    significantly from the suggested latency value passed to\n"
     "    open_stream(). The value of this field will be 0.0 for\n"
     "    input-only streams.\n\n"
     "    stream.get_info()[2] : The sample rate of the stream in Hertz\n"
     "    (samples per second). In cases where the hardware sample rate\n"
     "    is inaccurate and PortAudio is aware of it, the value of this\n"
     "    field may be different from the sample rate parameter passed\n"
     "    to open_stream(). If information about the actual hardware\n"
     "    sample rate is not available, this field will have the same\n"
     "    value as the sample rate parameter passed to open_stream()."},
    {NULL},
};

static PyTypeObject StreamType = {
    PyObject_HEAD_INIT(NULL)
    0, /* ob_size */
    "portaudio.Stream", /* tp_name */
    sizeof(Stream), /* tp_basicsize */
    0, /* tp_itemsize */
    (destructor)Stream_dealloc, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    0, /* tp_compare */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_hash */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    "A single Stream can provide multiple channels of real-time\n"
    "streaming audio input and output to a client application. Depending\n"
    "on the underlying host API, it may be possible to open multiple\n"
    "Streams using the same devie; however, this behavior is\n"
    "implementation-defined. Portable applications should assume that a\n"
    "device may be simultaneously used by at most one Stream.",
    0, /* tp_traverse */
    0, /* tp_clear */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0, /* tp_iter */
    0, /* tp_iternext */
    Stream_methods, /* tp_methods */
};

/* unexposed utility functions */

static int paTestCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    int numInputChannels, numOutputChannels, sampleFormat;
    PyObject *callback, *dataObject;
    PyArg_ParseTuple(userData, "iiiOO", &numInputChannels, &numOutputChannels,
                     &sampleFormat, &callback, &dataObject);

    PyObject *inputList, *outputList;
    inputList = PyList_New(framesPerBuffer * numInputChannels);
    outputList = PyList_New(framesPerBuffer * numOutputChannels);
    int i;
    /* check out this nasty bit of copy & paste */
    if (sampleFormat == paFloat32) {
        for (i = 0; i < framesPerBuffer * numInputChannels; i += 2) {
            PyObject *value1 = PyFloat_FromDouble(((float*)inputBuffer)[i]);
            PyList_SetItem(inputList, i, value1);
            PyObject *value2 = PyFloat_FromDouble(((float*)inputBuffer)[i+1]);
            PyList_SetItem(inputList, i+1, value2);
        }
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            PyObject *value1 = PyFloat_FromDouble(((float*)outputBuffer)[i]);
            PyList_SetItem(outputList, i, value1);
            PyObject *value2 = PyFloat_FromDouble(((float*)outputBuffer)[i+1]);
            PyList_SetItem(outputList, i+1, value2);
        }
    } else if (sampleFormat == paInt16) {
        for (i = 0; i < framesPerBuffer * numInputChannels; i += 2) {
            PyObject *value1 = PyInt_FromLong(((short*)inputBuffer)[i]);
            PyList_SetItem(inputList, i, value1);
            PyObject *value2 = PyInt_FromLong(((short*)inputBuffer)[i+1]);
            PyList_SetItem(inputList, i+1, value2);
        }
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            PyObject *value1 = PyInt_FromLong(((short*)outputBuffer)[i]);
            PyList_SetItem(outputList, i, value1);
            PyObject *value2 = PyInt_FromLong(((short*)outputBuffer)[i+1]);
            PyList_SetItem(outputList, i+1, value2);
        }
    } else if (sampleFormat == paInt32) {
        for (i = 0; i < framesPerBuffer * numInputChannels; i += 2) {
            PyObject *value1 = PyInt_FromLong(((int*)inputBuffer)[i]);
            PyList_SetItem(inputList, i, value1);
            PyObject *value2 = PyInt_FromLong(((int*)inputBuffer)[i+1]);
            PyList_SetItem(inputList, i+1, value2);
        }
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            PyObject *value1 = PyInt_FromLong(((int*)outputBuffer)[i]);
            PyList_SetItem(outputList, i, value1);
            PyObject *value2 = PyInt_FromLong(((int*)outputBuffer)[i+1]);
            PyList_SetItem(outputList, i+1, value2);
        }
    } else if (sampleFormat == paInt8) {
        for (i = 0; i < framesPerBuffer * numInputChannels; i += 2) {
            PyObject *value1 = PyInt_FromLong(((char*)inputBuffer)[i]);
            PyList_SetItem(inputList, i, value1);
            PyObject *value2 = PyInt_FromLong(((char*)inputBuffer)[i+1]);
            PyList_SetItem(inputList, i+1, value2);
        }
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            PyObject *value1 = PyInt_FromLong(((char*)outputBuffer)[i]);
            PyList_SetItem(outputList, i, value1);
            PyObject *value2 = PyInt_FromLong(((char*)outputBuffer)[i+1]);
            PyList_SetItem(outputList, i+1, value2);
        }
    } else if (sampleFormat == paUInt8) {
        for (i = 0; i < framesPerBuffer * numInputChannels; i += 2) {
            PyObject *value1 = PyInt_FromLong(((unsigned char*)inputBuffer)[i]);
            PyList_SetItem(inputList, i, value1);
            PyObject *value2 = PyInt_FromLong(((unsigned char*)inputBuffer)[i+1]);
            PyList_SetItem(inputList, i+1, value2);
        }
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            PyObject *value1 = PyInt_FromLong(((unsigned char*)outputBuffer)[i]);
            PyList_SetItem(outputList, i, value1);
            PyObject *value2 = PyInt_FromLong(((unsigned char*)outputBuffer)[i+1]);
            PyList_SetItem(outputList, i+1, value2);
        }
    }
    /* not sure how to implement paInt24 without doing something really ugly */

    PyObject *time = Py_BuildValue("fff", timeInfo->inputBufferAdcTime,
                                   timeInfo->currentTime,
                                   timeInfo->outputBufferDacTime);

    PyObject *arglist, *py_result;
    arglist = Py_BuildValue("OOOO", inputList, outputList, time, dataObject);
    py_result = PyObject_CallObject(callback, arglist);
    Py_DECREF(time);
    Py_DECREF(arglist);

    if (!py_result) {
        PyErr_PrintEx(0);
        Pa_Terminate();
        exit(1);
    }

    if (sampleFormat == paFloat32) {
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            ((float*)outputBuffer)[i] =
                (float)PyFloat_AsDouble(PyList_GetItem(outputList, i));
            ((float*)outputBuffer)[i+1] =
                (float)PyFloat_AsDouble(PyList_GetItem(outputList, i+1));
        }
    } else if (sampleFormat == paInt32) {
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            ((int*)outputBuffer)[i] =
                (int)PyInt_AsLong(PyList_GetItem(outputList, i));
            ((int*)outputBuffer)[i+1] =
                (int)PyInt_AsLong(PyList_GetItem(outputList, i+1));
        }
    } else if (sampleFormat == paInt16) {
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            ((short*)outputBuffer)[i] =
                (short)PyInt_AsLong(PyList_GetItem(outputList, i));
            ((short*)outputBuffer)[i+1] =
                (short)PyInt_AsLong(PyList_GetItem(outputList, i+1));
        }
    } else if (sampleFormat == paInt8) {
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            ((char*)outputBuffer)[i] =
                (char)PyInt_AsLong(PyList_GetItem(outputList, i));
            ((char*)outputBuffer)[i+1] =
                (char)PyInt_AsLong(PyList_GetItem(outputList, i+1));
        }
    } else if (sampleFormat == paUInt8) {
        for (i = 0; i < framesPerBuffer * numOutputChannels; i += 2) {
            ((unsigned char*)outputBuffer)[i] =
                (unsigned char)PyInt_AsLong(PyList_GetItem(outputList, i));
            ((unsigned char*)outputBuffer)[i+1] =
                (unsigned char)PyInt_AsLong(PyList_GetItem(outputList, i+1));
        }
    }

    long result;
    result = PyInt_AsLong(py_result);

    PyGILState_Release(gstate);

    return result;
}

/* module functions */

static PyObject *get_default_input_device(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaDeviceIndex index = Pa_GetDefaultInputDevice();
    if (index < 0) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(index));
        return NULL;
    }
    PyObject *result = PyInt_FromLong(index);
    Py_INCREF(result);
    return result;
}

static PyObject *get_default_output_device(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaDeviceIndex index = Pa_GetDefaultOutputDevice();
    if (index < 0) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(index));
        return NULL;
    }
    PyObject *result = PyInt_FromLong(index);
    Py_INCREF(result);
    return result;
}

static PyObject *get_device_count(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaDeviceIndex count = Pa_GetDeviceCount();
    if (count < 0) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(count));
        return NULL;
    }
    PyObject *result = PyInt_FromLong(count);
    Py_INCREF(result);
    return result;
}

static PyObject *get_last_host_error_info(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    const PaHostErrorInfo *info = Pa_GetLastHostErrorInfo();
    PyObject *result = Py_BuildValue("ls", info->errorCode, info->errorText);
    Py_INCREF(result);
    return result;
}

static PyObject *get_device_index(PyObject *self, PyObject *args) {
    int api, device;
    if (!PyArg_ParseTuple(args, "ii", &api, &device))
        return NULL;

    PaDeviceIndex index = Pa_HostApiDeviceIndexToDeviceIndex(api, device);
    if (index < 0) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(index));
        return NULL;
    }
    PyObject *result = PyInt_FromLong(index);
    Py_INCREF(result);
    return result;
}

static PyObject *get_device_info(PyObject *self, PyObject *args) {
    int device;
    if (!PyArg_ParseTuple(args, "i", &device))
        return NULL;

    const PaDeviceInfo *info = Pa_GetDeviceInfo(device);
    if (!info) {
        PyErr_SetString(PortAudioError, "unspecified error");
        return NULL;
    }
    PyObject *result = Py_BuildValue("siiifffff", info->name, info->hostApi,
                                     info->maxInputChannels,
                                     info->maxOutputChannels,
                                     info->defaultLowInputLatency,
                                     info->defaultLowOutputLatency,
                                     info->defaultHighInputLatency,
                                     info->defaultHighOutputLatency,
                                     info->defaultSampleRate);
    Py_INCREF(result);
    return result;
}

static PyObject *get_host_api_index(PyObject *self, PyObject *args) {
    int type;
    if (!PyArg_ParseTuple(args, "i", &type))
        return NULL;

    PaHostApiIndex index = Pa_HostApiTypeIdToHostApiIndex(type);
    if (index < 0) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(index));
        return NULL;
    }
    PyObject *result = PyInt_FromLong(index);
    Py_INCREF(result);
    return result;
}

static PyObject *get_default_host_api(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaHostApiIndex index = Pa_GetDefaultHostApi();
    if (index < 0) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(index));
        return NULL;
    }
    PyObject *result = PyInt_FromLong(index);
    Py_INCREF(result);
    return result;
}

static PyObject *get_host_api_count(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaHostApiIndex count = Pa_GetHostApiCount();
    if (count < 0) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(count));
        return NULL;
    }
    PyObject *result = PyInt_FromLong(count);
    Py_INCREF(result);
    return result;
}

static PyObject *get_host_api_info(PyObject *self, PyObject *args) {
    PaHostApiIndex hostApi;
    if (!PyArg_ParseTuple(args, "i", &hostApi))
        return NULL;

    const PaHostApiInfo *info = Pa_GetHostApiInfo(hostApi);
    if (!info) {
        PyErr_SetString(PortAudioError, "unspecified error");
        return NULL;
    }
    PyObject *result = Py_BuildValue("isiii", info->type, info->name,
                                     info->deviceCount,
                                     info->defaultInputDevice,
                                     info->defaultOutputDevice);
    Py_INCREF(result);
    return result;
}

static PyObject *get_sample_size(PyObject *self, PyObject *args) {
    unsigned long format;
    if (!PyArg_ParseTuple(args, "k", &format))
        return NULL;
    
    PaError size = Pa_GetSampleSize(format);
    if (size == paSampleFormatNotSupported) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(size));
        return NULL;
    }
    PyObject *result = PyInt_FromLong(size);
    Py_INCREF(result);
    return result;
}

static PyObject *get_version(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    
    PyObject *result = PyInt_FromLong(Pa_GetVersion());
    Py_INCREF(result);
    return result;
}

static PyObject *get_version_text(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    
    PyObject *result = PyString_FromString(Pa_GetVersionText());
    Py_INCREF(result);
    return result;
}

static PyObject *initialize(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *open_default_stream(PyObject *self, PyObject *args) {
    int numInputChannels, numOutputChannels;
    PaSampleFormat sampleFormat;
    double sampleRate;
    unsigned long framesPerBuffer;
    PyObject *callback, *userData;
    if (!PyArg_ParseTuple(args, "iikdkOO", &numInputChannels, &numOutputChannels,
                          &sampleFormat, &sampleRate, &framesPerBuffer,
                          &callback, &userData))
        return NULL;

    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
        return NULL;
    }
    Py_XINCREF(callback);
    Py_XINCREF(userData);

    PyObject *backend_data = Py_BuildValue("iiiOO", numInputChannels,
                                           numOutputChannels, sampleFormat,
                                           callback, userData);

    PaStream *stream;
    PaError err;
    err = Pa_OpenDefaultStream(&stream, numInputChannels, numOutputChannels,
                               sampleFormat, sampleRate, framesPerBuffer,
                               paTestCallback, (void*)backend_data);
    if (err != paNoError) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }
    Stream *py_stream;
    py_stream = (Stream*)StreamType.tp_alloc(&StreamType, 0);
    py_stream->stream = stream;

    Py_INCREF(py_stream);
    return (PyObject*)py_stream;
}

static PyObject *sleep_(PyObject *self, PyObject *args) {
    long msec;
    if (!PyArg_ParseTuple(args, "l", &msec))
        return NULL;

    Pa_Sleep(msec);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *terminate(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaError err;
    err = Pa_Terminate();
    if (err != paNoError) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef PortAudioMethods[] = {
    {"get_device_info", get_device_info, METH_VARARGS,
     "get_device_info(index) -> (str, int, int, int, float, float,\n"
     "                           float, float, float)\n\n"
     "Retrieve a tuple contining information about the device at\n"
     "'index'.\n\n"
     "    [0] : name\n"
     "    [1] : host API index\n"
     "    [2] : maximum number of input channels\n"
     "    [3] : maximum number of output channels\n"
     "    [4] : default latency value for interactive input\n"
     "    [5] : default latency value for interactive output\n"
     "    [6] : default latency value for non-interactive input\n"
     "    [7] : default latency value for non-interactive output\n"
     "    [8] : default sample rate\n\n"
     "May raise portaudio.Error."},
    {"get_default_input_device", get_default_input_device, METH_VARARGS,
     "get_default_input_device() -> int\n\n"
     "Retrieve the index of the default input device. The result can be\n"
     "used in the input device parameter to open_stream(). May raise\n"
     "portaudio.Error."},
    {"get_default_output_device", get_default_output_device, METH_VARARGS,
     "get_default_output_device() -> int\n\n"
     "Retrieve the index of the default output device. The result can be\n"
     "used in the output device parameter to open_stream(). May raise\n"
     "portaudio.Error."},
    {"get_device_count", get_device_count, METH_VARARGS,
     "get_device_count() -> int\n\n"
     "Retrieve the number of available devices. The number of available\n"
     "devices may be 0. May raise portaudio.Error."},
    {"get_last_host_error_info", get_last_host_error_info, METH_VARARGS,
     "get_last_host_error_info() -> (int, str)\n\n"
     "Return information about the last host error encountered. This\n"
     "function is provided as a last resort, primarily to enhance\n"
     "debugging by providing clients with access to all available error\n"
     "information. The values returned will only be valid if a PortAudio\n"
     "function has previously returned an 'unanticipated host' error."}, 
    {"get_device_index", get_device_index, METH_VARARGS,
     "get_device_index(host_api_index, host_api_device_index) -> int\n\n"
     "Convert a host-API-specific device index to standard PortAudio\n"
     "device index. This function may be used in conjunction with the\n"
     "device count of get_host_api_info() to enumerate all devices for\n"
     "the specified host API. May raise portaudio.Error."},
    {"get_host_api_index", get_host_api_index, METH_VARARGS,
     "get_host_api_index(type_id) -> int\n\n"
     "Convert a static host API unique identifier into a runtime host\n"
     "API index. May raise portaudio.Error."},
    {"get_default_host_api", get_default_host_api, METH_VARARGS,
     "get_default_host_api() -> int\n\n"
     "Retrieve the index of the default host API. The default host API\n"
     "will be the lowest common denominator host API on the current\n"
     "platform and is unlikely to provide the best performance. May\n"
     "raise portaudio.Error."},
    {"get_host_api_info", get_host_api_info, METH_VARARGS,
     "get_host_api_info(index) -> (int, string, int, int, int)\n\n"
     "Retrieve a tuple containing information about the host API at\n"
     "'index'.\n\n"
     "    get_host_api_info(index)[0] : The well-known unique identifier\n"
     "    of this host API.\n\n"
     "    get_host_api_info(index)[1] : A textual description of the\n"
     "    host API for display on user interfaces.\n\n"
     "    get_host_api_info(index)[2] : The number of devices belonging\n"
     "    to this host API. This field may be used in conjunction with\n"
     "    get_device_index() to enumerate all devices for this host\n"
     "    API.\n\n"
     "    get_host_api_info(index)[3] : The default index for this host\n"
     "    API. The value will be a device index ranging from 0 to\n"
     "    (get_device_count() - 1), or portaudio.NO_DEVICE if no default\n"
     "    input device is available.\n\n"
     "    get_host_api_info(index)[4] : The default output device for\n"
     "    this host API. The value will be a device index ranging from 0\n"
     "    to (get_device_count() - 1), or portaudio.NO_DEVICE if no\n"
     "    default output device is available.\n\n"
     "May raise portaudio.Error."},
    {"get_host_api_count", get_host_api_count, METH_VARARGS,
     "get_host_api_count() -> int\n\n"
     "Retrieve the number of available host APIs. Even if a host API is\n"
     "available it may have no devices available. May raise\n"
     "portaudio.Error."},
    {"get_version", get_version, METH_VARARGS,
     "get_version() -> int\n\n"
     "Return the release number of the currently running PortAudio\n"
     "build, e.g. 1900."},
    {"get_version_text", get_version_text, METH_VARARGS,
     "get_version_text() -> str\n\n"
     "Return a textual description of the current PortAudio build, e.g.\n"
     "\"Portaudio V19-devel 13 October 2002\"."},
    {"initialize", initialize, METH_VARARGS,
     "initialize()\n\n"
     "Initialize the library - call this before using PortAudio. With\n"
     "the exception of get_version() and get_version_text(), this\n"
     "function MUST be called before using any other PortAudio API\n"
     "functions. If initialize() is called multiple times, each\n"
     "successful call must be matched with a corresponding call to\n"
     "terminate(). Pairs of calls to initialize()/terminate() may\n"
     "overlap, and are not required to be fully nested. Note that if\n"
     "initialize() raises an exception, terminate() should NOT be\n"
     "called."},
    {"open_default_stream", open_default_stream, METH_VARARGS,
     "open_default_stream(num_input_channels, num_output_channels,\n"
     "                    sample_format, sample_rate, frames_per_buffer,\n"
     "                    stream_callback, user_data) -> Stream\n\n"
     "Open the default input and/or output devices, returning a Stream."},
    {"sleep", sleep_, METH_VARARGS,
     "sleep(msec)\n\n"
     "Put the caller to sleep for at least 'msec' milliseconds. This\n"
     "function may sleep longer than requested, so don't rely on this\n"
     "for accurate musical timing."},
    {"terminate", terminate, METH_VARARGS,
     "terminate()\n\n"
     "Terminate the library - call this when finished using PortAudio.\n"
     "In cases where initialize() has been called multiple times, each\n"
     "call must be matched with a corresponding call to terminate(). The\n"
     "final matching call to terminate() will automatically close any\n"
     "PortAudio streams that are still open. terminate() MUST be called\n"
     "before exiting a program which uses PortAudio. Failure to do so\n"
     "may result in serious resource leaks, such as audio devices not\n"
     "being available until the next reboot."},
    {"get_sample_size", get_sample_size, METH_VARARGS,
     "get_sample_size(format) -> int\n\n"
     "Retrive the size of a given sample format in bytes. May raise\n"
     "portaudio.Error if the format is not supported."},
    {NULL, NULL, 0, NULL},
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initportaudio(void) {
    PyObject *m;

    if (PyType_Ready(&StreamType) < 0)
        return;

    m = Py_InitModule("portaudio", PortAudioMethods);

    Py_INCREF(&StreamType);
    PyModule_AddObject(m, "Stream", (PyObject*)&StreamType);

    PortAudioError = PyErr_NewException("portaudio.Error", NULL, NULL);
    Py_INCREF(PortAudioError);
    PyModule_AddObject(m, "Error", PortAudioError);

    PyModule_AddIntConstant(m, "FLOAT32", paFloat32);
    PyModule_AddIntConstant(m, "INT16", paInt16);
    PyModule_AddIntConstant(m, "INT32", paInt32);
    PyModule_AddIntConstant(m, "INT24", paInt24);
    PyModule_AddIntConstant(m, "INT8", paInt8);
    PyModule_AddIntConstant(m, "UINT8", paUInt8);

    PyModule_AddIntConstant(m, "CONTINUE", paContinue);
    PyModule_AddIntConstant(m, "COMPLETE", paComplete);
    PyModule_AddIntConstant(m, "ABORT", paAbort);

    PyModule_AddIntConstant(m, "IN_DEVELOPMENT", paInDevelopment);
    PyModule_AddIntConstant(m, "DIRECT_SOUND", paDirectSound);
    PyModule_AddIntConstant(m, "MME", paMME);
    PyModule_AddIntConstant(m, "ASIO", paASIO);
    PyModule_AddIntConstant(m, "SOUND_MANAGER", paSoundManager);
    PyModule_AddIntConstant(m, "CORE_AUDIO", paCoreAudio);
    PyModule_AddIntConstant(m, "OSS", paOSS);
    PyModule_AddIntConstant(m, "ALSA", paALSA);
    PyModule_AddIntConstant(m, "AL", paAL);
    PyModule_AddIntConstant(m, "BE_OS", paBeOS);
    PyModule_AddIntConstant(m, "WDMKS", paWDMKS);
    PyModule_AddIntConstant(m, "JACK", paJACK);
    PyModule_AddIntConstant(m, "WASAPI", paWASAPI);
    PyModule_AddIntConstant(m, "AUDIO_SCIENCE_HPI", paAudioScienceHPI);

    if (!PyEval_ThreadsInitialized()) {
        PyEval_InitThreads();
    }
}
