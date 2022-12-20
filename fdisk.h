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


#ifndef UTIL_LINUX_PYLIBFDISK_H
#define UTIL_LINUX_PYLIBFDISK_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include <libfdisk/libfdisk.h>

#define CONSTRUCT_ERR	"Error during object construction"
#define ARG_ERR		"Invalid number or type of arguments"

typedef struct {
	PyObject_HEAD
	struct fdisk_context		*cxt;
	struct fdisk_table		*tb;
} ContextObject;

typedef struct {
	PyObject_HEAD
	struct fdisk_label		*lb;
} LabelObject;

typedef struct {
	PyObject_HEAD
	struct fdisk_partition		*pa;
} PartitionObject;

typedef struct {
	PyObject_HEAD
	struct fdisk_parttype		*type;
} PartTypeObject;

extern PyTypeObject ContextType;
extern PyTypeObject PartitionType;
extern PyTypeObject PartTypeType;

extern void Context_AddModuleObject(PyObject *mod);
extern void Label_AddModuleObject(PyObject *mod);
extern void Partition_AddModuleObject(PyObject *mod);
extern void PartType_AddModuleObject(PyObject *mod);

extern PyObject *PyObjectResultStr(const char *s);
extern PyObject *PyObjectResultLabel(struct fdisk_label *lb);
extern PyObject *PyObjectResultPartition(struct fdisk_partition *pa);
extern PyObject *PyObjectResultPartType(struct fdisk_parttype *t);

extern void *set_PyErr_from_rc(int err);

#endif
