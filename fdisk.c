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


#include <dirent.h>

#include "fdisk.h"

void *set_PyErr_from_rc(int e)
{
	switch (e) {
	case ENOMEM:
		PyErr_SetString(PyExc_MemoryError, strerror(e));
		break;
	case EINVAL:
		PyErr_SetString(PyExc_TypeError, strerror(e));
		break;
	case ENOSYS:
		PyErr_SetString(PyExc_NotImplementedError, strerror(e));
		break;
	default:
		PyErr_SetString(PyExc_Exception, strerror(e));
	}

	return NULL;
}

PyObject *PyObjectResultStr(const char *s)
{
	PyObject *result;
	if (!s)
		/* XXX: return Py_BuildValue("s", ""); */
		Py_RETURN_NONE;
	result = Py_BuildValue("s", s);
	if (!result)
		PyErr_SetString(PyExc_RuntimeError, CONSTRUCT_ERR);
	return result;
}

static PyMethodDef FdiskMethods[] = {
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef fdiskmodule = {
    PyModuleDef_HEAD_INIT,
    "fdisk",   /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    FdiskMethods
};

PyMODINIT_FUNC
PyInit_fdisk(void)
{
	PyObject *m = PyModule_Create(&fdiskmodule);

	PyModule_AddIntConstant(m, "FDISK_SIZEUNIT_BYTES", FDISK_SIZEUNIT_BYTES);
	PyModule_AddIntConstant(m, "FDISK_SIZEUNIT_HUMAN", FDISK_SIZEUNIT_HUMAN);

	PyModule_AddIntConstant(m, "FDISK_FIELD_DEVICE", FDISK_FIELD_DEVICE);
	PyModule_AddIntConstant(m, "FDISK_FIELD_SIZE", FDISK_FIELD_SIZE);
	PyModule_AddIntConstant(m, "FDISK_FIELD_TYPE", FDISK_FIELD_TYPE);
	PyModule_AddIntConstant(m, "FDISK_FIELD_TYPEID", FDISK_FIELD_TYPEID);
	PyModule_AddIntConstant(m, "FDISK_FIELD_FSTYPE", FDISK_FIELD_FSTYPE);

	PyModule_AddIntConstant(m, "FDISK_DISKLABEL_DOS", FDISK_DISKLABEL_DOS);
	PyModule_AddIntConstant(m, "FDISK_DISKLABEL_GPT", FDISK_DISKLABEL_GPT);
	PyModule_AddIntConstant(m, "FDISK_ITER_FORWARD", FDISK_ITER_FORWARD);
	PyModule_AddIntConstant(m, "FDISK_ITER_BACKWARD", FDISK_ITER_BACKWARD);

	Context_AddModuleObject(m);
	Label_AddModuleObject(m);
	Partition_AddModuleObject(m);
	PartType_AddModuleObject(m);


	return m;
}
