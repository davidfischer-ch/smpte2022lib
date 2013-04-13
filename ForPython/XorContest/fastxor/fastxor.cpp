// Copyright eryksun <http://stackoverflow.com/users/205580/eryksun>
// http://stackoverflow.com/questions/15459684/transmission-bytearray-from-python-to-c-and-return-it

#include <Python.h>

static PyObject *fast_xor_inplace(PyObject *self, PyObject *args)
{
    PyObject *arg1, *arg2;
    Py_buffer buffer1, buffer2;
    char *buf1, *buf2;
    int i;

    if (!PyArg_ParseTuple(args, "O|O:fast_xor_inplace", &arg1, &arg2))
        return NULL;

    if (PyObject_GetBuffer(arg1, &buffer1, PyBUF_WRITABLE) < 0)
        return NULL;

    if (PyObject_GetBuffer(arg2, &buffer2, PyBUF_WRITABLE) < 0)
        return NULL;

    buf1 = buffer1.buf;
    buf2 = buffer2.buf;
    for(i=0; i < buffer1.len; i++)
        buf1[i] ^= buf2[i];

    PyBuffer_Release(&buffer1);
    PyBuffer_Release(&buffer2);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *fast_xor_inplace2(PyObject *self, PyObject *args)
{
    PyObject *arg1;
    unsigned char arg2 = 55;
    Py_buffer buffer;
    char *buf;
    int i;

    if (!PyArg_ParseTuple(args, "O|b:fast_xor_inplace", &arg1, &arg2))
        return NULL;

    if (PyObject_GetBuffer(arg1, &buffer, PyBUF_WRITABLE) < 0)
        return NULL;

    buf = buffer.buf;
    for(i=0; i < buffer.len; i++)
        buf[i] ^= arg2;

    PyBuffer_Release(&buffer);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *fast_xor(PyObject *self, PyObject *args)
{
    PyObject *arg1;
    unsigned char arg2 = 55;
    PyObject *result;
    Py_buffer buffer;
    char *buf, *str;
    int i;

    if (!PyArg_ParseTuple(args, "O|b:fast_xor", &arg1, &arg2))
        return NULL;

    if (PyObject_GetBuffer(arg1, &buffer, PyBUF_SIMPLE) < 0)
        return NULL;

    result = PyString_FromStringAndSize(NULL, buffer.len);
    if (result == NULL)
        return NULL;

    buf = buffer.buf;
    str = PyString_AS_STRING(result);
    for(i=0; i < buffer.len; i++)
        str[i] = buf[i] ^ arg2;

    PyBuffer_Release(&buffer);
    return result;
}

static PyMethodDef fastxorMethods[] =
{
     {"fast_xor", fast_xor, METH_VARARGS, "fast xor"},
     {"fast_xor_inplace", fast_xor_inplace, METH_VARARGS, "fast inplace xor"},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initfastxor(void)
{
    Py_InitModule3("fastxor", fastxorMethods, "fast xor functions");
}
