#pragma once

#include <windows.h>
#include <wlanapi.h>
#include "wlan_def.h"

#define MAX_BUFFER_SIZE            (1024)
#define GUID_STR_BUFFER_SIZE    (64)

typedef struct {
    int source;
    int code;
    WLAN_REASON_CODE reasonCode;
} WLANINTERFACE_NOTIFICATION, *PWLANINTERFACE_NOTIFICATION;

typedef struct {
    PDOT11_SSID ssid;
    PWLAN_RAW_DATA pData;
} SCAN_PARAM, *PSCAN_PARAM;

/* bss info */
#define SSID_ARRAY_SIZE    (DOT11_SSID_MAX_LENGTH + 1)    /* can be used as string */
typedef struct {
    ULONG ssid_len;
    char ssid[SSID_ARRAY_SIZE];
    DOT11_MAC_ADDRESS    bssid;
    DOT11_BSS_TYPE        bsstype;        /* 1:infrastructure; 2:independent */
    DOT11_PHY_TYPE        phytype;        /* 1:fhss;2:dsss;3:irbaseband;4:ofdm,5:hrdss;6:eap;7:ht;8:vht */
    LONG                lRssi;
    ULONG                uLinkQuality;    /* 0~100 */
    USHORT                bcnPeriod;
    ULONG                ulChCenterFrequency;
    ULONG                ulRxRate;
    ULONG                ulTxRate;
} BSS_INFO, *PBSS_INFO;

typedef void(*WLANINTERFACENOTIFY)(WLANINTERFACE_NOTIFICATION *data, CWlanInterface *intf);

class CWlanInterface {
public:
    CWlanInterface(HANDLE wlanHandle, PWLAN_INTERFACE_INFO pInfo);
    ~CWlanInterface();

    /* Todo: add here */
private:
    GUID _guid;
    HANDLE _hWlan;
    WLANINTERFACENOTIFY _notifyFunc;
    BSS_INFO _curBSS;
    /*
    typedef enum _WLAN_INTERFACE_STATE {
    wlan_interface_state_not_ready              = 0,
    wlan_interface_state_connected              = 1,
    wlan_interface_state_ad_hoc_network_formed  = 2,
    wlan_interface_state_disconnecting          = 3,
    wlan_interface_state_disconnected           = 4,
    wlan_interface_state_associating            = 5,
    wlan_interface_state_discovering            = 6,
    wlan_interface_state_authenticating         = 7
    } WLAN_INTERFACE_STATE, *PWLAN_INTERFACE_STATE;
    */
    WLAN_INTERFACE_STATE _curState;
    char _desc[MAX_BUFFER_SIZE];
    char _guidStr[GUID_STR_BUFFER_SIZE];

    bool OnACMNotification(PWLAN_NOTIFICATION_DATA data, PWLANINTERFACE_NOTIFICATION notify);
    bool OnMSMNotification(PWLAN_NOTIFICATION_DATA data, PWLANINTERFACE_NOTIFICATION notify);
    DWORD Connect(PWLAN_CONNECTION_PARAMETERS params);
    PWLAN_CONNECTION_PARAMETERS AllocConnectParams(char *ssid, DOT11_MAC_ADDRESS *pbssid, int bssidNum);
    void ReleaseParams(PWLAN_CONNECTION_PARAMETERS params);
    DWORD GetWlanIntfInt(WLAN_INTF_OPCODE op, int *val);
    DWORD SetWlanIntfInt(WLAN_INTF_OPCODE op, int val);
    DWORD Scan(PSCAN_PARAM params);
    PSCAN_PARAM AllocScanParams(char *ssid, char *rawData, DWORD dataByteLen);
    void ReleaseScanParams(PSCAN_PARAM params);

public:
    char *GetDescription();
    char *GetGUIDStr();
    bool CompareGUID(GUID *guid);
    bool CompareGUID(char *guidStr);
    void OnNotification(PWLAN_NOTIFICATION_DATA data);
    void SetNotifyCallback(WLANINTERFACENOTIFY func);
    const char *FormatNotificationStr(int source, int code);
    const char *FormatIntfStateStr(int state);
    const char *FormatIntfStateStr();
    DWORD Connect(char *ssid);
    DWORD Connect(char *ssid, DOT11_MAC_ADDRESS bssidArray[], int bssidNum);
    DWORD Scan();
    DWORD Scan(char *ssid);
    DWORD Scan(char *ssid, char *rawdata, DWORD dataByteLen);
    DWORD AutoConf(bool set, int *pbuf);
    DWORD BssType(bool set, int *pbuf);
    DWORD IntfState(int *pbuf);
    DWORD Channel(int *pbuf);
    DWORD Rssi(int *pbuf);
    DWORD MediaStreamMode(bool set, int *pbuf);
    DWORD BackgroudScan(bool set, int *pbuf);
    DWORD CurOpMode(int *pbuf);
    DWORD GetBssList(char *ssid, PBSS_INFO pBss, int *num);
    bool IsConnect();
    PBSS_INFO GetCurBssInfo();
    void UpdateCurBssInfo();
};
