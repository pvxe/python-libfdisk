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

static PyMemberDef Context_members[] = {
	{ NULL }
};

static void Context_dealloc(ContextObject *self)
{
	if (!self->cxt) /* if init fails */
		return;

	fdisk_unref_context(self->cxt);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *Context_new(PyTypeObject *type,
			 PyObject *args __attribute__((unused)),
			 PyObject *kwds __attribute__((unused)))
{
	ContextObject *self = (ContextObject*) type->tp_alloc(type, 0);

	if (self) {
		self->cxt = NULL;
		self->tb = NULL;
	}

	return (PyObject *)self;
}

#define Context_HELP "Context(device=None, details=True, readonly=False)"
static int Context_init(ContextObject *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = {
		"device", "details", "readonly",
		NULL
	};
	int details = 1, readonly = 0, rc = 0;
	char *device = NULL;

	if (!PyArg_ParseTupleAndKeywords(args,
					kwds, "|spp", kwlist,
					&device, &details, &readonly)) {
		PyErr_SetString(PyExc_TypeError, ARG_ERR);
		return -1;
	}

	if (self->cxt)
		fdisk_unref_context(self->cxt);

	self->cxt = fdisk_new_context();
	if (!self->cxt) {
		PyErr_SetString(PyExc_MemoryError, "Couldn't allocate context");
		return -1;
	}

	if (device && (rc = fdisk_assign_device(self->cxt, device, readonly))) {
		set_PyErr_from_rc(-rc);
		return -1;
	}
	if (details && (rc = fdisk_enable_details(self->cxt, details))) {
		set_PyErr_from_rc(-rc);
		return -1;
	}
	fdisk_get_partitions(self->cxt, &self->tb);

	return 0;
}

#define Context_assign_device_HELP "assign_device(device)\n\n" \
	"Open the device, discovery topology, geometry, detect disklabel " \
	"and switch the current label driver to reflect the probing result. "
static PyObject *Context_assign_device(ContextObject *self, PyObject *args, PyObject *kwds)
{
	char *fname;

	if (!PyArg_ParseTuple(args, "s", &fname)) {
		PyErr_SetString(PyExc_TypeError, ARG_ERR);
		return NULL;
	}

	/* Assert self->cxt */

	/* XXX: readonly */
	fdisk_assign_device(self->cxt, fname, 1);

	fdisk_get_partitions(self->cxt, &self->tb);

	Py_INCREF(Py_None);
	return Py_None;
}

#define Context_partition_to_string_HELP "partition_to_string(pa, field)\n\n" \
	"Retrieve partition field using fdisk_partition_to_string." \
	"Field constants are available as FDISK_LABEL_*"
static PyObject *Context_partition_to_string(ContextObject *self, PyObject *args, PyObject *kwds)
{
	struct fdisk_partition *pa;
	enum fdisk_fieldtype field;
	PartitionObject *part;
	PyObject *ret;
	char *data;

	if (!PyArg_ParseTuple(args, "O!i", &PartitionType, &part, &field)) {
		PyErr_SetString(PyExc_TypeError, ARG_ERR);
		return NULL;
	}

	pa = part->pa;

	fdisk_partition_to_string(pa, self->cxt, field, &data);
	ret = Py_BuildValue("s", data);
	free(data);

	return ret;
}

#define Context_create_disklabel_HELP "create_disklabel(label)\n\n" \
	"Creates a new disk label of type name . If name is NULL, " \
	"then it will create a default system label type, either SUN or DOS."
static PyObject *Context_create_disklabel(ContextObject *self, PyObject *args, PyObject *kwds)
{
	char *label_name = NULL;

	if (!PyArg_ParseTuple(args, "|s", &label_name)) {
		PyErr_SetString(PyExc_TypeError, ARG_ERR);
		return NULL;
	}

	if (fdisk_create_disklabel(self->cxt, label_name)) {
		PyErr_Format(PyExc_RuntimeError, "Error creating label %s", label_name);
		return NULL;
	}

	Py_RETURN_NONE;
}

#define Context_write_disklabel_HELP "write_disklabel()\n\n" \
	"This function wipes the device (if enabled by fdisk_enable_wipe()) " \
	"and then it writes in-memory changes to disk. Be careful!"
static PyObject *Context_write_disklabel(ContextObject *self, PyObject *args, PyObject *kwds)
{
	int ret;

	ret = fdisk_write_disklabel(self->cxt);
	if (ret < 0) {
		PyErr_Format(PyExc_RuntimeError, "Error writing label to disk: %s", strerror(-ret));
		return NULL;
	}

	Py_RETURN_NONE;
}

#define Context_add_partition_HELP "add_partition(fdisk.Partition)\n\n" \
	"Adds partition to context. Returns partno of the new partition."
static PyObject *Context_add_partition(ContextObject *self, PyObject *args, PyObject *kwds)
{
	PartitionObject *partobj;
	size_t partno;
	int rc;

	if (!PyArg_ParseTuple(args, "O!", &PartitionType, &partobj)) {
		PyErr_SetString(PyExc_TypeError, ARG_ERR);
		return NULL;
	}

	if (!partobj->pa) {
		PyErr_SetString(PyExc_TypeError, ARG_ERR);
		return NULL;
	}

	rc = fdisk_add_partition(self->cxt, partobj->pa, &partno);
	if (rc < 0) {
		PyErr_Format(PyExc_RuntimeError, "Error adding partition to context: %s", strerror(-rc));
		return NULL;
	}

	return Py_BuildValue("n", partno);
}

static PyMethodDef Context_methods[] = {
	{"assign_device",	(PyCFunction)Context_assign_device, METH_VARARGS, Context_assign_device_HELP},
	{"partition_to_string",	(PyCFunction)Context_partition_to_string, METH_VARARGS, Context_partition_to_string_HELP},
	{"create_disklabel",	(PyCFunction)Context_create_disklabel, METH_VARARGS, Context_create_disklabel_HELP},
	{"write_disklabel",	(PyCFunction)Context_write_disklabel, METH_NOARGS, Context_write_disklabel_HELP},
	{"add_partition",	(PyCFunction)Context_add_partition, METH_VARARGS, Context_add_partition_HELP},
	{NULL}
};

static PyObject *Context_get_nsectors(ContextObject *self)
{
	return PyLong_FromUnsignedLong(fdisk_get_nsectors(self->cxt));
}

static PyObject *Context_get_sector_size(ContextObject *self)
{
	return PyLong_FromUnsignedLong(fdisk_get_sector_size(self->cxt));
}

static PyObject *Context_get_devname(ContextObject *self)
{
	return PyObjectResultStr(fdisk_get_devname(self->cxt));
}

static PyObject *Context_get_label(ContextObject *self)
{
	struct fdisk_context *cxt = self->cxt;

	if (fdisk_has_label(cxt)) {
		return PyObjectResultLabel(fdisk_get_label(cxt, NULL));
	} else {
		Py_RETURN_NONE;
	}
}

static PyObject *Context_get_nparts(ContextObject *self)
{
	return PyLong_FromLong(fdisk_table_get_nents(self->tb));
}

static PyObject *Context_get_partitions(ContextObject *self)
{
	PyObject *p, *list = PyList_New(0); /* XXX: null if failed*/
	struct fdisk_partition *pa;
	struct fdisk_iter *itr;
	struct fdisk_table *tb;
	/* char *data; */
	
	tb = self->tb;
	itr = fdisk_new_iter(FDISK_ITER_FORWARD);

	while(fdisk_table_next_partition(tb, itr, &pa) == 0) {
		/* const char *name = fdisk_partition_get_name(pa);*/
		p = PyObjectResultPartition(pa);
		PyList_Append(list, p);
		/*free(data);*/
	}	

	fdisk_free_iter(itr);

	return list;
}

static PyObject *Context_get_size_unit(ContextObject *self)
{
	return PyLong_FromLong(fdisk_get_size_unit(self->cxt));
}

static int Context_set_size_unit(ContextObject *self, PyObject *value, void *closure)
{
	int szunit;

	if (value == NULL) {
		PyErr_SetString(PyExc_TypeError,
				"Cannot set unit size: null size type");
		return -1;
	}

	if (!PyLong_Check(value)) {
		PyErr_SetString(PyExc_TypeError,
				"Cannot set unit size: invalid size type");
		return -1;
	}

	szunit = (int) PyLong_AsLong(value);
	if (fdisk_set_size_unit(self->cxt, szunit) < 0) {
		PyErr_SetString(PyExc_TypeError,
				"Cannot set unit size: invalid size type value");
		return -1;
	}

	return 0;
}

static PyGetSetDef Context_getseters[] = {
	{"nsectors",	(getter)Context_get_nsectors, NULL, "context number of sectors", NULL},
	{"sector_size",	(getter)Context_get_sector_size, NULL, "context sector size", NULL},
	{"devname",	(getter)Context_get_devname, NULL, "context devname", NULL},
	{"label",	(getter)Context_get_label, NULL, "context label type", NULL},
	{"nparts",	(getter)Context_get_nparts, NULL, "context label number of existing partitions", NULL},
	{"partitions",	(getter)Context_get_partitions, NULL, "context partitions", NULL},
	{"size_unit",	(getter)Context_get_size_unit, (setter)Context_set_size_unit, "context unit size", NULL},
	{NULL}
};

static PyObject *Context_repr(ContextObject *self)
{
	PyObject *lbo = Py_None;

	if (fdisk_has_label(self->cxt))
		lbo = PyObjectResultLabel(fdisk_get_label(self->cxt, NULL));

	return PyUnicode_FromFormat("<libfdisk.Context object at %p, label=%R, details=%s, readonly=%s>",
				    self,
				    lbo,
				    fdisk_is_details(self->cxt) ? "True" : "False",
				    fdisk_is_readonly(self->cxt) ? "True" : "False");
}

PyTypeObject ContextType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "libfdisk.Context",
	.tp_basicsize = sizeof(ContextObject),
	.tp_dealloc = (destructor)Context_dealloc,
	.tp_repr = (reprfunc) Context_repr,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = Context_HELP,
	.tp_methods = Context_methods,
	.tp_members = Context_members,
	.tp_getset = Context_getseters,
	.tp_init = (initproc)Context_init,
	.tp_new = Context_new,
};

void Context_AddModuleObject(PyObject *mod)
{
	if (PyType_Ready(&ContextType) < 0)
		return;

	Py_INCREF(&ContextType);
	PyModule_AddObject(mod, "Context", (PyObject *)&ContextType);
}
