/*
 * Copyright(C) 2016 WIS Team WIFI Detector. ALL rights reserved.
 */
/*
 * wlanMidware.cpp
 * Original Author: han    2016-08-05
 * operations for wlanapi
 */

#include "WlanInterface.h"
#include <stdio.h>

#include <iostream>
using namespace std;

static const char *intfStateStr[] = {
    "wlan_interface_state_not_ready",
    "wlan_interface_state_connected",
    "wlan_interface_state_ad_hoc_network_formed",
    "wlan_interface_state_disconnecting",
    "wlan_interface_state_disconnected",
    "wlan_interface_state_associating",
    "wlan_interface_state_discovering",
    "wlan_interface_state_authenticating"
};

static const char *acmNotifyStr[] = {
    "wlan_notification_acm_start",
    "wlan_notification_acm_autoconf_enabled",
    "wlan_notification_acm_autoconf_disabled",
    "wlan_notification_acm_background_scan_enabled",
    "wlan_notification_acm_background_scan_disabled",
    "wlan_notification_acm_bss_type_change",
    "wlan_notification_acm_power_setting_change",
    "wlan_notification_acm_scan_complete",
    "wlan_notification_acm_scan_fail",
    "wlan_notification_acm_connection_start",
    "wlan_notification_acm_connection_complete",
    "wlan_notification_acm_connection_attempt_fail",
    "wlan_notification_acm_filter_list_change",
    "wlan_notification_acm_interface_arrival",
    "wlan_notification_acm_interface_removal",
    "wlan_notification_acm_profile_change",
    "wlan_notification_acm_profile_name_change",
    "wlan_notification_acm_profiles_exhausted",
    "wlan_notification_acm_network_not_available",
    "wlan_notification_acm_network_available",
    "wlan_notification_acm_disconnecting",
    "wlan_notification_acm_disconnected",
    "wlan_notification_acm_adhoc_network_state_change",
    "wlan_notification_acm_profile_unblocked",
    "wlan_notification_acm_screen_power_change",
    "wlan_notification_acm_profile_blocked",
    "wlan_notification_acm_scan_list_refresh",
    "wlan_notification_acm_end"
};

static const char *msmNotifyStr[] = {
    "wlan_notification_msm_start",
    "wlan_notification_msm_associating",
    "wlan_notification_msm_associated",
    "wlan_notification_msm_authenticating",
    "wlan_notification_msm_connected",
    "wlan_notification_msm_roaming_start",
    "wlan_notification_msm_roaming_end",
    "wlan_notification_msm_radio_state_change",
    "wlan_notification_msm_signal_quality_change",
    "wlan_notification_msm_disassociating",
    "wlan_notification_msm_disconnected",
    "wlan_notification_msm_peer_join",
    "wlan_notification_msm_peer_leave",
    "wlan_notification_msm_adapter_removal",
    "wlan_notification_msm_adapter_operation_mode_change",
    "wlan_notification_msm_end"
};

