#include <inttypes.h>
#include <secure_storage_ta.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

static TEE_Result delete_object(uint32_t param_types, TEE_Param params[4])
{
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                        TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE);
    TEE_ObjectHandle object;
    TEE_Result res;
    char *obj_id;
    size_t obj_id_sz;

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    obj_id_sz = params[0].memref.size;
    obj_id = TEE_Malloc(obj_id_sz, 0);
    if (!obj_id)
        return TEE_ERROR_OUT_OF_MEMORY;

    TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
                                   obj_id, obj_id_sz,
                                   TEE_DATA_FLAG_ACCESS_READ |
                                   TEE_DATA_FLAG_ACCESS_WRITE_META,
                                   &object);
    if (res != TEE_SUCCESS) {
        TEE_Free(obj_id);
        return res;
    }

    TEE_CloseAndDeletePersistentObject1(object);
    TEE_Free(obj_id);

    return res;
}

static TEE_Result create_raw_object(uint32_t param_types, TEE_Param params[4])
{
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                        TEE_PARAM_TYPE_MEMREF_INPUT,
                        TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE);
    TEE_ObjectHandle object;
    TEE_Result res;
    char *obj_id;
    size_t obj_id_sz;
    char *data;
    size_t data_sz;
    uint32_t obj_data_flag;

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    obj_id_sz = params[0].memref.size;
    obj_id = TEE_Malloc(obj_id_sz, 0);
    if (!obj_id)
        return TEE_ERROR_OUT_OF_MEMORY;

    TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

    data_sz = params[1].memref.size;
    data = TEE_Malloc(data_sz, 0);
    if (!data) {
        TEE_Free(obj_id);
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    TEE_MemMove(data, params[1].memref.buffer, data_sz);

    obj_data_flag = TEE_DATA_FLAG_ACCESS_READ |
                    TEE_DATA_FLAG_ACCESS_WRITE |
                    TEE_DATA_FLAG_ACCESS_WRITE_META |
                    TEE_DATA_FLAG_OVERWRITE;

    res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
                                    obj_id, obj_id_sz,
                                    obj_data_flag,
                                    TEE_HANDLE_NULL,
                                    NULL, 0,
                                    &object);
    if (res != TEE_SUCCESS) {
        TEE_Free(obj_id);
        TEE_Free(data);
        return res;
    }

    res = TEE_WriteObjectData(object, data, data_sz);
    if (res != TEE_SUCCESS) {
        TEE_CloseAndDeletePersistentObject1(object);
    } else {
        TEE_CloseObject(object);
    }
    TEE_Free(obj_id);
    TEE_Free(data);
    return res;
}

static TEE_Result read_raw_object(uint32_t param_types, TEE_Param params[4])
{
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                        TEE_PARAM_TYPE_MEMREF_OUTPUT,
                        TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE);
    TEE_ObjectHandle object;
    TEE_ObjectInfo object_info;
    TEE_Result res;
    uint32_t read_bytes;
    char *obj_id;
    size_t obj_id_sz;
    char *data;
    size_t data_sz;

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    obj_id_sz = params[0].memref.size;
    obj_id = TEE_Malloc(obj_id_sz, 0);
    if (!obj_id)
        return TEE_ERROR_OUT_OF_MEMORY;

    TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

    data_sz = params[1].memref.size;
    data = TEE_Malloc(data_sz, 0);
    if (!data) {
        TEE_Free(obj_id);
        return TEE_ERROR_OUT_OF_MEMORY;
    }

    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
                                   obj_id, obj_id_sz,
                                   TEE_DATA_FLAG_ACCESS_READ |
                                   TEE_DATA_FLAG_SHARE_READ,
                                   &object);
    if (res != TEE_SUCCESS) {
        TEE_Free(obj_id);
        TEE_Free(data);
        return res;
    }

    res = TEE_GetObjectInfo1(object, &object_info);
    if (res != TEE_SUCCESS) {
        goto exit;
    }

    if (object_info.dataSize > data_sz) {
        params[1].memref.size = object_info.dataSize;
        res = TEE_ERROR_SHORT_BUFFER;
        goto exit;
    }

    res = TEE_ReadObjectData(object, data, object_info.dataSize,
                             &read_bytes);
    if (res == TEE_SUCCESS)
        TEE_MemMove(params[1].memref.buffer, data, read_bytes);
    if (res != TEE_SUCCESS || read_bytes != object_info.dataSize) {
        goto exit;
    }

    params[1].memref.size = read_bytes;
exit:
    TEE_CloseObject(object);
    TEE_Free(obj_id);
    TEE_Free(data);
    return res;
}

static TEE_Result get_object_size(uint32_t param_types, TEE_Param params[4])
{
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                        TEE_PARAM_TYPE_MEMREF_OUTPUT,
                        TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE);
    TEE_ObjectHandle object;
    TEE_ObjectInfo object_info;
    TEE_Result res;
    char *obj_id;
    size_t obj_id_sz;

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    obj_id_sz = params[0].memref.size;
    obj_id = TEE_Malloc(obj_id_sz, 0);
    if (!obj_id)
        return TEE_ERROR_OUT_OF_MEMORY;

    TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
                                   obj_id, obj_id_sz,
                                   TEE_DATA_FLAG_ACCESS_READ |
                                   TEE_DATA_FLAG_SHARE_READ,
                                   &object);
    if (res != TEE_SUCCESS) {
        TEE_Free(obj_id);
        return res;
    }

    res = TEE_GetObjectInfo1(object, &object_info);
    if (res != TEE_SUCCESS) {
        TEE_Free(obj_id);
        TEE_CloseObject(object);
        return res;
    }

    params[1].memref.size = sizeof(object_info.dataSize);
    TEE_MemMove(params[1].memref.buffer, &object_info.dataSize, sizeof(object_info.dataSize));

    TEE_Free(obj_id);
    TEE_CloseObject(object);
    return res;
}

TEE_Result TA_CreateEntryPoint(void)
{
    return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
    /* Nothing to do */
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t __unused param_types,
                                  TEE_Param __unused params[4],
                                  void __unused **session)
{
    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __unused *session)
{
    /* Nothing to do */
}

TEE_Result TA_InvokeCommandEntryPoint(void __unused *session,
                                     uint32_t command,
                                     uint32_t param_types,
                                     TEE_Param params[4])
{
    switch (command) {
    case TA_SECURE_STORAGE_CMD_WRITE_RAW:
        return create_raw_object(param_types, params);
    case TA_SECURE_STORAGE_CMD_READ_RAW:
        return read_raw_object(param_types, params);
    case TA_SECURE_STORAGE_CMD_DELETE:
        return delete_object(param_types, params);
    case TA_SECURE_STORAGE_CMD_GET_SIZE:
        return get_object_size(param_types, params);
    default:
        EMSG("Command ID 0x%x is not supported", command);
        return TEE_ERROR_NOT_SUPPORTED;
    }
}
