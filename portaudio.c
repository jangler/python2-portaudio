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
    err = Pa_AbortStream(self->stream);
    if (err != paNoError) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef Stream_methods[] = {
    {"start", (PyCFunction)Stream_start, METH_VARARGS,
     "Commence audio processing."},
    {"stop", (PyCFunction)Stream_stop, METH_VARARGS,
     "Terminate audio processing, waiting until all pending audio buffers "
     "have been played before returning."},
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
    "Provides multiple channels of real-time streaming audio input and "
    "output to a client application.", /* tp_str */
    0, /* tp_traverse */
    0, /* tp_clear */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0, /* tp_iter */
    0, /* tp_iternext */
    Stream_methods, /* tp_methods */
};

/* TimeInfo (PaStreamCallbackTimeInfo) */

typedef struct {
    PyObject_HEAD
    PaStreamCallbackTimeInfo *info;
} TimeInfo;

static void TimeInfo_dealloc(TimeInfo* self) {
    Py_XDECREF(self->info);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *TimeInfo_current_time(TimeInfo *self) {
    PyObject *result;
    result = PyFloat_FromDouble(self->info->currentTime);
    return result;
}

static PyObject *TimeInfo_input_time(TimeInfo *self) {
    PyObject *result;
    result = PyFloat_FromDouble(self->info->inputBufferAdcTime);
    return result;
}

static PyObject *TimeInfo_output_time(TimeInfo *self) {
    PyObject *result;
    result = PyFloat_FromDouble(self->info->outputBufferDacTime);
    return result;
}

static PyMethodDef TimeInfo_methods[] = {
    {"current_time", (PyCFunction)TimeInfo_current_time, METH_NOARGS,
     "The time when the stream callback was invoked."},
    {"input_time", (PyCFunction)TimeInfo_input_time, METH_NOARGS,
     "The time when the first sample of the input buffer was captured at the "
     "ADC input."},
    {"output_time", (PyCFunction)TimeInfo_output_time, METH_NOARGS,
     "The time when the first sample of the output buffer will output the "
     "DAC."},
    {NULL},
};

static PyTypeObject TimeInfoType = {
    PyObject_HEAD_INIT(NULL)
    0, /* ob_size */
    "portaudio.TimeInfo", /* tp_name */
    sizeof(TimeInfo), /* tp_basicsize */
    0, /* tp_itemsize */
    (destructor)TimeInfo_dealloc, /* tp_dealloc */
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
    "Time information for the buffers passed to the stream "
    "callback.", /* tp_str */
    0, /* tp_traverse */
    0, /* tp_clear */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0, /* tp_iter */
    0, /* tp_iternext */
    TimeInfo_methods, /* tp_methods */
};

/* module functions */

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

static PyObject *my_callback;

static int paTestCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject *inputList, *outputList;
    inputList = PyList_New(framesPerBuffer * 2);
    outputList = PyList_New(framesPerBuffer * 2);
    int i;
    for (i = 0; i < framesPerBuffer * 2; i += 2) {
        PyObject *value1 = PyFloat_FromDouble(((float*)outputBuffer)[i]);
        PyList_SetItem(outputList, i, value1);
        PyObject *value2 = PyFloat_FromDouble(((float*)outputBuffer)[i+1]);
        PyList_SetItem(outputList, i+1, value2);
    }

    TimeInfo *time;
    time = (TimeInfo*)TimeInfoType.tp_alloc(&TimeInfoType, 0);
    time->info = (PaStreamCallbackTimeInfo*)(timeInfo);

    PyObject *arglist, *py_result;
    arglist = Py_BuildValue("OOkOO", inputList, outputList, framesPerBuffer,
                            time, userData);
    py_result = PyObject_CallObject(my_callback, arglist);
    Py_DECREF(arglist);

    for (i = 0; i < framesPerBuffer * 2; i += 2) {
        ((float*)outputBuffer)[i] =
            (float)PyFloat_AsDouble(PyList_GetItem(outputList, i));
        ((float*)outputBuffer)[i+1] =
            (float)PyFloat_AsDouble(PyList_GetItem(outputList, i+1));
    }

    long result;
    result = PyInt_AsLong(py_result);

    PyGILState_Release(gstate);

    return result;
}

typedef struct {
    float left_phase, right_phase;
} paTestData;

static paTestData data;

static int testCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData) {
    paTestData *data = (paTestData*)userData;
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer;

    for (i = 0; i < framesPerBuffer; i++) {
        *out++ = data->left_phase;
        *out++ = data->right_phase;
        data->left_phase += 0.01f;
        if (data->left_phase >= 1.0f)
            data->left_phase -= 2.0f;
        data->right_phase += 0.01f;
        if (data->right_phase >= 1.0f)
            data->right_phase -= 2.0f;
    }
    return 0;
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
    Py_XDECREF(my_callback);
    my_callback = callback;
    Py_XINCREF(userData);

    PaStream *stream;
    PaError err;
    err = Pa_OpenDefaultStream(&stream, numInputChannels, numOutputChannels,
                               sampleFormat, sampleRate, framesPerBuffer,
                               paTestCallback, (void*)userData);
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

static PyObject *subtest(PyObject *self, PyObject *args) {
    Stream *py_stream;
    if (!PyArg_ParseTuple(args, "O", &py_stream))
        return NULL;

    PyObject *status;
    status = Stream_start(py_stream, Py_BuildValue("()"));
    if (status == NULL)
        return NULL;

    status = sleep_(self, Py_BuildValue("(l)", 1000));
    if (status == NULL)
        return NULL;

    status = Stream_stop(py_stream, Py_BuildValue("()"));
    if (status == NULL)
        return NULL;

    status = terminate(self, Py_BuildValue("()"));
    if (status == NULL)
        return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *test(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PaError err;
    PyObject *status;

    status = initialize(self, Py_BuildValue("()"));
    if (status == NULL)
        return NULL;

    PaStream *stream;
    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, 44100, 256,
                               testCallback, &data);
    if (err != paNoError) {
        PyErr_SetString(PortAudioError, Pa_GetErrorText(err));
        return NULL;
    }
    Stream *py_stream;
    py_stream = (Stream*)StreamType.tp_alloc(&StreamType, 0);
    py_stream->stream = stream;

    subtest(self, Py_BuildValue("(O)", py_stream));

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef PortAudioMethods[] = {
    {"initialize", initialize, METH_VARARGS,
     "Library initialization function - call this before using PortAudio."},
    {"open_default_stream", open_default_stream, METH_VARARGS,
     "Opens the default input and/or output devices."},
    {"sleep", sleep_, METH_VARARGS,
     "Put the caller to sleep for at least 'msec' milliseconds."},
    {"terminate", terminate, METH_VARARGS,
     "Library termination function - call this when finished using "
     "PortAudio."},
    {"test", test, METH_VARARGS, "test"},
    {NULL, NULL, 0, NULL},
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initportaudio(void) {
    PyObject *m;

    if (PyType_Ready(&TimeInfoType) < 0)
        return;
    StreamType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&StreamType) < 0)
        return;

    m = Py_InitModule("portaudio", PortAudioMethods);

    Py_INCREF(&TimeInfoType);
    PyModule_AddObject(m, "TimeInfo", (PyObject*)&TimeInfoType);
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

    if (!PyEval_ThreadsInitialized()) {
        PyEval_InitThreads();
    }
}
