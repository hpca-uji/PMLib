import sys
import os
import ctypes
from ctypes.util import find_library
from importlib import import_module

def load_library(name):
    """
    Loads an external library using ctypes.CDLL.

    It searches the library using ctypes.util.find_library(). If the library is
    not found, it traverses the LD_LIBRARY_PATH until it finds it. If it is not
    in any of the LD_LIBRARY_PATH paths, an ImportError exception is raised.

    Parameters
    ----------
    name : str
        The library name without any prefix like lib, suffix like .so, .dylib or
        version number (this is the form used for the posix linker option -l).

    Returns
    -------
    The loaded library.
    """
    path = find_library(name)
    if path is None:
        if sys.platform in ('linux2', 'linux'):
            full_name = f"lib{name}.so"
        elif sys.platform == 'darwin':
            full_name = f"lib{name}.dylib"
        elif sys.platform == 'win32':
            full_name = f"lib{name}.dll"
        else:
            raise SystemExit(f"Trying to load '{name}' library, but platform '{sys.platform}' is not yet supported!")
        current_path=os.getcwd()
        if os.path.exists(os.path.join(current_path, full_name)):
                path = os.path.join(current_path, full_name)    
        else:
          for current_path in os.environ.get('LD_LIBRARY_PATH', '').split(':'):
            if os.path.exists(os.path.join(current_path, full_name)):
                path = os.path.join(current_path, full_name)
                break
          else:
            # Didn't find the library
            raise ImportError(f"Library '{full_name}' could not be found. Please add its path to LD_LIBRARY_PATH "
                              f"using 'export LD_LIBRARY_PATH={name.upper()}_LIB_PATH:$LD_LIBRARY_PATH' and "
                              f"then call this application again.")
    return ctypes.CDLL(path)

