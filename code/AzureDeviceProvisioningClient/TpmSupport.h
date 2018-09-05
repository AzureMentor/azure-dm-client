// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "stdafx.h"

namespace Microsoft { namespace Windows { namespace IoT { namespace DeviceManagement { namespace Provisioning {
    typedef std::vector<BYTE> keybuffer_t;

class TpmSupport
{
private:
    static std::wstring Azure_EncodeBase32(std::vector<unsigned char> &pData);

public:

    static HRESULT ImportKeyToTpm(UINT32 logicalDeviceNumber, unsigned char* pbKeyBlob, size_t cbKeyBlob);

    static HRESULT GetEndorsementKeyPub(std::vector<unsigned char>&);

    static HRESULT GetStorageRootKeyPub(std::vector<unsigned char>&);

    static HRESULT InitializeTpm();

    static HRESULT DeinitializeTpm();

    static HRESULT IsDeviceProvisioned(UINT32 logicalDeviceNumber, _Out_ bool *provisioned);

    static HRESULT EmptyTpmSlot(UINT32 logicalDeviceNumber);

    static HRESULT LimpetGenerateDpsSASToken(
        const std::wstring &authenticationKey, // hostage blob sent by DPS service
        std::wstring &tokenScope,
        __int64 expiration,
        _Out_ std::wstring& szConnectionString
    );

    static HRESULT StoreIothubUri(
        __in UINT32 tpmSlotNumber,
        __in_ecount_z(LIMPET_STRING_SIZE) WCHAR* szURI,
        __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcName);

    static HRESULT GetRegistrationId(std::wstring &registrationId);

    static HRESULT GetAzureConnectionString(_In_ UINT32 logicalDeviceNumber, _In_ DWORD expiryDurationInSeconds, _In_opt_ PCWSTR moduleId, __out_ecount_z(*pcchConnectionString) PWSTR connectionString, _Inout_ size_t *pcchConnectionString);
};

}}}}};