#include "Python.h"
#include "graphgen.hpp"

extern "C" {

static PyObject * GG_srand(PyObject *self, PyObject *args) {
    int s;
    if(!PyArg_ParseTuple(args, "i", &s))
        return NULL;
    srand(s);
    Py_RETURN_NONE;
}

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
    0,                                  /* ob_size*/
    "graphgen.LinearSampleIterator",    /* tp_name*/
    sizeof(LSIteratorObj),              /* tp_basicsize*/
    0,                                  /* tp_itemsize*/
    0,                                  /* tp_dealloc*/
    0,                                  /* tp_print*/
    0,                                  /* tp_getattr*/
    0,                                  /* tp_setattr*/
    0,                                  /* tp_compare*/
    0,                                  /* tp_repr*/
    0,                                  /* tp_as_number*/
    0,                                  /* tp_as_sequence*/
    0,                                  /* tp_as_mapping*/
    0,                                  /* tp_hash */
    0,                                  /* tp_call*/
    0,                                  /* tp_str*/
    0,                                  /* tp_getattro*/
    0,                                  /* tp_setattro*/
    0,                                  /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                 /* tp_flags*/
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

typedef struct {
    PyObject_HEAD
    LinearSample* ls;
} LinearSampleObj;

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
    long int max;
    if(!PyArg_ParseTuple(args, "il", &num, &max))
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
    0,                                  /* ob_size*/
    "graphgen.LinearSample",            /* tp_name*/
    sizeof(LinearSampleObj),            /* tp_basicsize*/
    0,                                  /* tp_itemsize*/
    (destructor)LS_dealloc,             /* tp_dealloc*/
    0,                                  /* tp_print*/
    0,                                  /* tp_getattr*/
    0,                                  /* tp_setattr*/
    0,                                  /* tp_compare*/
    0,                                  /* tp_repr*/
    0,                                  /* tp_as_number*/
    0,                                  /* tp_as_sequence*/
    0,                                  /* tp_as_mapping*/
    0,                                  /* tp_hash */
    0,                                  /* tp_call*/
    0,                                  /* tp_str*/
    0,                                  /* tp_getattro*/
    0,                                  /* tp_setattro*/
    0,                                  /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                 /* tp_flags*/
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


typedef struct {
    PyObject_HEAD
    DSU* dsu;
} DSUObj;

static PyObject* DSU_dealloc(DSUObj* self) {
    if(self->dsu) delete self->dsu;
    self->ob_type->tp_free((PyObject*)self);
}

static int DSU_init(DSUObj* self,
                    PyObject *args,
                    PyObject *kwds) {
    int sz;
    if(!PyArg_ParseTuple(args, "i", &sz))
        return -1;
    if(self->dsu) {
        // Someone who feels playful could call __init__() twice
        delete self->dsu;
        self->dsu = NULL;
    }
    try {
        self->dsu = new DSU(sz);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return -1;
    }
    return 0;
}

static PyObject* DSU_find(DSUObj* self,
                          PyObject *args,
                          PyObject *kwds) {
    int a;
    if(!PyArg_ParseTuple(args, "i", &a))
        return NULL;
    if(a<0 || a>=self->dsu->size()) {
        PyErr_SetString(PyExc_ValueError, "Value out of range!");
        return NULL;
    }
    return PyInt_FromLong(self->dsu->find(a));
}

static PyObject* DSU_merge(DSUObj* self,
                           PyObject *args,
                           PyObject *kwds) {
    int a, b;
    if(!PyArg_ParseTuple(args, "ii", &a, &b))
        return NULL;
    if(a<0 || a>=self->dsu->size() || b<0 || b>=self->dsu->size()) {
        PyErr_SetString(PyExc_ValueError, "Values out of range!");
        return NULL;
    }
    self->dsu->merge(a, b);
    Py_RETURN_NONE;
}

static PyMethodDef DSU_methods[] = {
    {"find", (PyCFunction)DSU_find, METH_VARARGS,
     "Find the representative of an element."},
    {"merge", (PyCFunction)DSU_merge, METH_VARARGS,
     "Merge two sets together."},
    {NULL}
};

static PyTypeObject DSUType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size*/
    "graphgen.DSU",                     /* tp_name*/
    sizeof(DSUObj),                     /* tp_basicsize*/
    0,                                  /* tp_itemsize*/
    (destructor) DSU_dealloc,           /* tp_dealloc*/
    0,                                  /* tp_print*/
    0,                                  /* tp_getattr*/
    0,                                  /* tp_setattr*/
    0,                                  /* tp_compare*/
    0,                                  /* tp_repr*/
    0,                                  /* tp_as_number*/
    0,                                  /* tp_as_sequence*/
    0,                                  /* tp_as_mapping*/
    0,                                  /* tp_hash */
    0,                                  /* tp_call*/
    0,                                  /* tp_str*/
    0,                                  /* tp_getattro*/
    0,                                  /* tp_setattro*/
    0,                                  /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                 /* tp_flags*/
    "Disjoint Set data structure",      /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    DSU_methods,                        /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc) DSU_init,                /* tp_init */
    PyType_GenericAlloc,                /* tp_alloc */
};

