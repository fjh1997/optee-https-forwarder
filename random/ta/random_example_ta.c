#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <random_ta.h>

TEE_Result TA_CreateEntryPoint(void)
{
    return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                    TEE_Param __maybe_unused params[4],
                                    void __maybe_unused **sess_ctx)
{
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);
    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    (void)&params;
    (void)&sess_ctx;

    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
    (void)&sess_ctx;
}

static TEE_Result random_number_generate(uint32_t param_types,
                                         TEE_Param params[4])
{
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
                        TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    IMSG("Generating random data over %u bytes.", params[0].memref.size);

    /* Generate the random data directly into the output buffer */
    TEE_GenerateRandom(params[0].memref.buffer, params[0].memref.size);

    return TEE_SUCCESS;
}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
                                      uint32_t cmd_id,
                                      uint32_t param_types, TEE_Param params[4])
{
    (void)&sess_ctx;

    switch (cmd_id) {
    case TA_RANDOM_CMD_GENERATE:
        return random_number_generate(param_types, params);
    default:
        return TEE_ERROR_BAD_PARAMETERS;
    }
}
