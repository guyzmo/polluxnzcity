from setuptools import setup, find_packages
import os

import re
VERSIONFILE="pollux/_version.py"
verstrline = open(VERSIONFILE, "rt").read()
VSRE = r"^__version__ = ['\"]([^'\"]*)['\"]"
mo = re.search(VSRE, verstrline, re.M)
if mo:
    version = mo.group(1)
else:
    raise RuntimeError("Unable to find version string in %s." % (VERSIONFILE,))

def read(*names):
    values = dict()
    for name in names:
        if os.path.isfile(name):
            value = open(name).read()
        else:
            value = ''
        values[name] = value
    return values

long_description="""
Pollux Gateway Configuration

%(README)s

h1. News

%(CHANGES)s

""" % read('README', 'CHANGES')

setup(name='pollux',
      version=version,
      description="Pollux Gateway Configurator",
      long_description=long_description,
      classifiers=[], # Get strings from http://pypi.python.org/pypi?%3Aaction=list_classifiers
      keywords='',
      author='Kasey and Guyzmo, CKAB',
      author_email='contact@ckab.net',
      url='http://ckab.net',
      license='GPLv2',
      packages=find_packages(exclude=['ez_setup', 'examples', 'tests']),
      include_package_data=True,
      package_data={'': ['*.js', '*.css', '*.png', '*.tpl']},
      namespace_packages = ['pollux'],
      zip_safe=False,
      install_requires=[
          # -*- Extra requirements: -*-
	  "bottle"
      ],
      entry_points="""
      # -*- Entry points: -*-
      [console_scripts]
      pollux_config_paster = pollux.pollux_config:run_app
      pollux_config_lighttpdconf = pollux.pollux_config:get_lighttpd_configuration
      """,
      )
