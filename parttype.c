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

static PyMemberDef PartType_members[] = {
	{ NULL }
};

static void PartType_dealloc(PartTypeObject *self)
{
	Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyMethodDef PartType_methods[] = {
	{NULL}
};

static PyObject *PartType_get_name(PartTypeObject *self)
{
	return PyObjectResultStr(fdisk_parttype_get_name(self->type));
}

static PyObject *PartType_get_code(PartTypeObject *self)
{
	return PyLong_FromUnsignedLong(fdisk_parttype_get_code(self->type));
}

static PyGetSetDef PartType_getseters[] = {
	{"name",	(getter)PartType_get_name, NULL, "parttype human readable name", NULL},
	{"code",	(getter)PartType_get_code, NULL, "parttype DOS code", NULL},
	{NULL}
};

static PyObject *PartType_repr(PartTypeObject *self)
{
	return PyUnicode_FromFormat("<libfdisk.PartType object at %p, name=%s>",
			self, fdisk_parttype_get_name(self->type));
}

PyTypeObject PartTypeType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "libfdisk.PartType",
	.tp_basicsize = sizeof(PartTypeObject),
	.tp_dealloc = (destructor)PartType_dealloc,
	.tp_repr = (reprfunc) PartType_repr,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	/* TODO: .tp_doc = PartType_HELP, */
	.tp_methods = PartType_methods,
	.tp_members = PartType_members,
	.tp_getset = PartType_getseters,
};

PyObject *PyObjectResultPartType(struct fdisk_parttype *t)
{
        PartTypeObject *result;

        if (!t) {
                PyErr_SetString(PyExc_AssertionError, "lb assert failed");
                return NULL;
        }

        result = PyObject_New(PartTypeObject, &PartTypeType);
        if (!result) {
                PyErr_SetString(PyExc_MemoryError, "Couldn't allocate PartType object");
                return NULL;
        }

        result->type = t;
        return (PyObject *) result;
}

void PartType_AddModuleObject(PyObject *mod)
{
	if (PyType_Ready(&PartTypeType) < 0)
		return;

	Py_INCREF(&PartTypeType);
	PyModule_AddObject(mod, "PartType", (PyObject *)&PartTypeType);
}
