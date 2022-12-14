from setuptools import setup, Extension

libfdisk = Extension('fdisk',
                    libraries = ['fdisk'],
                    sources = ['fdisk.c', 'context.c', 'label.c',
                               'partition.c', 'parttype.c'])

setup (name = 'libfdisk',
       version = '1.0',
       description = 'Python bindings for libfdisk',
       ext_modules = [libfdisk])
