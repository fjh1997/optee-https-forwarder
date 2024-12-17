#ifndef __SECURE_STORAGE_H__
#define __SECURE_STORAGE_H__

/* UUID of the trusted application */
#define TA_SECURE_STORAGE_UUID \
    { 0xf4e750bb, 0x1437, 0x4fbf, \
        { 0x87, 0x85, 0x8d, 0x35, 0x80, 0xc3, 0x49, 0x94 } }

/*
 * TA_SECURE_STORAGE_CMD_READ_RAW - Read raw data from a secure storage file
 * param[0] (memref) ID used to identify the persistent object
 * param[1] (memref) Raw data dumped from the persistent object
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_CMD_READ_RAW        0

/*
 * TA_SECURE_STORAGE_CMD_WRITE_RAW - Write raw data to a secure storage file
 * param[0] (memref) ID used to identify the persistent object
 * param[1] (memref) Raw data to be written in the persistent object
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_CMD_WRITE_RAW       1

/*
 * TA_SECURE_STORAGE_CMD_DELETE - Delete a persistent object
 * param[0] (memref) ID used to identify the persistent object
 * param[1] unused
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_CMD_DELETE          2

/*
 * TA_SECURE_STORAGE_CMD_GET_SIZE - Get the size of a persistent object
 * param[0] (memref) ID used to identify the persistent object
 * param[1] (memref) Size of the persistent object
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_CMD_GET_SIZE        3

#endif /* __SECURE_STORAGE_H__ */
