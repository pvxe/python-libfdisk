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
	struct fdisk_label		*lb;
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

extern PyTypeObject ContextType;
extern PyTypeObject PartitionType;

extern void Context_AddModuleObject(PyObject *mod);
extern void Label_AddModuleObject(PyObject *mod);

extern PyObject *PyObjectResultStr(const char *s);
extern PyObject *PyObjectResultLabel(struct fdisk_label *lb);
extern PyObject *PyObjectResultPartition(struct fdisk_partition *pa);

#endif
