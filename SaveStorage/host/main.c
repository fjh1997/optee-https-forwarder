#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>
#include <secure_storage_ta.h>

struct test_ctx {
    TEEC_Context ctx;
    TEEC_Session sess;
};

static void prepare_tee_session(struct test_ctx *ctx) {
    TEEC_UUID uuid = TA_SECURE_STORAGE_UUID;
    uint32_t origin;
    TEEC_Result res;

    /* Initialize a context connecting us to the TEE */
    res = TEEC_InitializeContext(NULL, &ctx->ctx);
    if (res != TEEC_SUCCESS) {
        PyErr_Format(PyExc_RuntimeError, "TEEC_InitializeContext failed with code 0x%x", res);
        return;
    }

    /* Open a session with the TA */
    res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS) {
        TEEC_FinalizeContext(&ctx->ctx);
        PyErr_Format(PyExc_RuntimeError, "TEEC_Opensession failed with code 0x%x origin 0x%x",
                     res, origin);
        return;
    }
}

static void terminate_tee_session(struct test_ctx *ctx) {
    TEEC_CloseSession(&ctx->sess);
    TEEC_FinalizeContext(&ctx->ctx);
}

static TEEC_Result read_secure_object(struct test_ctx *ctx, char *id,
                                      char *data, size_t data_len) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;
    size_t id_len = strlen(id);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_MEMREF_TEMP_OUTPUT,
                                     TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = id;
    op.params[0].tmpref.size = id_len;

    op.params[1].tmpref.buffer = data;
    op.params[1].tmpref.size = data_len;

    res = TEEC_InvokeCommand(&ctx->sess,
                             TA_SECURE_STORAGE_CMD_READ_RAW,
                             &op, &origin);
    switch (res) {
        case TEEC_SUCCESS:
        case TEEC_ERROR_SHORT_BUFFER:
        case TEEC_ERROR_ITEM_NOT_FOUND:
            break;
        default:
            printf("Command READ_RAW failed: 0x%x / %u\n", res, origin);
    }

    return res;
}

static TEEC_Result write_secure_object(struct test_ctx *ctx, char *id,
                                       char *data, size_t data_len) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;
    size_t id_len = strlen(id);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = id;
    op.params[0].tmpref.size = id_len;

    op.params[1].tmpref.buffer = data;
    op.params[1].tmpref.size = data_len;

    res = TEEC_InvokeCommand(&ctx->sess,
                             TA_SECURE_STORAGE_CMD_WRITE_RAW,
                             &op, &origin);
    if (res != TEEC_SUCCESS)
        printf("Command WRITE_RAW failed: 0x%x / %u\n", res, origin);

    return res;
}

static TEEC_Result delete_secure_object(struct test_ctx *ctx, char *id) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;
    size_t id_len = strlen(id);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_NONE, TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = id;
    op.params[0].tmpref.size = id_len;

    res = TEEC_InvokeCommand(&ctx->sess,
                             TA_SECURE_STORAGE_CMD_DELETE,
                             &op, &origin);

    switch (res) {
        case TEEC_SUCCESS:
        case TEEC_ERROR_ITEM_NOT_FOUND:
            break;
        default:
            printf("Command DELETE failed: 0x%x / %u\n", res, origin);
    }

    return res;
}

// Python interface functions
// 添加获取长度的函数
static TEEC_Result get_secure_object_size(struct test_ctx *ctx, char *id, size_t *data_len) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;
    size_t id_len = strlen(id);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = id;
    op.params[0].tmpref.size = id_len;

    op.params[1].tmpref.buffer = data_len;
    op.params[1].tmpref.size = sizeof(size_t);

    res = TEEC_InvokeCommand(&ctx->sess, TA_SECURE_STORAGE_CMD_GET_SIZE, &op, &origin);
    if (res != TEEC_SUCCESS) {
        PyErr_Format(PyExc_RuntimeError, "Command GET_SIZE failed: 0x%x / %u", res, origin);
    }

    return res;
}
static PyObject* py_read_secure_object(PyObject* self, PyObject* args) {
    struct test_ctx ctx;
    char *id;
    size_t data_len;
    char *data;
   // printf("memory size1%lu",data_len);
    if (!PyArg_ParseTuple(args, "s", &id)) {
        return NULL;
    }

    prepare_tee_session(&ctx);

    // 首先获取数据长度
    TEEC_Result res = get_secure_object_size(&ctx, id, &data_len);
    if (res != TEEC_SUCCESS) {
        terminate_tee_session(&ctx);
        return NULL;
    }
    //printf("memory size%lu",data_len);
    // 分配内存
    data_len=5280;
    data = PyMem_Malloc(data_len);
    if (data == NULL) {
        PyErr_NoMemory();
        terminate_tee_session(&ctx);
        return NULL;
    }

    // 读取数据
    res = read_secure_object(&ctx, id, data, data_len);
    terminate_tee_session(&ctx);

    if (res != TEEC_SUCCESS) {
        PyMem_Free(data);
        PyErr_Format(PyExc_RuntimeError, "read_secure_object failed: 0x%x", res);
        return NULL;
    }

    PyObject* result = PyBytes_FromStringAndSize(data, data_len);
    PyMem_Free(data);
    return result;
}


static PyObject* py_write_secure_object(PyObject* self, PyObject* args) {
    struct test_ctx ctx;
    char *id, *data;
    Py_ssize_t data_len;

    if (!PyArg_ParseTuple(args, "ss#", &id, &data, &data_len)) {
        return NULL;
    }

    prepare_tee_session(&ctx);
    TEEC_Result res = write_secure_object(&ctx, id, data, data_len);
    terminate_tee_session(&ctx);

    if (res != TEEC_SUCCESS) {
        PyErr_Format(PyExc_RuntimeError, "write_secure_object failed: 0x%x", res);
        return NULL;
    }
    
    Py_RETURN_NONE;
}

static PyObject* py_delete_secure_object(PyObject* self, PyObject* args) {
    struct test_ctx ctx;
    char *id;

    if (!PyArg_ParseTuple(args, "s", &id)) {
        return NULL;
    }

    prepare_tee_session(&ctx);
    TEEC_Result res = delete_secure_object(&ctx, id);
    terminate_tee_session(&ctx);

    if (res != TEEC_SUCCESS && res != TEEC_ERROR_ITEM_NOT_FOUND) {
        PyErr_Format(PyExc_RuntimeError, "delete_secure_object failed: 0x%x", res);
        return NULL;
    }
    
    Py_RETURN_NONE;
}

// Module method table
// Module method table
static PyMethodDef SecureStorageMethods[] = {
    {"read_secure_object", py_read_secure_object, METH_VARARGS, "Read an object from secure storage"},
    {"write_secure_object", py_write_secure_object, METH_VARARGS, "Write an object to secure storage"},
    {"delete_secure_object", py_delete_secure_object, METH_VARARGS, "Delete an object from secure storage"},
    {NULL, NULL, 0, NULL}
};



// Module definition
static struct PyModuleDef securestoragemodule = {
    PyModuleDef_HEAD_INIT,
    "Tsecurestorage", // Module name
    NULL,            // Module documentation
    -1,              // Size of per-interpreter state or -1
    SecureStorageMethods
};

// Module initialization
PyMODINIT_FUNC PyInit_Tsecurestorage(void) {
    return PyModule_Create(&securestoragemodule);
}
