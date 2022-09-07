"""
Python interface to the PMLib library
"""
#
#  This file is part of Python Distributed Training of Neural Networks (PyDTNN)
#
#  Copyright (C) 2021-22 Universitat Jaume I
#
#  PyDTNN is free software: you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the Free Software
#  Foundation, either version 3 of the License, or (at your option) any later
#  version.
#
#  This program is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
#  License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program. If not, see <https://www.gnu.org/licenses/>.
#

import sys
import ctypes
import ctypes.util
from enum import Enum

if sys.platform in ('linux2', 'linux'):
    _libpmlib_libname_list = ['libpmlib.so']
elif sys.platform == 'darwin':
    _libpmlib_libname_list = ['libpmlib.dylib']
elif sys.platform == 'win32':
    _libpmlib_libname_list = ['libpmlib.dll']
else:
    raise SystemExit('PMLib: current platform is not yet supported!')

_libpmlib = None
for _libpmlib_libname in _libpmlib_libname_list:
    try:
        _libpmlib = ctypes.cdll.LoadLibrary(_libpmlib_libname)
    except OSError:
        pass
    else:
        break
if _libpmlib is None:
    raise OSError('PMLib library not found')

SERVER_IP_LEN = 16
MAX_TIMING    = 10 
LINE_SETSIZE  = 16
NLINEBITS     = 128

class SERVER(ctypes.Structure):
    _fields_ = [("server_ip", ctypes.c_char * SERVER_IP_LEN),
                ("port"     , ctypes.c_int)]

class LINES(ctypes.Structure):
    _fields_ = [("__bits", ctypes.c_char * LINE_SETSIZE)]

class PM_MEASURES(ctypes.Structure):
    _fields_ = [("watts_size"     , ctypes.c_int),
                ("watts_sets_size", ctypes.c_int),
                ("watts_sets"     , ctypes.POINTER(ctypes.c_int)),
                ("watts"          , ctypes.POINTER(ctypes.c_double)),
                ("lines_len"      , ctypes.c_int)]
    
class PM_MEASURES_WT(ctypes.Structure):
    _fields_ = [("next_timing", ctypes.c_int),
                ("timing"     , ctypes.POINTER(ctypes.c_double)),
                ("energy"     , PM_MEASURES)]
                
class COUNTER(ctypes.Structure):
    _fields_ = [("sock"     , ctypes.c_int), 
                ("aggregate", ctypes.c_int), 
                ("lines"    , LINES),
                ("num_lines", ctypes.c_int),
                ("interval" , ctypes.c_int),
                ("measures" , PM_MEASURES_WT)]                

  
class pmlibError(Exception):
    def __init__(self, error):
        self.error = error

    def __str__(self):
        return f'{self.error}'


# Helper functions

# int pm_set_server( char *ip, int port, server_t *pm_server);
_libpmlib.pm_set_server.restype = int
_libpmlib.pm_set_server.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.POINTER(SERVER)]

def pm_create_server(server_ip, port):
    """
    -------
    """
    server = SERVER()
    status = _libpmlib.pm_set_server(server_ip.encode('utf-8'), port, ctypes.byref(server))
    if status == 0:
       return server
    else:
       raise pmlibError("Error: pmlib_create_server returned an error\n")


# int pm_set_lines( char *lines_string ,line_t *lines );
_libpmlib.pm_set_lines.restype = int
_libpmlib.pm_set_lines.argtypes = [ctypes.c_char_p, ctypes.POINTER(LINES)]

def pm_create_lines(lines_string):
    """
    -------
    """
    lines = LINES()
    status = _libpmlib.pm_set_lines(lines_string.encode('utf-8'), ctypes.byref(lines))
    if status == 0:
        return lines
    else:
        raise pmlibError("Error: pmlib_create_lines returned an error\n")

   
# int pm_create_counter(char *pm_id, line_t lines, int aggregate, int interval, server_t pm_server, counter_t *pm_counter);
_libpmlib.pm_create_counter.restype  = int
_libpmlib.pm_create_counter.argtypes = [ctypes.c_char_p, LINES, ctypes.c_int, ctypes.c_int, SERVER, ctypes.POINTER(COUNTER)]

def pm_create_counter(counter_string, lines, aggregate, interval, server):
    """
    -------
    """
    counter = COUNTER()
    status = _libpmlib.pm_create_counter(counter_string.encode('utf-8'), lines, aggregate, interval, server, ctypes.byref(counter))
    if status == 0:
        return counter
    else:
        raise pmlibError("Error: pmlib_create_counter returned an error\n")


# int pm_start_counter( counter_t *pm_counter );
_libpmlib.pm_start_counter.restype = int
_libpmlib.pm_start_counter.argtypes = [ctypes.POINTER(COUNTER)]

def pm_start_counter(counter):
    """
    ----------
    """    
    status = _libpmlib.pm_start_counter(ctypes.byref(counter))    
    return status


# int pm_stop_counter( counter_t *pm_counter );
_libpmlib.pm_stop_counter.restype = int
_libpmlib.pm_stop_counter.argtypes = [ctypes.POINTER(COUNTER)]

def pm_stop_counter(counter):
    """
    ----------
    """    
    status = _libpmlib.pm_stop_counter(ctypes.byref(counter))    
    return status


# int pm_get_counter_data( counter_t *pm_counter );
_libpmlib.pm_get_counter_data.restype  = int
_libpmlib.pm_get_counter_data.argtypes = [ctypes.POINTER(COUNTER)]

def pm_get_counter_data(counter):
    """
    ----------
    """    
    status = _libpmlib.pm_get_counter_data(ctypes.byref(counter))    
    return status


# int pm_print_data_text(char *file_name,  counter_t pm_counter, line_t lines, int set);
_libpmlib.pm_print_data_text.restype  = int
_libpmlib.pm_print_data_text.argtypes = [ctypes.c_char_p, COUNTER, LINES, ctypes.c_int]

def pm_print_data_text(output_string, counter, lines, set_value):
    """
    -------
    """
    status = _libpmlib.pm_print_data_text(output_string.encode('utf-8'), counter, lines, set_value)
    return status


# int pm_get_counter_data( counter_t *pm_counter );
_libpmlib.pm_finalize_counter.restype  = int
_libpmlib.pm_finalize_counter.argtypes = [ctypes.POINTER(COUNTER)]

def pm_finalize_counter(counter):
    """
    ----------
    """    
    status = _libpmlib.pm_finalize_counter(ctypes.byref(counter))
    if status == 0:
        return status
    else:
        raise pmlibError("Error: pmlib_create_counter returned an error\n")