typedef struct {
    PyObject_HEAD
    UndirectedGraph<RandomIntLabels>* g;
} UGObj;

static PyObject* UG_dealloc(UGObj* self) {
    if(self->g) delete self->g;
    self->ob_type->tp_free((PyObject*)self);
}

static int UG_init(UGObj* self,
                   PyObject *args,
                   PyObject *kwds) {
    int sz;
    if(!PyArg_ParseTuple(args, "i", &sz))
        return -1;
    if(self->g) {
        // Someone who feels playful could call __init__() twice
        delete self->g;
        self->g = NULL;
    }
    try {
        self->g = new UndirectedGraph<RandomIntLabels>(sz);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return -1;
    }
    return 0;
}

static PyObject* UG_str(PyObject* self) {
    const string& repr = ((UGObj*)self)->g->to_string();
    return PyString_FromStringAndSize(repr.c_str(), repr.size()-1);
}

static PyObject* UG_add_edge(UGObj* self,
                             PyObject *args,
                             PyObject *kwds) {
    int a, b;
    if(!PyArg_ParseTuple(args, "ii", &a, &b))
        return NULL;
    try {
        self->g->add_edge(a, b);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* UG_add_random_edges(UGObj* self,
                                     PyObject *args,
                                     PyObject *kwds) {
    int M;
    if(!PyArg_ParseTuple(args, "i", &M))
        return NULL;
    try {
        self->g->add_random_edges(M);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* UG_build_forest(UGObj* self,
                                PyObject *args,
                                PyObject *kwds) {
    int M;
    if(!PyArg_ParseTuple(args, "i", &M))
        return NULL;
    try {
        self->g->build_forest(M);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* UG_connect(UGObj* self) {
    try {
        self->g->connect();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* UG_build_path(UGObj* self) {
    try {
        self->g->build_path();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* UG_build_cycle(UGObj* self) {
    try {
        self->g->build_cycle();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* UG_build_tree(UGObj* self) {
    try {
        self->g->build_tree();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* UG_build_star(UGObj* self) {
    try {
        self->g->build_star();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* UG_build_wheel(UGObj* self) {
    try {
        self->g->build_wheel();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* UG_build_clique(UGObj* self) {
    try {
        self->g->build_clique();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyMethodDef UG_methods[] = {
    {"add_edge", (PyCFunction)UG_add_edge, METH_VARARGS,
     "Add an edge to the graph."},
    {"add_random_edges", (PyCFunction)UG_add_random_edges, METH_VARARGS,
     "Add some new edges to the graph."},
    {"connect", (PyCFunction)UG_connect, METH_NOARGS,
     "Add the minimum number of edges to make the graph connected."},
    {"build_forest", (PyCFunction)UG_build_forest, METH_VARARGS,
     "Creates a forest with the given number of edges."},
    {"build_path", (PyCFunction)UG_build_path, METH_NOARGS,
     "Creates a path."},
    {"build_cycle", (PyCFunction)UG_build_cycle, METH_NOARGS,
     "Creates a cycle."},
    {"build_tree", (PyCFunction)UG_build_tree, METH_NOARGS,
     "Creates a tree."},
    {"build_star", (PyCFunction)UG_build_star, METH_NOARGS,
     "Creates a star."},
    {"build_wheel", (PyCFunction)UG_build_wheel, METH_NOARGS,
     "Creates a wheel."},
    {"build_clique", (PyCFunction)UG_build_clique, METH_NOARGS,
     "Creates a clique."},
    {NULL}
};

static PyTypeObject UGType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size*/
    "graphgen.UndirectedGraph",         /* tp_name*/
    sizeof(UGObj),                      /* tp_basicsize*/
    0,                                  /* tp_itemsize*/
    (destructor) UG_dealloc,            /* tp_dealloc*/
    0,                                  /* tp_print*/
    0,                                  /* tp_getattr*/
    0,                                  /* tp_setattr*/
    0,                                  /* tp_compare*/
    0,                                  /* tp_repr*/
    0,                                  /* tp_as_number*/
    0,                                  /* tp_as_sequence*/
    0,                                  /* tp_as_mapping*/
    0,                                  /* tp_hash */
    0,                                  /* tp_call*/
    UG_str,                             /* tp_str*/
    0,                                  /* tp_getattro*/
    0,                                  /* tp_setattro*/
    0,                                  /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                 /* tp_flags*/
    "Undirected graph",                 /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    UG_methods,                         /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc) UG_init,                 /* tp_init */
    PyType_GenericAlloc,                /* tp_alloc */
};

typedef struct {
    PyObject_HEAD
    DirectedGraph<RandomIntLabels>* g;
} DGObj;

static PyObject* DG_dealloc(DGObj* self) {
    if(self->g) delete self->g;
    self->ob_type->tp_free((PyObject*)self);
}

static int DG_init(DGObj* self,
                   PyObject *args,
                   PyObject *kwds) {
    int sz;
    if(!PyArg_ParseTuple(args, "i", &sz))
        return -1;
    if(self->g) {
        // Someone who feels playful could call __init__() twice
        delete self->g;
        self->g = NULL;
    }
    try {
        self->g = new DirectedGraph<RandomIntLabels>(sz);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return -1;
    }
    return 0;
}

static PyObject* DG_str(PyObject* self) {
    const string& repr = ((DGObj*)self)->g->to_string();
    return PyString_FromStringAndSize(repr.c_str(), repr.size()-1);
}

static PyObject* DG_add_edge(DGObj* self,
                             PyObject *args,
                             PyObject *kwds) {
    int a, b;
    if(!PyArg_ParseTuple(args, "ii", &a, &b))
        return NULL;
    try {
        self->g->add_edge(a, b);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* DG_add_random_edges(DGObj* self,
                                     PyObject *args,
                                     PyObject *kwds) {
    int M;
    if(!PyArg_ParseTuple(args, "i", &M))
        return NULL;
    try {
        self->g->add_random_edges(M);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* DG_build_forest(DGObj* self,
                                PyObject *args,
                                PyObject *kwds) {
    int M;
    if(!PyArg_ParseTuple(args, "i", &M))
        return NULL;
    try {
        self->g->build_forest(M);
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* DG_connect(DGObj* self) {
    try {
        self->g->connect();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* DG_build_path(DGObj* self) {
    try {
        self->g->build_path();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* DG_build_cycle(DGObj* self) {
    try {
        self->g->build_cycle();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* DG_build_tree(DGObj* self) {
    try {
        self->g->build_tree();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* DG_build_star(DGObj* self) {
    try {
        self->g->build_star();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* DG_build_wheel(DGObj* self) {
    try {
        self->g->build_wheel();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* DG_build_clique(DGObj* self) {
    try {
        self->g->build_clique();
    }
    catch(exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyMethodDef DG_methods[] = {
    {"add_edge", (PyCFunction)DG_add_edge, METH_VARARGS,
     "Add an edge to the graph."},
    {"add_random_edges", (PyCFunction)DG_add_random_edges, METH_VARARGS,
     "Add some new edges to the graph."},
    {"connect", (PyCFunction)DG_connect, METH_NOARGS,
     "Add the minimum number of edges to make the graph connected."},
    {"build_forest", (PyCFunction)DG_build_forest, METH_VARARGS,
     "Creates a forest with the given number of edges."},
    {"build_path", (PyCFunction)DG_build_path, METH_NOARGS,
     "Creates a path."},
    {"build_cycle", (PyCFunction)DG_build_cycle, METH_NOARGS,
     "Creates a cycle."},
    {"build_tree", (PyCFunction)DG_build_tree, METH_NOARGS,
     "Creates a tree."},
    {"build_star", (PyCFunction)DG_build_star, METH_NOARGS,
     "Creates a star."},
    {"build_wheel", (PyCFunction)DG_build_wheel, METH_NOARGS,
     "Creates a wheel."},
    {"build_clique", (PyCFunction)DG_build_clique, METH_NOARGS,
     "Creates a clique."},
    {NULL}
};

static PyTypeObject DGType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size*/
    "graphgen.DirectedGraph",           /* tp_name*/
    sizeof(DGObj),                      /* tp_basicsize*/
    0,                                  /* tp_itemsize*/
    (destructor) DG_dealloc,            /* tp_dealloc*/
    0,                                  /* tp_print*/
    0,                                  /* tp_getattr*/
    0,                                  /* tp_setattr*/
    0,                                  /* tp_compare*/
    0,                                  /* tp_repr*/
    0,                                  /* tp_as_number*/
    0,                                  /* tp_as_sequence*/
    0,                                  /* tp_as_mapping*/
    0,                                  /* tp_hash */
    0,                                  /* tp_call*/
    DG_str,                             /* tp_str*/
    0,                                  /* tp_getattro*/
    0,                                  /* tp_setattro*/
    0,                                  /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                 /* tp_flags*/
    "Directed graph",                   /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    DG_methods,                         /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc) DG_init,                 /* tp_init */
    PyType_GenericAlloc,                /* tp_alloc */
};

static PyMethodDef graphgen_methods[] = {
    {"srand", (PyCFunction)GG_srand, METH_VARARGS, "Call srand()"},
    {NULL}  /* Sentinel */
};

PyMODINIT_FUNC initgraphgen(void) {
    PyObject* m;
    LinearSampleType.tp_new = PyType_GenericNew;
    LSIteratorType.tp_new = PyType_GenericNew;
    DSUType.tp_new = PyType_GenericNew;
    UGType.tp_new = PyType_GenericNew;
    DGType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&LinearSampleType) < 0)
        return;
    if (PyType_Ready(&LSIteratorType) < 0)
        return;
    if (PyType_Ready(&DSUType) < 0)
        return;
    if (PyType_Ready(&UGType) < 0)
        return;
    if (PyType_Ready(&DGType) < 0)
        return;

    m = Py_InitModule3("graphgen", graphgen_methods,
                       "Module to generate graphs");

    Py_INCREF(&LinearSampleType);
    Py_INCREF(&LSIteratorType);
    PyModule_AddObject(m, "LinearSample", (PyObject *)&LinearSampleType);
    PyModule_AddObject(m, "LinearSampleIterator", (PyObject *)&LSIteratorType);
    PyModule_AddObject(m, "DSU", (PyObject *)&DSUType);
    PyModule_AddObject(m, "UndirectedGraph", (PyObject *)&UGType);
    PyModule_AddObject(m, "DirectedGraph", (PyObject *)&DGType);
}

}
