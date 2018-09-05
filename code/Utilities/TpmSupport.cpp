// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"
#include <regex>
#include "DMProcess.h"
#include "TpmSupport.h"
#include "DMException.h"
#include "Logger.h"

using namespace std;

// Example -ast output:
//
// HostName=iot-open-house-demo.azure-devices.net;DeviceId=device0;SharedAccessSignature=SharedAccessSignature sr=iot-open-house-demo.azure-devices.net/devices/device0&sig=dPmolG%2foBeyrrO1pt6dCQr853a6%2fc6xxWYth%2fUptFyg%3d&se=1486606191

// Example -rur output:
//
//<?xml version="1.0" encoding="utf-8"?>
//<ServiceURI>
//  iot-open-house-demo.azure-devices.net/device0
//</ServiceURI>

namespace Microsoft { namespace Windows { namespace Azure { namespace DeviceManagement { namespace Utils {

std::string Tpm::RunLimpet(const std::wstring& params)
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    std::string output;

    // build limpet command and invoke it  
    wchar_t sys32dir[MAX_PATH];
    GetSystemDirectoryW(sys32dir, _countof(sys32dir));

    wchar_t fullCommand[MAX_PATH];
    swprintf_s(fullCommand, _countof(fullCommand), L"%s\\%s %s", sys32dir, L"limpet.exe", params.c_str());

    unsigned long returnCode;

    Process::Launch(fullCommand, returnCode, output);

    return output;
}

std::string Tpm::GetServiceUrl(int logicalId)
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    const std::string response = RunLimpet(to_wstring(logicalId) + L" -rur");

    std::regex rgx(".*<ServiceURI>\\s*(\\S+)\\s*</ServiceURI>.*");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx))
    {
        auto m = match[1];
        return m.str();
    }
    auto responseW = Utils::MultibyteToWide(response.c_str());
    TRACEP(LoggingLevel::Verbose, L"Unexpected response from Limpet:", responseW.c_str());
    throw DMException(-1, "cannot parse Limpet response. Is TPM supported?");
}

std::string Tpm::GetSASToken(int logicalId, unsigned int durationInSeconds)
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    const std::string response = RunLimpet(to_wstring(logicalId) + L" -ast " + to_wstring(durationInSeconds));

    // There is a bug in Limpet that produces the entire connection string and not only the SAS token
    // Work around by extracting the actual connection string
    // The workaround will continue to work (but will be unnecessary) once the bug in Limpet is fixed

    std::regex rgx(".*(SharedAccessSignature sr.*)");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx))
    {
        auto m = match[1];
        return m.str();
    }
    auto responseW = Utils::MultibyteToWide(response.c_str());
    TRACEP(LoggingLevel::Verbose, L"Unexpected response from Limpet:", responseW.c_str());
    throw DMException(-1, "cannot parse Limpet response. Is TPM supported?");
}

void Tpm::ClearTPM()
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    const std::string response = RunLimpet(L"-fct");

    std::regex rgx(".*<TpmClear>\\s*(\\S+)\\s*</TpmClear>.*");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx))
    {
        auto m = match[1].str();
        string result = Utils::TrimString(m, string(" \t\n\r"));
        if (0 == _stricmp(result.c_str(), "ok"))
        {
            TRACE(LoggingLevel::Verbose, L"ClearTPM completed successfully.");
            return;
        }
    }
    auto responseW = Utils::MultibyteToWide(response.c_str());
    TRACEP(LoggingLevel::Verbose, L"Unexpected response from Limpet:", responseW.c_str());
    throw DMException(-1, "cannot parse Limpet response. Is TPM supported?");
}

std::string Tpm::GetEndorsementKey()
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    const std::string response = RunLimpet(L"-erk");
    std::regex rgx(".*<ERKPub>\\s*(\\S+)\\s*</ERKPub>.*");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx))
    {
        auto m = match[1].str();
        string result = Utils::TrimString(m, string(" \t\n\r"));
        TRACE(LoggingLevel::Verbose, L"GetEndorsementKey completed successfully.");
        return result;
    }
    throw DMException(-1, "cannot parse Limpet response. Is TPM supported?");
}

std::string Tpm::GetSRootKey()
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    const std::string response = RunLimpet(L"-srk");
    std::regex rgx(".*<SRKPub>\\s*(\\S+)\\s*</SRKPub>.*");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx))
    {
        auto m = match[1].str();
        string result = Utils::TrimString(m, string(" \t\n\r"));
        TRACE(LoggingLevel::Verbose, L"GetSRootKey completed successfully.");
        return result;
    }
    throw DMException(-1, "cannot parse Limpet response. Is TPM supported?");
}

void Tpm::DestroyServiceUrl(int logicalId)
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    RunLimpet(to_wstring(logicalId) + L" -dur");
}

void Tpm::StoreServiceUrl(int logicalId, const std::string& url)
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    RunLimpet(to_wstring(logicalId) + (std::wstring)L" -sur " + Utils::MultibyteToWide(url.c_str()));
}

void Tpm::ImportSymetricIdentity(int logicalId, const std::string& hostageFile)
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    RunLimpet(to_wstring(logicalId) + L" -isi " + Utils::MultibyteToWide(hostageFile.c_str()));
}

void Tpm::EvictHmacKey(int logicalId)
{
    TRACE(LoggingLevel::Verbose, __FUNCTION__);

    RunLimpet(to_wstring(logicalId) + L" -ehk");
}




}}}}}
