#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <opencv2/opencv.hpp>
#include <numpy/arrayobject.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

int call_py(const char *fileName, const char *funcName, cv::Mat img1, cv::Mat img2)
{
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue, *output_array;
    import_array();

    npy_intp dims1[] = {img1.rows, img1.cols};
    npy_intp dims2[] = {img2.rows, img2.cols};
    uint8_t *ptr1 = img1.ptr<uint8_t>(0);
    uint8_t *ptr2 = img2.ptr<uint8_t>(0);

    // in_array_1 = PyArray_SimpleNewFromData(2, dims1, NPY_UINT8, ptr1);
    // in_array_2 = PyArray_SimpleNewFromData(2, dims2, NPY_UINT8, ptr2);

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\".\")");
    pName = PyUnicode_DecodeFSDefault(fileName);
    /* Error checking of pName left out */

    // std::cout << "Finished initialization...\n";

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, funcName);
        /* pFunc is a new reference */

        if (pFunc && PyCallable_Check(pFunc)) {
            pArgs = PyTuple_New(2);
            // pValue = PyUnicode_FromString("arg1");
            pValue = PyArray_SimpleNewFromData(2, dims1, NPY_UINT8, ptr1);
            // std::cout << "&&&&&&&&&&&&&&&&\n";

            if (!pValue) {
                Py_DECREF(pArgs);
                Py_DECREF(pModule);
                fprintf(stderr, "Cannot convert argument\n");
                return 1;
            }
            /* pValue reference stolen here: */
            PyTuple_SetItem(pArgs, 0, pValue);
            // pValue = PyUnicode_FromString("arg2");
            pValue = PyArray_SimpleNewFromData(2, dims2, NPY_UINT8, ptr2);

            if (!pValue) {
                Py_DECREF(pArgs);
                Py_DECREF(pModule);
                fprintf(stderr, "Cannot convert argument\n");
                return 1;
            }
            /* pValue reference stolen here: */
            PyTuple_SetItem(pArgs, 1, pValue);
            // std::cout << "Ready to call function\n";
            pValue = PyObject_CallObject(pFunc, pArgs);

            double val1 = PyFloat_AsDouble(PyList_GetItem(pValue, 0));
            double val2 = PyFloat_AsDouble(PyList_GetItem(pValue, 1));

            std::cout << "value1 : " << val1 << "\n";
            std::cout << "value2 : " << val2 << "\n";

            Py_DECREF(pArgs);
            if (pValue != NULL) {
                printf("Result of call: %ld\n", PyLong_AsLong(pValue));
                Py_DECREF(pValue);
            }
            else {
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                PyErr_Print();
                fprintf(stderr,"Call failed\n");
                return 1;
            }
        }
        else {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Cannot find function \"%s\"\n", funcName);
        }
        Py_XDECREF(pFunc);
        // Py_XDECREF(output_array);
        Py_DECREF(pModule);
    }
    else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", fileName);
        return 1;
    }
    return 0;
}

int main()
{
    Py_Initialize();

    cv::Mat img1 = cv::imread("image1.jpg", cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = cv::imread("image2.jpg", cv::IMREAD_GRAYSCALE);

    call_py("SLAM_Matcher", "match_images", img1, img2);

    // for(int i = 0; i < 10; i++)
    // {
    //     call_py("SLAM_Matcher", "match_images", img1, img2);
    // }

    // if (Py_FinalizeEx() < 0) {
    //     return 120;
    // }
    return 0;
}