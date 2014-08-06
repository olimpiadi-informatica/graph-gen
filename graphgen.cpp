#include "Python.h"
#include "graphgen.hpp"

extern "C" {

    static PyObject * GG_srand(PyObject *self, PyObject *args) {
        int s;
        if (!PyArg_ParseTuple(args, "i", &s))
            return NULL;
        srand(s);
        Py_RETURN_NONE;
    }

    typedef struct {
        PyObject_HEAD
        std::vector<int64_t>::iterator it;
        std::vector<int64_t>::iterator end;
    } RSIteratorObj;

    static PyObject* RSI_next(RSIteratorObj* RSI) {
        if (RSI->it == RSI->end)
            return NULL;
        PyObject* res = PyInt_FromLong((long)*RSI->it);
        RSI->it++;
        return res;
    }

    static PyTypeObject RSIteratorType = {
        PyObject_HEAD_INIT(NULL)
        0,                                  /* ob_size */
        "graphgen.RangeSamplerIterator",    /* tp_name */
        sizeof(RSIteratorObj),              /* tp_basicsize */
        0,                                  /* tp_itemsize */
        0,                                  /* tp_dealloc */
        0,                                  /* tp_print */
        0,                                  /* tp_getattr */
        0,                                  /* tp_setattr */
        0,                                  /* tp_compare */
        0,                                  /* tp_repr */
        0,                                  /* tp_as_number */
        0,                                  /* tp_as_sequence */
        0,                                  /* tp_as_mapping */
        0,                                  /* tp_hash */
        0,                                  /* tp_call */
        0,                                  /* tp_str */
        0,                                  /* tp_getattro */
        0,                                  /* tp_setattro */
        0,                                  /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                 /* tp_flags */
        "RangeSampler iterator",            /* tp_doc */
        0,                                  /* tp_traverse */
        0,                                  /* tp_clear */
        0,                                  /* tp_richcompare */
        0,                                  /* tp_weaklistoffset */
        PyObject_SelfIter,                  /* tp_iter */
        (iternextfunc)RSI_next,             /* tp_iternext */
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
        RangeSampler* rs;
    } RangeSamplerObj;

    static void RS_dealloc(RangeSamplerObj* self) {
        if (self->rs)
            delete self->rs;
        self->ob_type->tp_free((PyObject*)self);
    }

    static PyObject* RS_iter(RangeSamplerObj* self) {
        auto res = (RSIteratorObj*) PyType_GenericAlloc(&RSIteratorType, 0);
        res->it = self->rs->begin();
        res->end = self->rs->end();
        return (PyObject*) res;
    }

    static int RS_init(
        RangeSamplerObj* self,
        PyObject* args,
        PyObject* kwds
    ) {
        size_t num;
        long long min, max;

        if (!PyArg_ParseTuple(args, "nLL", &num, &min, &max))
            return -1;
        if (self->rs) {
            // Someone who feels playful could call __init__() twice
            delete self->rs;
            self->rs = NULL;
        }
        try {
            self->rs = new RangeSampler(num, min, max);
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return -1;
        }

        return 0;
    }

    static PyTypeObject RangeSamplerType = {
        PyObject_HEAD_INIT(NULL)
        0,                                  /* ob_size */
        "graphgen.RangeSampler",            /* tp_name */
        sizeof(RangeSamplerObj),            /* tp_basicsize */
        0,                                  /* tp_itemsize */
        (destructor)RS_dealloc,             /* tp_dealloc */
        0,                                  /* tp_print */
        0,                                  /* tp_getattr */
        0,                                  /* tp_setattr */
        0,                                  /* tp_compare */
        0,                                  /* tp_repr */
        0,                                  /* tp_as_number */
        0,                                  /* tp_as_sequence */
        0,                                  /* tp_as_mapping */
        0,                                  /* tp_hash */
        0,                                  /* tp_call */
        0,                                  /* tp_str */
        0,                                  /* tp_getattro */
        0,                                  /* tp_setattro */
        0,                                  /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                 /* tp_flags */
        "RangeSampler",                     /* tp_doc */
        0,                                  /* tp_traverse */
        0,                                  /* tp_clear */
        0,                                  /* tp_richcompare */
        0,                                  /* tp_weaklistoffset */
        (getiterfunc) RS_iter,              /* tp_iter */
        0,                                  /* tp_iternext */
        0,                                  /* tp_methods */
        0,                                  /* tp_members */
        0,                                  /* tp_getset */
        0,                                  /* tp_base */
        0,                                  /* tp_dict */
        0,                                  /* tp_descr_get */
        0,                                  /* tp_descr_set */
        0,                                  /* tp_dictoffset */
        (initproc) RS_init,                 /* tp_init */
        PyType_GenericAlloc,                /* tp_alloc */
    };

    typedef struct {
        PyObject_HEAD
        DisjointSet* disjoint_set;
    } DisjointSetObj;

    static void DisjointSet_dealloc(DisjointSetObj* self) {
        if (self->disjoint_set)
            delete self->disjoint_set;
        self->ob_type->tp_free((PyObject*)self);
    }

    static int DisjointSet_init (
        DisjointSetObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        size_t sz;
        if (!PyArg_ParseTuple(args, "n", &sz))
            return -1;
        if (self->disjoint_set) {
            // Someone who feels playful could call __init__() twice
            delete self->disjoint_set;
            self->disjoint_set = NULL;
        }
        try {
            self->disjoint_set = new DisjointSet(sz);
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return -1;
        }
        return 0;
    }

    static PyObject* DisjointSet_find (
        DisjointSetObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        size_t a;
        if (!PyArg_ParseTuple(args, "n", &a))
            return NULL;
        if (a<0 || a>=self->disjoint_set->size()) {
            PyErr_SetString(PyExc_ValueError, "Value out of range!");
            return NULL;
        }
        return PyInt_FromLong(self->disjoint_set->find(a));
    }

    static PyObject* DisjointSet_merge (
        DisjointSetObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        size_t a, b;
        if (!PyArg_ParseTuple(args, "nn", &a, &b))
            return NULL;
        if (a>=self->disjoint_set->size() || b>=self->disjoint_set->size()) {
            PyErr_SetString(PyExc_ValueError, "Values out of range!");
            return NULL;
        }
        if (self->disjoint_set->merge(a, b)) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    static PyMethodDef DisjointSet_methods[] = {
        {"find", (PyCFunction)DisjointSet_find, METH_VARARGS,
         "Find the representative of an element."},
        {"merge", (PyCFunction)DisjointSet_merge, METH_VARARGS,
         "Merge two sets together."},
        {NULL}
    };

    static PyTypeObject DisjointSetType = {
        PyObject_HEAD_INIT(NULL)
        0,                                  /* ob_size */
        "graphgen.DisjointSet",             /* tp_name */
        sizeof(DisjointSetObj),             /* tp_basicsize */
        0,                                  /* tp_itemsize */
        (destructor) DisjointSet_dealloc,   /* tp_dealloc */
        0,                                  /* tp_print */
        0,                                  /* tp_getattr */
        0,                                  /* tp_setattr */
        0,                                  /* tp_compare */
        0,                                  /* tp_repr */
        0,                                  /* tp_as_number */
        0,                                  /* tp_as_sequence */
        0,                                  /* tp_as_mapping */
        0,                                  /* tp_hash */
        0,                                  /* tp_call */
        0,                                  /* tp_str */
        0,                                  /* tp_getattro */
        0,                                  /* tp_setattro */
        0,                                  /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                 /* tp_flags */
        "Disjoint Set data structure",      /* tp_doc */
        0,                                  /* tp_traverse */
        0,                                  /* tp_clear */
        0,                                  /* tp_richcompare */
        0,                                  /* tp_weaklistoffset */
        0,                                  /* tp_iter */
        0,                                  /* tp_iternext */
        DisjointSet_methods,                /* tp_methods */
        0,                                  /* tp_members */
        0,                                  /* tp_getset */
        0,                                  /* tp_base */
        0,                                  /* tp_dict */
        0,                                  /* tp_descr_get */
        0,                                  /* tp_descr_set */
        0,                                  /* tp_dictoffset */
        (initproc) DisjointSet_init,        /* tp_init */
        PyType_GenericAlloc,                /* tp_alloc */
    };

    typedef struct {
        PyObject_HEAD
        UndirectedGraph<int>* g;

        // FIXME
        IotaLabeler* labeler;
        NoWeighter* weighter;
    } UGObj;

    static void UG_dealloc(UGObj* self) {
        if (self->g) delete self->g;
        if (self->labeler) delete self->labeler;
        if (self->weighter) delete self->weighter;
        self->ob_type->tp_free((PyObject*)self);
    }

    static int UG_init (
        UGObj *self,
        PyObject *args,
        PyObject *kwds
    ) {
        int sz;
        if (!PyArg_ParseTuple(args, "i", &sz))
            return -1;
        if (self->g) {
            // Someone who feels playful could call __init__() twice
            delete self->g;
            delete self->labeler;
            delete self->weighter;
            self->g = NULL;
        }
        try {
            self->labeler = new IotaLabeler();
            self->weighter = new NoWeighter();
            self->g = new UndirectedGraph<int>(sz, *(self->labeler), *(self->weighter));
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return -1;
        }
        return 0;
    }

    static PyObject* UG_str(PyObject* self) {
        const std::string& repr = ((UGObj*)self)->g->to_string();
        return PyString_FromStringAndSize(repr.c_str(), repr.size()-1);
    }

    static PyObject* UG_add_edge (
        UGObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        int a, b;
        if (!PyArg_ParseTuple(args, "ii", &a, &b))
            return NULL;
        try {
            self->g->add_edge(a, b);
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* UG_add_edges (
        UGObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        int M;
        if (!PyArg_ParseTuple(args, "i", &M))
            return NULL;
        try {
            self->g->add_edges(M);
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* UG_build_forest (
        UGObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        int M;
        if (!PyArg_ParseTuple(args, "i", &M))
            return NULL;
        try {
            self->g->build_forest(M);
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* UG_connect(UGObj* self) {
        try {
            self->g->connect();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* UG_build_path(UGObj* self) {
        try {
            self->g->build_path();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* UG_build_cycle(UGObj* self) {
        try {
            self->g->build_cycle();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* UG_build_tree(UGObj* self) {
        try {
            self->g->build_tree();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* UG_build_star(UGObj* self) {
        try {
            self->g->build_star();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* UG_build_wheel(UGObj* self) {
        try {
            self->g->build_wheel();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* UG_build_clique(UGObj* self) {
        try {
            self->g->build_clique();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyMethodDef UG_methods[] = {
        {"add_edge", (PyCFunction)UG_add_edge, METH_VARARGS,
         "Add an edge to the graph."},
        {"add_edges", (PyCFunction)UG_add_edges, METH_VARARGS,
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
        DirectedGraph<int>* g;

        // FIXME
        IotaLabeler* labeler;
        NoWeighter* weighter;        
    } DGObj;

    static void DG_dealloc(DGObj* self) {
        if (self->g) delete self->g;
        if (self->labeler) delete self->labeler;
        if (self->weighter) delete self->weighter;
        self->ob_type->tp_free((PyObject*)self);
    }

    static int DG_init (
        DGObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        int sz;
        if (!PyArg_ParseTuple(args, "i", &sz))
            return -1;
        if (self->g) {
            // Someone who feels playful could call __init__() twice
            delete self->g;
            delete self->labeler;
            delete self->weighter;
            self->g = NULL;
        }
        try {
            self->labeler = new IotaLabeler();
            self->weighter = new NoWeighter();
            self->g = new DirectedGraph<int>(sz, *(self->labeler), *(self->weighter));
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return -1;
        }
        return 0;
    }

    static PyObject* DG_str(PyObject* self) {
        const std::string& repr = ((DGObj*)self)->g->to_string();
        return PyString_FromStringAndSize(repr.c_str(), repr.size()-1);
    }

    static PyObject* DG_add_edge (
        DGObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        int a, b;
        if (!PyArg_ParseTuple(args, "ii", &a, &b))
            return NULL;
        try {
            self->g->add_edge(a, b);
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* DG_add_edges (
        DGObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        int M;
        if (!PyArg_ParseTuple(args, "i", &M))
            return NULL;
        try {
            self->g->add_edges(M);
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* DG_build_forest(
        DGObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        int M;
        if (!PyArg_ParseTuple(args, "i", &M))
            return NULL;
        try {
            self->g->build_forest(M);
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* DG_connect(DGObj* self) {
        try {
            self->g->connect();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* DG_build_path(DGObj* self) {
        try {
            self->g->build_path();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* DG_build_cycle(DGObj* self) {
        try {
            self->g->build_cycle();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* DG_build_tree(DGObj* self) {
        try {
            self->g->build_tree();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* DG_build_star(DGObj* self) {
        try {
            self->g->build_star();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* DG_build_wheel(DGObj* self) {
        try {
            self->g->build_wheel();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyObject* DG_build_clique(DGObj* self) {
        try {
            self->g->build_clique();
        }
        catch(std::exception& e) {
            PyErr_SetString(PyExc_ValueError, e.what());
            return NULL;
        }
        Py_RETURN_NONE;
    }

    static PyMethodDef DG_methods[] = {
        {"add_edge", (PyCFunction)DG_add_edge, METH_VARARGS,
         "Add an edge to the graph."},
        {"add_edges", (PyCFunction)DG_add_edges, METH_VARARGS,
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
        0,                                  /* ob_size */
        "graphgen.DirectedGraph",           /* tp_name */
        sizeof(DGObj),                      /* tp_basicsize */
        0,                                  /* tp_itemsize */
        (destructor) DG_dealloc,            /* tp_dealloc */
        0,                                  /* tp_print */
        0,                                  /* tp_getattr */
        0,                                  /* tp_setattr */
        0,                                  /* tp_compare */
        0,                                  /* tp_repr */
        0,                                  /* tp_as_number */
        0,                                  /* tp_as_sequence */
        0,                                  /* tp_as_mapping */
        0,                                  /* tp_hash */
        0,                                  /* tp_call */
        DG_str,                             /* tp_str */
        0,                                  /* tp_getattro */
        0,                                  /* tp_setattro */
        0,                                  /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                 /* tp_flags */
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
        RangeSamplerType.tp_new = PyType_GenericNew;
        RSIteratorType.tp_new = PyType_GenericNew;
        DisjointSetType.tp_new = PyType_GenericNew;
        UGType.tp_new = PyType_GenericNew;
        DGType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&RangeSamplerType) < 0)
            return;
        if (PyType_Ready(&RSIteratorType) < 0)
            return;
        if (PyType_Ready(&DisjointSetType) < 0)
            return;
        if (PyType_Ready(&UGType) < 0)
            return;
        if (PyType_Ready(&DGType) < 0)
            return;

        m = Py_InitModule3("graphgen", graphgen_methods,
                           "Module to generate graphs");

        Py_INCREF(&RangeSamplerType);
        Py_INCREF(&RSIteratorType);
        PyModule_AddObject(m, "RangeSampler", (PyObject *)&RangeSamplerType);
        PyModule_AddObject(m, "RangeSamplerIterator", (PyObject *)&RSIteratorType);
        PyModule_AddObject(m, "DisjointSet", (PyObject *)&DisjointSetType);
        PyModule_AddObject(m, "UndirectedGraph", (PyObject *)&UGType);
        PyModule_AddObject(m, "DirectedGraph", (PyObject *)&DGType);
    }
}