static inline void guid2str(char *buf, size_t len, GUID *guid)
{
    _snprintf_s(buf, len, len, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid->Data1, guid->Data2, guid->Data3,
        guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
        guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

CWlanInterface::CWlanInterface(HANDLE wlanHandle, PWLAN_INTERFACE_INFO pInfo)
{
    /* wlan handle, operation needs */
    this->_hWlan = wlanHandle;
    this->_notifyFunc = NULL;

    memcpy_s(&this->_guid, sizeof(GUID), &pInfo->InterfaceGuid, sizeof(GUID));
    this->_curState = pInfo->isState;

    memset(this->_desc, 0, MAX_BUFFER_SIZE);
    ::WideCharToMultiByte(CP_ACP, 0, pInfo->strInterfaceDescription, (int)wcslen(pInfo->strInterfaceDescription), this->_desc, MAX_BUFFER_SIZE,
        NULL, NULL);

    memset(this->_guidStr, 0, GUID_STR_BUFFER_SIZE);
    guid2str(this->_guidStr, GUID_STR_BUFFER_SIZE, &this->_guid);

    if (this->IsConnect()) {
        this->UpdateCurBssInfo();
    }
}

CWlanInterface::~CWlanInterface()
{
    this->_hWlan = NULL;
    this->_notifyFunc = NULL;
    memset(&this->_guid, 0, sizeof(GUID));
    memset(this->_desc, 0, MAX_BUFFER_SIZE);
    memset(this->_guidStr, 0, GUID_STR_BUFFER_SIZE);
}

char *CWlanInterface::GetDescription()
{
    return this->_desc;
}

char *CWlanInterface::GetGUIDStr()
{
    return this->_guidStr;
}

bool CWlanInterface::CompareGUID(GUID *guid)
{
    return memcmp(&this->_guid, guid, sizeof(GUID)) == 0 ? true : false;
}

bool CWlanInterface::CompareGUID(char *guidStr)
{
    return strcmp(this->_guidStr, guidStr) == 0 ? true : false;
}

void CWlanInterface::OnNotification(PWLAN_NOTIFICATION_DATA data)
{
    WLANINTERFACE_NOTIFICATION notify;
    bool ret = false;

    notify.code = data->NotificationCode;
    notify.source = data->NotificationSource;

    switch (data->NotificationSource) {
    /* not support following notifications
    case WLAN_NOTIFICATION_SOURCE_NONE:
    case WLAN_NOTIFICATION_SOURCE_ONEX:
    case WLAN_NOTIFICATION_SOURCE_SECURITY:
    case WLAN_NOTIFICATION_SOURCE_IHV:
    case WLAN_NOTIFICATION_SOURCE_HNWK:
        break;
        not support end */
    case WLAN_NOTIFICATION_SOURCE_ACM:
        ret = this->OnACMNotification(data, &notify);
        break;
    case WLAN_NOTIFICATION_SOURCE_MSM:
        ret = this->OnMSMNotification(data, &notify);
        break;
    default:
        return;
    }

    if (ret && this->_notifyFunc) {
        this->_notifyFunc(&notify, this);
    }
}

bool CWlanInterface::OnACMNotification(PWLAN_NOTIFICATION_DATA data, PWLANINTERFACE_NOTIFICATION notify)
{
    bool ret = true;

    switch (data->NotificationCode)
    {
    case wlan_notification_acm_connection_start:
    case wlan_notification_acm_connection_complete:
    case wlan_notification_acm_connection_attempt_fail:
    case wlan_notification_acm_disconnecting:
    case wlan_notification_acm_disconnected:
        {
            if (data->dwDataSize < sizeof(WLAN_CONNECTION_NOTIFICATION_DATA)) {
                ret = false;
                break;
            }

            PWLAN_CONNECTION_NOTIFICATION_DATA pData = (PWLAN_CONNECTION_NOTIFICATION_DATA)data->pData;
            notify->reasonCode = pData->wlanReasonCode;

            break;
        }
    case wlan_notification_acm_scan_fail:
        {
            if (data->dwDataSize < sizeof(WLAN_REASON_CODE)) {
                ret = false;
                break;
            }

            notify->reasonCode = *(WLAN_REASON_CODE*)data->pData;

            break;
        }
    case wlan_notification_acm_scan_complete:
        /* do nothing */
        break;
    default:
        {
            ret = false;
            break;
        }
    }

    if (data->NotificationCode == wlan_notification_acm_connection_complete) {
        this->UpdateCurBssInfo();
    }

    return ret;
}

bool CWlanInterface::OnMSMNotification(PWLAN_NOTIFICATION_DATA data, PWLANINTERFACE_NOTIFICATION notify)
{
    switch (data->NotificationCode)
    {
    case wlan_notification_msm_associating:
    case wlan_notification_msm_associated:
    case wlan_notification_msm_authenticating:
    case wlan_notification_msm_connected:
    case wlan_notification_msm_roaming_start:
    case wlan_notification_msm_roaming_end:
    case wlan_notification_msm_disassociating:
    case wlan_notification_msm_disconnected:
    case wlan_notification_msm_peer_join:
    case wlan_notification_msm_peer_leave:
    case wlan_notification_msm_adapter_removal:
        {
            if (data->dwDataSize < sizeof(WLAN_MSM_NOTIFICATION_DATA)) {
                return false;
            }

            PWLAN_MSM_NOTIFICATION_DATA pData = (PWLAN_MSM_NOTIFICATION_DATA)data->pData;
            notify->reasonCode = pData->wlanReasonCode;

            if (data->NotificationCode == wlan_notification_msm_connected) {
                this->UpdateCurBssInfo();
            }

            return true;
        }
    default:
        return false;
    }
}

void CWlanInterface::SetNotifyCallback(WLANINTERFACENOTIFY func)
{
    this->_notifyFunc = func;
}

const char *CWlanInterface::FormatNotificationStr(int source, int code)
{
    switch (source) {
    case WLAN_NOTIFICATION_SOURCE_ACM:
        {
            if ((code >= wlan_notification_acm_start && code <= wlan_notification_acm_end)) {
                return acmNotifyStr[code];
            }
        }
        break;
    case WLAN_NOTIFICATION_SOURCE_MSM:
        {
            if ((code >= wlan_notification_msm_start && code <= wlan_notification_msm_end)) {
                return msmNotifyStr[code];
            }
        }
        break;
    default:
        break;
    }

    return NULL;
}

const char *CWlanInterface::FormatIntfStateStr(int state)
{
    if ((state >= wlan_interface_state_not_ready) && (state <= wlan_interface_state_authenticating)) {
        return intfStateStr[state];
    }

    return NULL;
}

const char *CWlanInterface::FormatIntfStateStr()
{
    return intfStateStr[this->_curState];
}

PWLAN_CONNECTION_PARAMETERS CWlanInterface::AllocConnectParams(char *ssid, DOT11_MAC_ADDRESS *pbssid, int bssidNum)
{
    PDOT11_SSID pssid;
    int i;
    PWLAN_CONNECTION_PARAMETERS params;

    params = (PWLAN_CONNECTION_PARAMETERS)malloc(sizeof(WLAN_CONNECTION_PARAMETERS));
    if (params == NULL) {
        return NULL;
    }

    memset(params, 0, sizeof(WLAN_CONNECTION_PARAMETERS));
    params->dot11BssType = dot11_BSS_type_infrastructure;
    /* not support security */
    params->strProfile = NULL;
    params->dwFlags = 0;

    if (ssid == NULL) {
        params->pDot11Ssid = NULL;
        params->wlanConnectionMode = wlan_connection_mode_auto;
    }
    else {
        pssid = (PDOT11_SSID)malloc(sizeof(DOT11_SSID));
        if (pssid == NULL) {
            goto _fail;
        }

        params->wlanConnectionMode = wlan_connection_mode_discovery_unsecure;
        memset(pssid, 0, sizeof(DOT11_SSID));
        pssid->uSSIDLength = (ULONG)strnlen_s(ssid, DOT11_SSID_MAX_LENGTH);
        strcpy_s((char *)pssid->ucSSID, DOT11_SSID_MAX_LENGTH, ssid);
        params->pDot11Ssid = pssid;
    }

    if ((pbssid == NULL) || (bssidNum == 0)) {
        params->pDesiredBssidList = NULL;
    }
    else {
        PDOT11_BSSID_LIST bssidList = (PDOT11_BSSID_LIST)malloc(sizeof(DOT11_BSSID_LIST)+sizeof(DOT11_MAC_ADDRESS)* bssidNum);
        if (bssidList == NULL) {
            goto _fail;
        }

        /* not sure what does it mean */
        bssidList->Header.Revision = DOT11_BSSID_LIST_REVISION_1;
        bssidList->Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
        bssidList->Header.Size = sizeof(DOT11_BSSID_LIST);
        bssidList->uNumOfEntries = bssidNum;
        bssidList->uTotalNumOfEntries = bssidNum;

        for (i = 0; i < bssidNum; i++) {
            memcpy_s(&bssidList->BSSIDs[i], sizeof(DOT11_MAC_ADDRESS), &pbssid[i], sizeof(DOT11_MAC_ADDRESS));
        }

        params->pDesiredBssidList = bssidList;
    }

    return params;

_fail:
    this->ReleaseParams(params);

    return NULL;
}

void CWlanInterface::ReleaseParams(PWLAN_CONNECTION_PARAMETERS params)
{
    if (params == NULL) {
        return;
    }

    if (params->pDot11Ssid) {
        free(params->pDot11Ssid);
        params->pDot11Ssid = NULL;
    }

    if (params->pDesiredBssidList) {
        free(params->pDesiredBssidList);
        params->pDesiredBssidList = NULL;
    }

    free(params);
}

DWORD CWlanInterface::Connect(PWLAN_CONNECTION_PARAMETERS params)
{
    if (params == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    else {
        return WlanConnect(this->_hWlan, &this->_guid, params, NULL);
    }
}

DWORD CWlanInterface::Connect(char *ssid)
{
    DWORD ret;

    PWLAN_CONNECTION_PARAMETERS params = this->AllocConnectParams(ssid, NULL, 0);

    ret = this->Connect(params);

    this->ReleaseParams(params);

    return ret;
}

DWORD CWlanInterface::Connect(char *ssid, DOT11_MAC_ADDRESS bssidArray[], int bssidNum)
{
    DWORD ret;

    PWLAN_CONNECTION_PARAMETERS params = this->AllocConnectParams(ssid, bssidArray, bssidNum);

    ret = this->Connect(params);

    this->ReleaseParams(params);

    return ret;
}

DWORD CWlanInterface::GetWlanIntfInt(WLAN_INTF_OPCODE op, int *val)
{
    DWORD res;
    DWORD size = sizeof(int);
    int *p;

    if (val == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    res = WlanQueryInterface(this->_hWlan, &this->_guid, op, NULL, &size, (PVOID *)&p, NULL);
    if (res == ERROR_SUCCESS) {
        memcpy_s(val, sizeof(int), p, sizeof(int));
        WlanFreeMemory(p);
    }

    return res;
}

DWORD CWlanInterface::SetWlanIntfInt(WLAN_INTF_OPCODE op, int val)
{
    return WlanSetInterface(this->_hWlan, &this->_guid, op, sizeof(int), &val, NULL);
}

DWORD CWlanInterface::Scan(PSCAN_PARAM params)
{
    if (params == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    else {
        return WlanScan(this->_hWlan, &this->_guid, params->ssid, params->pData, NULL);
    }
}

PSCAN_PARAM CWlanInterface::AllocScanParams(char *ssid, char *rawData, DWORD dataByteLen)
{
    PSCAN_PARAM params = (PSCAN_PARAM)malloc(sizeof(SCAN_PARAM));

    if (params == NULL) {
        return NULL;
    }

    if (ssid) {
        params->ssid = (PDOT11_SSID)malloc(sizeof(DOT11_SSID));
        if (params->ssid == NULL) {
            goto _fail;
        }

        memset(params->ssid, 0, sizeof(DOT11_SSID));
        params->ssid->uSSIDLength = (ULONG)strnlen_s(ssid, DOT11_SSID_MAX_LENGTH);
        strcpy_s((char *)params->ssid->ucSSID, DOT11_SSID_MAX_LENGTH, ssid);
    }

    if (rawData && !dataByteLen) {
        params->pData = (PWLAN_RAW_DATA)malloc(sizeof(WLAN_RAW_DATA) + sizeof(char) * dataByteLen);
        if (params->pData == NULL) {
            goto _fail;
        }

        memset(params->pData, 0, sizeof(WLAN_RAW_DATA) + sizeof(char) * dataByteLen);
        params->pData->dwDataSize = dataByteLen;
        memcpy_s(params->pData, dataByteLen, rawData, dataByteLen);
    }

    return params;

_fail:
    this->ReleaseScanParams(params);

    return NULL;
}

void CWlanInterface::ReleaseScanParams(PSCAN_PARAM params)
{
    if (params == NULL) {
        return;
    }

    if (params->ssid) {
        free(params->ssid);
        params->ssid = NULL;
    }

    if (params->pData) {
        free(params->pData);
        params->pData = NULL;
    }

    free(params);
}

DWORD CWlanInterface::Scan()
{
    PSCAN_PARAM param = this->AllocScanParams(NULL, NULL, 0);
    DWORD ret = this->Scan(param);
    this->ReleaseScanParams(param);

    return ret;
}

DWORD CWlanInterface::Scan(char *ssid)
{
    PSCAN_PARAM param = this->AllocScanParams(ssid, NULL, 0);
    DWORD ret = this->Scan(param);
    this->ReleaseScanParams(param);

    return ret;
}

DWORD CWlanInterface::Scan(char *ssid, char *rawdata, DWORD dataByteLen)
{
    PSCAN_PARAM param = this->AllocScanParams(ssid, rawdata, dataByteLen);
    DWORD ret = this->Scan(param);
    this->ReleaseScanParams(param);

    return ret;
}

DWORD CWlanInterface::AutoConf(bool set, int *pbuf)
{
    return set ? SetWlanIntfInt(wlan_intf_opcode_autoconf_enabled, *pbuf) : GetWlanIntfInt(wlan_intf_opcode_autoconf_enabled, pbuf);
}

DWORD CWlanInterface::BssType(bool set, int *pbuf)
{
    return set ? SetWlanIntfInt(wlan_intf_opcode_bss_type, *pbuf) : GetWlanIntfInt(wlan_intf_opcode_bss_type, pbuf);
}

DWORD CWlanInterface::IntfState(int *pbuf)
{
    return GetWlanIntfInt(wlan_intf_opcode_interface_state, pbuf);
}

DWORD CWlanInterface::Channel(int *pbuf)
{
    return GetWlanIntfInt(wlan_intf_opcode_channel_number, pbuf);
}

DWORD CWlanInterface::Rssi(int *pbuf)
{
    return GetWlanIntfInt(wlan_intf_opcode_rssi, pbuf);
}

DWORD CWlanInterface::MediaStreamMode(bool set, int *pbuf)
{
    return set ? SetWlanIntfInt(wlan_intf_opcode_media_streaming_mode, *pbuf) : GetWlanIntfInt(wlan_intf_opcode_media_streaming_mode, pbuf);
}

DWORD CWlanInterface::BackgroudScan(bool set, int *pbuf)
{
    return set ? SetWlanIntfInt(wlan_intf_opcode_background_scan_enabled, *pbuf) : GetWlanIntfInt(wlan_intf_opcode_background_scan_enabled, pbuf);
}

DWORD CWlanInterface::CurOpMode(int *pbuf)
{
    return GetWlanIntfInt(wlan_intf_opcode_current_operation_mode, pbuf);
}

static int freqToChannel(int freq)
{
    freq = freq / 1000;
    if (freq == 2484)
        return 14;
    if (freq < 2484)
        return (freq - 2407) / 5;
    if (freq < 5000) {
        if ((freq) > 4940 && (freq) < 4990) {
            return ((freq * 10) +
                (((freq % 5) == 2) ? 5 : 0) - 49400) / 5;
        }
        else if (freq > 4900) {
            return (freq - 4000) / 5;
        }
        else {
            return 15 + ((freq - 2512) / 20);
        }
    }

    return (freq - 5000) / 5;
}


DWORD CWlanInterface::GetBssList(char *ssid, PBSS_INFO pBss, int *num)
{
    DWORD res;
    PDOT11_SSID pssid = NULL;
    PWLAN_BSS_LIST pBssList;
    int cnt;

    if ((pBss == NULL) || (num == NULL) || (*num == 0)) {
        return ERROR_INVALID_PARAMETER;
    }

    if (ssid) {
        pssid = (PDOT11_SSID)malloc(sizeof(DOT11_SSID));
        if (pssid == NULL) {
            return ERROR_OUTOFMEMORY;
        }

        memset(pssid, 0, sizeof(DOT11_SSID));
        pssid->uSSIDLength = (ULONG)strnlen_s(ssid, DOT11_SSID_MAX_LENGTH);
        strcpy_s((char *)pssid->ucSSID, DOT11_SSID_MAX_LENGTH, ssid);
    }

    res = WlanGetNetworkBssList(this->_hWlan, &this->_guid, pssid, dot11_BSS_type_any, false, NULL, &pBssList);
    if (res != ERROR_SUCCESS) {
        goto _fail;
    }

    if (*num < (int)pBssList->dwNumberOfItems) {
        cnt = *num;
    } else {
        cnt = (int)pBssList->dwNumberOfItems;
    }

    *num = cnt;

    for (int i = 0; i < cnt; i++) {
        memset(&pBss[i], 0, sizeof(BSS_INFO));
        pBss[i].ssid_len = pBssList->wlanBssEntries[i].dot11Ssid.uSSIDLength;
        memcpy_s(pBss[i].ssid, DOT11_SSID_MAX_LENGTH, pBssList->wlanBssEntries[i].dot11Ssid.ucSSID, pBss[i].ssid_len);
        memcpy_s(pBss[i].bssid, sizeof(DOT11_MAC_ADDRESS), pBssList->wlanBssEntries[i].dot11Bssid, sizeof(DOT11_MAC_ADDRESS));
        pBss[i].bsstype = pBssList->wlanBssEntries[i].dot11BssType;
        pBss[i].phytype = pBssList->wlanBssEntries[i].dot11BssPhyType;
        pBss[i].lRssi = pBssList->wlanBssEntries[i].lRssi;
        pBss[i].uLinkQuality = pBssList->wlanBssEntries[i].uLinkQuality;
        pBss[i].bcnPeriod = pBssList->wlanBssEntries[i].usBeaconPeriod;
        pBss[i].ulChCenterFrequency = freqToChannel(pBssList->wlanBssEntries[i].ulChCenterFrequency);
    }

    WlanFreeMemory(pBssList);

_fail:
    if (pssid) {
        free(pssid);
    }

    return res;
}

bool CWlanInterface::IsConnect()
{
    DWORD res;
    int val;

    res = this->IntfState(&val);

    if (res != ERROR_SUCCESS) {
        return false;
    }

    return (val == wlan_interface_state_connected) ? true : false;
}

void CWlanInterface::UpdateCurBssInfo()
{
    DWORD res;
    DWORD size;
    PWLAN_CONNECTION_ATTRIBUTES pattr;
    int rssi;
    int channel;

    memset(&this->_curBSS, 0, sizeof(BSS_INFO));

    res = WlanQueryInterface(this->_hWlan, &this->_guid, wlan_intf_opcode_current_connection, NULL, &size, (PVOID*)&pattr, NULL);
    if (res != ERROR_SUCCESS) {
        return;
    }

    if (pattr->isState != wlan_interface_state_connected) {
        goto _exit;
    }

    res = this->Rssi(&rssi);
    if (res != ERROR_SUCCESS) {
        goto _exit;
    }

    res = this->Channel(&channel);
    if (res != ERROR_SUCCESS) {
        goto _exit;
    }

    memset(&this->_curBSS, 0, sizeof(BSS_INFO));
    this->_curBSS.ssid_len = pattr->wlanAssociationAttributes.dot11Ssid.uSSIDLength;
    memcpy_s(this->_curBSS.ssid, DOT11_SSID_MAX_LENGTH, pattr->wlanAssociationAttributes.dot11Ssid.ucSSID, this->_curBSS.ssid_len);
    memcpy_s(this->_curBSS.bssid, sizeof(DOT11_MAC_ADDRESS), pattr->wlanAssociationAttributes.dot11Bssid, sizeof(DOT11_MAC_ADDRESS));
    this->_curBSS.bsstype = pattr->wlanAssociationAttributes.dot11BssType;
    this->_curBSS.phytype = pattr->wlanAssociationAttributes.dot11PhyType;
    this->_curBSS.uLinkQuality = pattr->wlanAssociationAttributes.wlanSignalQuality;
    this->_curBSS.lRssi = rssi;
    this->_curBSS.ulChCenterFrequency = channel;
    this->_curBSS.ulRxRate = pattr->wlanAssociationAttributes.ulRxRate;
    this->_curBSS.ulTxRate = pattr->wlanAssociationAttributes.ulTxRate;

_exit:
    WlanFreeMemory(pattr);
}

PBSS_INFO CWlanInterface::GetCurBssInfo()
{
    return &this->_curBSS;
}
