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


static PyMemberDef Partition_members[] = {
	{ NULL }
};


static void Partition_dealloc(PartitionObject *self)
{
	if (self->pa)
		fdisk_unref_partition(self->pa);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *Partition_new(PyTypeObject *type,
			 PyObject *args __attribute__((unused)),
			 PyObject *kwds __attribute__((unused)))
{
	PartitionObject *self = (PartitionObject*) type->tp_alloc(type, 0);

	if (self)
		self->pa = NULL;

	return (PyObject *)self;
}

#define Partition_HELP "Partition()"
static int Partition_init(PartitionObject *self, PyObject *args, PyObject *kwds)
{
	/*
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
	*/

	self->pa = fdisk_new_partition();

	return 0;
}


static PyMethodDef Partition_methods[] = {
	{NULL}
};


static PyObject *Partition_get_partno(PartitionObject *self)
{
	if (fdisk_partition_has_partno(self->pa)) {
		return PyLong_FromSize_t(fdisk_partition_get_partno(self->pa));
	}
	Py_RETURN_NONE;
}
static PyObject *Partition_get_size(PartitionObject *self)
{
	if (fdisk_partition_has_size(self->pa)) {
		return PyLong_FromUnsignedLongLong(fdisk_partition_get_size(self->pa));
	}
	Py_RETURN_NONE;
}
static PyGetSetDef Partition_getseters[] = {
	{"partno",	(getter)Partition_get_partno, NULL, "partition number", NULL},
	{"size",	(getter)Partition_get_size, NULL, "number of sectors", NULL},
	{NULL}
};

static PyObject *Partition_repr(PartitionObject *self)
{
	return PyUnicode_FromFormat("<libfdisk.Partition object at %p>",
			self);
}

PyTypeObject PartitionType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "libfdisk.Partition",
	.tp_basicsize = sizeof(PartitionObject),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor)Partition_dealloc,
	.tp_repr = (reprfunc) Partition_repr,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = PyDoc_STR(Partition_HELP),
	.tp_methods = Partition_methods,
	.tp_members = Partition_members,
	.tp_getset = Partition_getseters,
	.tp_init = (initproc)Partition_init,
	.tp_new = Partition_new,
};

PyObject *PyObjectResultPartition(struct fdisk_partition *pa)
{
        PartitionObject *result;

        if (!pa) {
                PyErr_SetString(PyExc_AssertionError, "pa assert failed");
                return NULL;
        }

        result = PyObject_New(PartitionObject, &PartitionType);
        if (!result) {
                PyErr_SetString(PyExc_MemoryError, "Couldn't allocate Partition object");
                return NULL;
        }

        result->pa = pa;
        return (PyObject *) result;
}

void Partition_AddModuleObject(PyObject *mod)
{
	if (PyType_Ready(&PartitionType) < 0)
		return;

	Py_INCREF(&PartitionType);
	PyModule_AddObject(mod, "Partition", (PyObject *)&PartitionType);
}
