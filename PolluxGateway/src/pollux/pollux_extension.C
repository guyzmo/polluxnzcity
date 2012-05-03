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

#include <pollux/types.h>
#include <sstream>

#include <Python.h>

namespace "pollux" {
/** Initializes the python environment to load the extensions 
    *
    * @param path: string containing the path to the extension directory
    *
    * */
PolluxExtension::PolluxExtension(std::string& path) {
    std::ostringstream python_path;
    python_path << "sys.path.append(\"" << path << "\")";
    append_python_path = python_path.str();

    // Initialize the Python Interpreter
    Py_Initialize();

    PyRun_SimpleString("import sys");
    PyRun_SimpleString(append_python_path.c_str());

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
int PolluxExtension::push_to_datastore(std::string& module, 
                        string_string_map configuration_map,
                        std::vector<string_string_map*> values_list) {

    PyObject *pName, *pModule, *pDict, *pFunc, *pValue, *pArgs, *pValues, *pConfig, *k, *v;
    
    int ret = 0;

    // Build the name object
    pName = PyString_FromString(module);

    // Load the module object
    pModule = PyImport_Import(pName);

    // pDict is a borrowed reference 
    pDict = PyModule_GetDict(pModule);

    // pFunc is also a borrowed reference 
    pFunc = PyDict_GetItemString(pDict, "push_to_datastore");

    if (PyCallable_Check(pFunc)) {
        pArgs = PyTuple_New(2);

        // values
        pValues = PyList_New(0);
        for (std::vector<string_string_map*>::iterator it=values_list.begin(); it != values_list.end(); ++it) {
            val = PyDict_New();
            for (string_string_map::iterator val_it=(*it)->begin(); val_it != (*it)->end(); ++val_it) {
                k = PyString_FromString(val_it->first.c_str());
                v = PyString_FromString(val_it->second.c_str());
                PyDict_SetItem(val, k, v);
#ifdef VERBOSE
                printf("pValues: %04X, val: %04X ; k: %s [%04X] ; v: %s [%04X]\n", (unsigned int)pValues, (unsigned int) val, val_it->first.c_str(), (unsigned int)k, val_it->second.c_str(), (unsigned int)v);
#endif
            }
            PyList_Append(pValues, val);
        }
        PyTuple_SetItem(pArgs, 0, pValues);

        // config
        pConfig = PyDict_New();
        for (string_string_map::iterator it=configuration_map.begin(); it != configuration_map.end(); ++it) {
            k = PyString_FromString(it->first.c_str());
            v = PyString_FromString(it->second.c_str());
            PyDict_SetItem(pConfig, k, v);
#ifdef VERBOSE
            printf("pConfig: %04X ; k: %04X/%s ; v: %04X/%s\n", (unsigned int)pFunc, (unsigned int)k, it->first.c_str(), (unsigned int)v, it->second.c_str());
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
        }
    }

    // Clean up
    Py_DECREF(pConfig);
    Py_DECREF(pValues);
    Py_DECREF(pArgs);
    Py_DECREF(pDict);
    Py_DECREF(pModule);
    Py_DECREF(pName);

    return ret;
}

}
