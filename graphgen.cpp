#include "Python.h"
#include "graphgen.hpp"

typedef struct {
    PyObject_HEAD
    LinearSample* ls;
} LinearSampleObj;

typedef struct {
    PyObject_HEAD
    vector<int>::iterator it;
    vector<int>::iterator end;
} LSIteratorObj;

static PyObject* LSI_next(LSIteratorObj* LSI) {
    if(LSI->it == LSI->end) return NULL;
    PyObject* res = PyInt_FromLong((long)*LSI->it);
    LSI->it++;
    return res;
}

static PyTypeObject LSIteratorType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /*ob_size*/
    "graphgen.LinearSampleIterator",    /*tp_name*/
    sizeof(LSIteratorObj),              /*tp_basicsize*/
    0,                                  /*tp_itemsize*/
    0,                                  /*tp_dealloc*/
    0,                                  /*tp_print*/
    0,                                  /*tp_getattr*/
    0,                                  /*tp_setattr*/
    0,                                  /*tp_compare*/
    0,                                  /*tp_repr*/
    0,                                  /*tp_as_number*/
    0,                                  /*tp_as_sequence*/
    0,                                  /*tp_as_mapping*/
    0,                                  /*tp_hash */
    0,                                  /*tp_call*/
    0,                                  /*tp_str*/
    0,                                  /*tp_getattro*/
    0,                                  /*tp_setattro*/
    0,                                  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                 /*tp_flags*/
    "LinearSample iterator",            /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    PyObject_SelfIter,                  /* tp_iter */
    (iternextfunc)LSI_next,             /* tp_iternext */
    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    PyType_GenericAlloc,                /* tp_alloc */
};

static PyObject* LS_dealloc(LinearSampleObj* self) {
    if(self->ls) delete self->ls;
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject* LS_iter(LinearSampleObj* self) {
    auto res = (LSIteratorObj*) PyType_GenericAlloc(&LSIteratorType, 0);
    res->it = self->ls->begin();
    res->end = self->ls->end();
    return (PyObject*) res;
}

static int LS_init(LinearSampleObj* self,
                         PyObject *args,
                         PyObject *kwds) {
    int num;
    size_t max;
    if(!PyArg_ParseTuple(args, "ik", &num, &max))
        return -1;
    if(self->ls) {
        // Someone who feels playful could call __init__() twice
        delete self->ls;
        self->ls = NULL;
    }
    try {
        self->ls = new LinearSample(num, max);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return -1;
    }
    return 0;
}

static PyTypeObject LinearSampleType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /*ob_size*/
    "graphgen.LinearSample",            /*tp_name*/
    sizeof(LinearSampleObj),            /*tp_basicsize*/
    0,                                  /*tp_itemsize*/
    (destructor)LS_dealloc,             /*tp_dealloc*/
    0,                                  /*tp_print*/
    0,                                  /*tp_getattr*/
    0,                                  /*tp_setattr*/
    0,                                  /*tp_compare*/
    0,                                  /*tp_repr*/
    0,                                  /*tp_as_number*/
    0,                                  /*tp_as_sequence*/
    0,                                  /*tp_as_mapping*/
    0,                                  /*tp_hash */
    0,                                  /*tp_call*/
    0,                                  /*tp_str*/
    0,                                  /*tp_getattro*/
    0,                                  /*tp_setattro*/
    0,                                  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                 /*tp_flags*/
    "LinearSample",                     /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc) LS_iter,              /* tp_iter */
    0,                                  /* tp_iternext */
    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc) LS_init,                 /* tp_init */
    PyType_GenericAlloc,                /* tp_alloc */
};

static PyMethodDef graphgen_methods[] = {
    {NULL}  /* Sentinel */
};

PyMODINIT_FUNC initgraphgen(void) {
    PyObject* m;
    LinearSampleType.tp_new = PyType_GenericNew;
    LSIteratorType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&LinearSampleType) < 0)
        return;
    if (PyType_Ready(&LSIteratorType) < 0)
        return;

    m = Py_InitModule3("graphgen", graphgen_methods,
                       "Module to generate graphs");

    Py_INCREF(&LinearSampleType);
    Py_INCREF(&LSIteratorType);
    PyModule_AddObject(m, "LinearSample", (PyObject *)&LinearSampleType);
    PyModule_AddObject(m, "LinearSampleIterator", (PyObject *)&LSIteratorType);
}
