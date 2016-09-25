/*
* Copyright(C) 2016 WIS Team WIFI Detector. ALL rights reserved.
*/
/*
* network.cpp
* Original Author: han    2016-08-05
*
*/
#ifndef UNICODE
#define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define USE_MIB_IFROW_VER    2

#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <objbase.h>
#include <wtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "Network.h"

#include <setupapi.h>
#pragma comment(lib, "setupapi.lib")

#pragma comment(lib, "Iphlpapi.lib")

static inline void STR_2_GUID(char *guiStr, GUID *guid)
{
    printf("%s\n", guiStr);
    sscanf_s(guiStr, "{%8x-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x}", &(guid->Data1),
    (unsigned int *)&(guid->Data2), (unsigned int *)&(guid->Data3),
    (unsigned int *)&(guid->Data4[0]), (unsigned int *)&(guid->Data4[1]),
    (unsigned int *)&(guid->Data4[2]), (unsigned int *)&(guid->Data4[3]),
    (unsigned int *)&(guid->Data4[4]), (unsigned int *)&(guid->Data4[5]),
    (unsigned int *)&(guid->Data4[6]), (unsigned int *)&(guid->Data4[7]));
}

static inline void guid2str(char *buf, size_t len, GUID *guid)
{
    _snprintf_s(buf, len, len, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid->Data1, guid->Data2, guid->Data3,
        guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
        guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

CNetworkIf::CNetworkIf(const char *guid)
{
    this->Init();
    memset(this->_guid, 0, ADAPTERNAME_SIZE);
    strcpy_s(this->_guid, guid);

#if(USE_MIB_IFROW_VER == 2)
    this->_pIfRow = malloc(sizeof(MIB_IF_ROW2));
#else
    this->_pIfRow = malloc(sizeof(MIB_IFROW));
#endif
}

CNetworkIf::~CNetworkIf()
{
    memset(this->_guid, 0, ADAPTERNAME_SIZE);
    this->Init();

    if (this->_pIfRow) {
        free(this->_pIfRow);
        this->_pIfRow = NULL;
    }
}

void CNetworkIf::Init()
{
    memset(this->_desc, 0, ADAPTERDESC_SIZE);
    memset(this->_mac, 0, MAX_ADAPTER_ADDRESS_LENGTH);
    this->_macAddressLen = 0;
    memset(this->_curIpStr, 0, IPSTRING_SIZE);
    memset(this->_curIpMaskStr, 0, IPSTRING_SIZE);
    memset(this->_gatewayIpStr, 0, IPSTRING_SIZE);
    this->_dhcpEnable = false;
}

bool CNetworkIf::IsValid()
{
    return this->_macAddressLen == 0 ? false : true;
}

void CNetworkIf::ParseInfo(PIP_ADAPTER_INFO pAdapter, MIB_IFROW *ifRow)
{
    strcpy_s(this->_desc, pAdapter->Description);
    this->_macAddressLen = pAdapter->AddressLength;
    memcpy_s(this->_mac, MAX_ADAPTER_ADDRESS_LENGTH, pAdapter->Address, MAX_ADAPTER_ADDRESS_LENGTH);
    strcpy_s(this->_curIpStr, pAdapter->IpAddressList.IpAddress.String);
    strcpy_s(this->_curIpMaskStr, pAdapter->IpAddressList.IpMask.String);
    this->_rateMbps = ifRow->dwSpeed / 1000 / 1000;
    strcpy_s(this->_gatewayIpStr, pAdapter->GatewayList.IpAddress.String);
    this->_dhcpEnable = (pAdapter->DhcpEnabled == 0) ? false : true;
}

DWORD CNetworkIf::UpdateIfInfo()
{
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    PIP_ADAPTER_INFO pInfo = NULL;
    ULONG outBufLen = sizeof(IP_ADAPTER_INFO);
    DWORD ret;
    bool found = false;
    MIB_IFROW *ifRow;

    pAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
    if (pAdapterInfo == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    /* get buffer length */
    ret = GetAdaptersInfo(pAdapterInfo, &outBufLen);
    if (ret == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
        if (pAdapterInfo == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        ret = GetAdaptersInfo(pAdapterInfo, &outBufLen);
        if (ret != ERROR_SUCCESS) {
            goto _exit;
        }
    }

    /* find what we need */
    pInfo = pAdapterInfo;
    while (pInfo) {
        if (strcmp(pInfo->AdapterName, this->_guid) == 0) {
            found = true;
            break;
        }

        pInfo = pInfo->Next;
    }

    if (found) {
        ifRow = (MIB_IFROW *)malloc(sizeof(MIB_IFROW));
        if (ifRow == NULL) {
            ret = ERROR_NOT_ENOUGH_MEMORY;
            goto _exit;
        }

        ifRow->dwIndex = pInfo->Index;
        this->_index = pInfo->Index;
        ret = GetIfEntry(ifRow);
        if (ret != ERROR_SUCCESS) {
            goto _exit;
        }

        /* parse info that we need */
        this->Init();
        this->ParseInfo(pInfo, ifRow);

        free(ifRow);
    }
    else {
        ret = ERROR_NOT_FOUND;
    }

_exit:
    free(pAdapterInfo);

    return ret;
}

UCHAR *CNetworkIf::GetMacAddr()
{
    return this->_mac;
}

char *CNetworkIf::GetDesc()
{
    return this->_desc;
}

char *CNetworkIf::GetCurIpStr()
{
    return this->_curIpStr;
}

char *CNetworkIf::GetCurIpMaskStr()
{
    return this->_curIpMaskStr;
}

int CNetworkIf::GetMaxRateMbps()
{
    return this->_rateMbps;
}

char *CNetworkIf::GetGatewayIpStr()
{
    return this->_gatewayIpStr;
}

bool CNetworkIf::IsDhcpEnable()
{
    return this->_dhcpEnable;
}

DWORD CNetworkIf::GetNetState(PNETSTATE pst)
{
    DWORD ret;

    if (pst == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (this->_pIfRow == NULL) {
#if(USE_MIB_IFROW_VER == 2)
        this->_pIfRow = malloc(sizeof(MIB_IF_ROW2));
#else
        this->_pIfRow = malloc(sizeof(MIB_IFROW));
#endif
        if (this->_pIfRow == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

#if(USE_MIB_IFROW_VER == 2)
    MIB_IF_ROW2 *ifrow = (MIB_IF_ROW2 *)this->_pIfRow;
    memset(ifrow, 0, sizeof(MIB_IF_ROW2));
    ifrow->InterfaceIndex = this->_index;
    ret = GetIfEntry2(ifrow);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }
    pst->maxRateMbps = (DWORD)(ifrow->TransmitLinkSpeed / 1000 / 1000);
    pst->downBytes = (DWORD)ifrow->InOctets;
    pst->downBits = pst->downBytes * 8;
    pst->upBytes = (DWORD)ifrow->OutOctets;
    pst->upBits = pst->upBytes * 8;
#else
    MIB_IFROW *ifrow = (MIB_IFROW *)this->_pIfRow;
    ifrow->dwIndex = this->_index;
    ret = GetIfEntry(ifrow);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }
    pst->maxRateMbps = ifrow->dwSpeed / 1000 / 1000;
    pst->downBytes = ifrow->dwInOctets;
    pst->downBits = pst->downBytes * 8;
    pst->upBytes = ifrow->dwOutOctets;
    pst->upBits = pst->upBytes * 8;
#endif
    this->_rateMbps = pst->maxRateMbps;

    return ERROR_SUCCESS;
}

/* just pass through */
DWORD CNetworkIf::GetIPSt(MIB_IPSTATS *pst)
{
    return GetIpStatistics(pst);
}

#include <iostream>
using namespace std;

DWORD CNetworkIf::EnableIf(bool enable)
{
#if 0
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    GUID guid;

    //STR_2_GUID(this->_guid, &guid);

    hDevInfo = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return ERROR_SYSTEM_DEVICE_NOT_FOUND;
    }

    /* enum devices */
    SP_DEVINFO_DATA deviceData = { sizeof(SP_DEVINFO_DATA) };
    char buf[64];
    memset(buf, 0, 64);

    for (int i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceData); i++) {
        guid2str(buf, 64, &deviceData.ClassGuid);
        if (strcmp(buf, this->_guid) == 0) {
        //if (memcmp(&guid, &deviceData.ClassGuid, sizeof(GUID) == 0)) {
            SP_PROPCHANGE_PARAMS params = { sizeof(SP_PROPCHANGE_PARAMS) };
            DWORD size;
            params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            params.Scope = DICS_FLAG_CONFIGSPECIFIC;
            params.StateChange = enable ? DICS_DISABLE : DICS_ENABLE;
            params.HwProfile = 0;
            cout << SetupDiGetClassInstallParams(hDevInfo, &deviceData, (SP_CLASSINSTALL_HEADER *)&params, sizeof(SP_PROPCHANGE_PARAMS), &size) << endl;
            cout << SetupDiChangeState(hDevInfo, &deviceData) << endl;
        }
    }
#endif

    return ERROR_SUCCESS;
}
