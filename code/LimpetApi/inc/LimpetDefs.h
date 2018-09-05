// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#define LIMPET_TPM_OBJECT_NAME             (sizeof(UINT16) + SHA256_DIGEST_SIZE)
#define LIMPET_DEVICE_ID_STRING_LENGTH     (70) 
#define LIMPET_STRING_SIZE                 (512)
#define MAX_CONNECTION_STRING_LEN          (1024)

#define LIMPET_TPM20_SRK_HANDLE            (0x81000001)
#define LIMPET_TPM20_ERK_HANDLE            (0x81010001)
#define LIMPET_TPM20_PERSISTED_KEY_INDEX   (0x81000100)

#define LIMPET_TPM20_HT_NV_INDEX           (0x01)
#define LIMPET_TPM20_OWNER_NV_SPACE        ((LIMPET_TPM20_HT_NV_INDEX << 24) | (0x01 << 22))
#define LIMPET_TPM20_PERSISTED_URI_INDEX   (LIMPET_TPM20_OWNER_NV_SPACE + 0x100)
#define LIMPET_TPM20_MAX_LOGICAL_DEVICE    (10)
#define LIMPET_HARDWARE_DEVICE_ID_SIZE     (0x22)

#ifndef SHA1_DIGEST_SIZE
#define SHA1_DIGEST_SIZE                    20
#endif
#ifndef SHA256_DIGEST_SIZE
#define SHA256_DIGEST_SIZE                  32
#endif
#ifndef MAX_AES_KEY_BITS
#define MAX_AES_KEY_BITS                    128
#endif
#ifndef MAX_AES_KEY_BYTES
#define MAX_AES_KEY_BYTES           ((MAX_AES_KEY_BITS + 7) / 8)
#endif