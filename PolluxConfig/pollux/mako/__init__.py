# See http://peak.telecommunity.com/DevCenter/setuptools#namespace-packages
try: # pragma: no cover
    __import__('pkg_resources').declare_namespace(__name__)
except ImportError: # pragma: no cover
    from pkgutil import extend_path
    __path__ = extend_path(__path__, __name__)

# mako/__init__.py
# Copyright (C) 2006-2012 the Mako authors and contributors <see AUTHORS file>
#
# This module is part of Mako and is released under
# the MIT License: http://www.opensource.org/licenses/mit-license.php


__version__ = '0.6.2'
