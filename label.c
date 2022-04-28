/*
 * (C) 2022 Soleta Consulting S.L. <info@soleta.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Author: Jose M. Guisado <jguisado@soleta.eu>
 */


#include "fdisk.h"


static PyMemberDef Label_members[] = {
	{ NULL }
};


static void Label_dealloc(LabelObject *self)
{
	Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *Label_new(PyTypeObject *type,
			 PyObject *args __attribute__((unused)),
			 PyObject *kwds __attribute__((unused)))
{
	LabelObject *self = (LabelObject*) type->tp_alloc(type, 0);

	if (self) {
		self->lb = NULL;
	}

	return (PyObject *)self;
}

#define Label_HELP "Label(context=None)"
static int Label_init(LabelObject *self, PyObject *args, PyObject *kwds)
{
	ContextObject *cxt = NULL;
	struct fdisk_label *lb;
	char *kwlist[] = {
		"context",
		NULL
	};
	
	if (!PyArg_ParseTupleAndKeywords(args,
					kwds, "|O!", kwlist,
					&ContextType, &cxt)) {
		PyErr_SetString(PyExc_TypeError, "Error");
		return -1;
	}

	if (cxt && (lb = fdisk_get_label(cxt->cxt, NULL))) {
		self->lb = lb;
	}

	return 0;
}


static PyMethodDef Label_methods[] = {
	{NULL}
};


static PyObject *Label_get_type(LabelObject *self)
{
	return PyLong_FromLong(fdisk_label_get_type(self->lb));
}
static PyObject *Label_get_name(LabelObject *self)
{
	return PyObjectResultStr(fdisk_label_get_name(self->lb));
}
static PyGetSetDef Label_getseters[] = {
	{"type",	(getter)Label_get_type, NULL, "label type", NULL},
	{"name",	(getter)Label_get_name, NULL, "label name", NULL},
	{NULL}
};

static PyObject *Label_repr(LabelObject *self)
{
	return PyUnicode_FromFormat("<libfdisk.Label object at %p, name=%s>",
			self, fdisk_label_get_name(self->lb));
}

PyTypeObject LabelType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "libfdisk.Label",
	.tp_basicsize = sizeof(LabelObject),
	.tp_dealloc = (destructor)Label_dealloc,
	.tp_repr = (reprfunc) Label_repr,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = Label_HELP,
	.tp_methods = Label_methods,
	.tp_members = Label_members,
	.tp_getset = Label_getseters,
	.tp_init = (initproc)Label_init,
	.tp_new = Label_new,
};

PyObject *PyObjectResultLabel(struct fdisk_label *lb)
{
        LabelObject *result;

        if (!lb) {
                PyErr_SetString(PyExc_AssertionError, "lb assert failed");
                return NULL;
        }


        result = PyObject_New(LabelObject, &LabelType);
        if (!result) {
                PyErr_SetString(PyExc_MemoryError, "Couldn't allocate Label object");
                return NULL;
        }

        /* Py_INCREF(result); */

        result->lb = lb;
        return (PyObject *) result;
}

void Label_AddModuleObject(PyObject *mod)
{
	if (PyType_Ready(&LabelType) < 0)
		return;

	Py_INCREF(&LabelType);
	PyModule_AddObject(mod, "Label", (PyObject *)&LabelType);
}
