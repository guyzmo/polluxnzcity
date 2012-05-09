/*
 * Pollux'NZ City source code
 *
 * (c) 2012 CKAB / hackable:Devices
 * (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <pollux/pollux_extension.h>
#include <Python.h>
#include <iostream>

using namespace pollux;
/** Initializes the python environment to load the extensions 
    *
    * @param path: string containing the path to the extension directory
    *
    * */
PolluxExtension::PolluxExtension(const std::string& path) {
    std::ostringstream python_path;
    python_path<<"sys.path.append(\""<<path<<"/extensions/datastores/\")";

    // Initialize the Python Interpreter
    Py_Initialize();

    PyRun_SimpleString("import sys");
    PyRun_SimpleString(python_path.str().c_str());

}
/** stops the python environment */
PolluxExtension::~PolluxExtension() {
    Py_Finalize();
}

/** executes the module module 
    *
    * @param module: string containing the name of the module
    * @param configuration_map: configuration map matching the module to be launched
    * @param values_list: list of maps containing all the measures
    * @return integer: <0 module error, >0 success, =0 couldn't load module
    *
    * the name of the module shall match configuration file's 
    * datastore item and the name of the python module in the
    * extension's directory
    *
    * the measures are a list of maps having the following nomenclature:
    * "k" : "name of the measure" [string]
    * "u" : "unit of the measure" [string]
    * "v" : "value of the measure" [string containing float]
    * "p" : "precision of the measure" [string containing float]
    */
int PolluxExtension::push_to_datastore(const std::string& module, 
                        string_string_map& configuration_map,
                        std::vector<string_string_map*>& values_list) {

    PyObject *pName, *pModule, *pDict, *pFunc, *pValue, *pArgs, *pValues=NULL, *pConfig=NULL, *k, *v;
    
    int ret = 0;

    // Build the name object
    pName = PyString_FromString(module.c_str());

    // Load the module object
    pModule = PyImport_Import(pName);

    if (pModule == NULL) {
        Py_DECREF(pName);
        return 0;
    }

    // pDict is a borrowed reference 
    pDict = PyModule_GetDict(pModule);

    if (pDict == NULL) {
        Py_DECREF(pName);
        Py_DECREF(pModule);
        return 0;
    }

    // pFunc is also a borrowed reference 
    pFunc = PyDict_GetItemString(pDict, "push_to_datastore");

    if (pFunc == NULL) {
        Py_DECREF(pName);
        Py_DECREF(pModule);
        Py_DECREF(pDict);
        return 0;
    }


    if (PyCallable_Check(pFunc)) {
        pArgs = PyTuple_New(2);

        // values
        pValues = PyList_New(0);
        for (std::vector<string_string_map*>::iterator it=values_list.begin(); it != values_list.end(); ++it) {
            pValue = PyDict_New();
            for (string_string_map::iterator val_it=(*it)->begin(); val_it != (*it)->end(); ++val_it) {
                k = PyString_FromString(val_it->first.c_str());
                v = PyString_FromString(val_it->second.c_str());
                PyDict_SetItem(pValue, k, v);
#ifdef VERBOSE
                printf("pValues: %p, pValue: %p ; k: %s [%p] ; v: %s [%p]\n", pValues,  pValue, val_it->first.c_str(), k, val_it->second.c_str(), v);
#endif
            }
            PyList_Append(pValues, pValue);
        }
        pValue = NULL;
        PyTuple_SetItem(pArgs, 0, pValues);

        // config
        pConfig = PyDict_New();
        for (string_string_map::iterator it=configuration_map.begin(); it != configuration_map.end(); ++it) {
            k = PyString_FromString(it->first.c_str());
            v = PyString_FromString(it->second.c_str());
            PyDict_SetItem(pConfig, k, v);
#ifdef VERBOSE
            printf("pConfig: %p ; k: %p/%s ; v: %p/%s\n", pFunc, k, it->first.c_str(), v, it->second.c_str());
#endif
        }
        PyTuple_SetItem(pArgs, 1, pConfig);

        pValue = PyObject_CallObject(pFunc, pArgs);

        if (pArgs != NULL) {
            Py_DECREF(pArgs);
        }

        if (pValue != NULL) {
#ifdef VERBOSE
            printf("Return of call : %d\n", PyInt_AsLong(pValue));
#endif
            ret = PyInt_AsLong(pValue);
            Py_DECREF(pValue);
        } else {
            PyErr_Print();
            ret = 0;
        }
    }

    return ret;
}
