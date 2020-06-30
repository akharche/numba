from __future__ import absolute_import, print_function
import llvmlite.binding as ll
import os
from distutils import sysconfig


def init_jit():
    from numba.dppy.dispatcher import DPPyDispatcher
    return DPPyDispatcher

def initialize_all():
    from numba.targets.registry import dispatcher_registry
    dispatcher_registry.ondemand['dppy'] = init_jit

    ll.load_library_permanently('libdpglue.so')
    ll.load_library_permanently('libOpenCL.so')

def _initialize_ufunc():
    from numba.npyufunc import Vectorize

    def init_vectorize():
        from numba.dppy.vectorizers import OclVectorize

        return OclVectorize

    Vectorize.target_registry.ondemand['dppy'] = init_vectorize

def _initialize_gufunc():
    from numba.npyufunc import GUVectorize

    def init_guvectorize():
        from numba.dppy.vectorizers import OclGUFuncVectorize

        return OclGUFuncVectorize

    GUVectorize.target_registry.ondemand['dppy'] = init_guvectorize