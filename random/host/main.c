#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <tee_client_api.h>
#include <random_ta.h>

static PyObject* generate_random(PyObject* self, PyObject* args) {
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op = { 0 };
    TEEC_UUID uuid = TA_RANDOM_UUID;
    uint32_t err_origin;
    uint8_t* random_data = NULL;
    Py_ssize_t random_length;

    // Parse Python arguments
    if (!PyArg_ParseTuple(args, "n", &random_length)) {
        return NULL;
    }

    random_data = (uint8_t*)malloc(random_length);
    if (!random_data) {
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory");
        return NULL;
    }

    // Initialize TEE Context
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        free(random_data);
        PyErr_Format(PyExc_RuntimeError, "TEEC_InitializeContext failed: 0x%x", res);
        return NULL;
    }

    // Open a session to the TA
    res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS) {
        TEEC_FinalizeContext(&ctx);
        free(random_data);
        PyErr_Format(PyExc_RuntimeError, "TEEC_OpenSession failed: 0x%x origin 0x%x", res, err_origin);
        return NULL;
    }

    // Set up the operation parameters
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = random_data;
    op.params[0].tmpref.size = random_length;

    // Invoke the command
    res = TEEC_InvokeCommand(&sess, TA_RANDOM_CMD_GENERATE, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        TEEC_CloseSession(&sess);
        TEEC_FinalizeContext(&ctx);
        free(random_data);
        PyErr_Format(PyExc_RuntimeError, "TEEC_InvokeCommand failed: 0x%x origin 0x%x", res, err_origin);
        return NULL;
    }

    // Close session and clean up
    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);

    PyObject* result = Py_BuildValue("y#", random_data, random_length);
    free(random_data);
    return result;
}

// Module method table
static PyMethodDef RandomMethods[] = {
    {"generate_random", generate_random, METH_VARARGS, "Generate random data of specified length"},
    {NULL, NULL, 0, NULL}
};

// Module definition
static struct PyModuleDef randommodule = {
    PyModuleDef_HEAD_INIT,
    "TEErandom", // Module name
    NULL,        // Module documentation
    -1,          // Size of per-interpreter state or -1
    RandomMethods
};

// Module initialization
PyMODINIT_FUNC PyInit_TEErandom(void) {
    return PyModule_Create(&randommodule);
}
