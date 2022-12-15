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

#define Partition_HELP 	"Partition(partno_follow_default=False, " \
			"start_follow_default=False, " \
			"end_follow_default=False)"
static int Partition_init(PartitionObject *self, PyObject *args, PyObject *kwds)
{
	char *kwlist[] = {
		"partno_follow_default",
		"start_follow_default",
		"end_follow_default",
		NULL
	};
	int partno_follow_default = 0,
	    start_follow_default = 0,
	    end_follow_default = 0,
	    rc;

	if (!PyArg_ParseTupleAndKeywords(args,
					kwds, "|ppp", kwlist,
					&partno_follow_default,
					&start_follow_default,
					&end_follow_default)) {
		PyErr_SetString(PyExc_TypeError, "Invalid arguments for new partition");
		return -1;
	}

	self->pa = fdisk_new_partition();
	if ((rc = fdisk_partition_partno_follow_default(self->pa, partno_follow_default) < 0)) {
		set_PyErr_from_rc(-rc);
		return -1;
	}
	if ((rc = fdisk_partition_start_follow_default(self->pa, start_follow_default) < 0)) {
		set_PyErr_from_rc(-rc);
		return -1;
	}
	if ((rc = fdisk_partition_end_follow_default(self->pa, end_follow_default) < 0)) {
		set_PyErr_from_rc(-rc);
		return -1;
	}

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

static int Partition_set_partno(PartitionObject *self, PyObject *value, void *closure)
{
	size_t num;

	if (value == NULL) {
		fdisk_partition_unset_partno(self->pa);
		return 0;
	}

	if (!PyLong_Check(value)) {
		PyErr_SetString(PyExc_TypeError, ARG_ERR);
		return -1;
	}
	num = PyLong_AsSize_t(value);
	if (fdisk_partition_set_partno(self->pa, num) < 0) {
		PyErr_SetString(PyExc_TypeError,
				"libfdisk reported error setting partno");
		return -1;
	}

	return 0;
}

static PyObject *Partition_get_size(PartitionObject *self)
{
	if (fdisk_partition_has_size(self->pa)) {
		return PyLong_FromUnsignedLongLong(fdisk_partition_get_size(self->pa));
	}
	Py_RETURN_NONE;
}

static int Partition_set_size(PartitionObject *self, PyObject *value, void *closure)
{
	uint64_t sectors;

	if (value == NULL) {
		fdisk_partition_unset_size(self->pa);
		return 0;
	}

	if (!PyLong_Check(value)) {
		PyErr_SetString(PyExc_TypeError,
				ARG_ERR);
		return -1;
	}
	sectors = PyLong_AsUnsignedLongLong(value);
	if (fdisk_partition_set_size(self->pa, sectors) < 0) {
		PyErr_SetString(PyExc_TypeError,
				"libfdisk reported error setting partition size");
		return -1;
	}

	return 0;
}

static PyObject *Partition_get_type(PartitionObject *self)
{
	struct fdisk_parttype *t;

	t = fdisk_partition_get_type(self->pa);
	if (t)
		return PyObjectResultPartType(t);

	Py_RETURN_NONE;
}

static int Partition_set_type(PartitionObject *self, PyObject *value, void *closure)
{
	if (!value) {
		PyErr_SetString(PyExc_TypeError,
				"partition type assertion error");
		return -1;
	}
	if (!PyObject_TypeCheck(value, &PartTypeType)) {
		PyErr_SetString(PyExc_TypeError,
				"invalid partition type");
		return -1;
	}
	if (fdisk_partition_set_type(self->pa, ((PartTypeObject *) value)->type) < 0) {
		PyErr_SetString(PyExc_TypeError,
				"libfdisk reported error setting partition type");
		return -1;
	}

	return 0;
}

static PyGetSetDef Partition_getseters[] = {
	{"partno",	(getter)Partition_get_partno, (setter)Partition_set_partno, "partition number", NULL},
	{"size",	(getter)Partition_get_size, (setter)Partition_set_size, "number of sectors", NULL},
	{"type",	(getter)Partition_get_type, (setter)Partition_set_type, "number of sectors", NULL},
	{NULL}
};

static PyObject *Partition_repr(PartitionObject *self)
{
	size_t partno;

	if (fdisk_partition_has_partno(self->pa)) {
		partno = fdisk_partition_get_partno(self->pa);
		return PyUnicode_FromFormat("<libfdisk.Partition object at %p, partno=%zu>",
				self, partno);
	}

	return PyUnicode_FromFormat("<libfdisk.Partition object at %p, partno=None>",
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
