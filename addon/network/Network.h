#pragma once

#include <windows.h>
#include <stdio.h>
#include <iphlpapi.h>

#define ADAPTERNAME_SIZE    (MAX_ADAPTER_NAME_LENGTH + 4)
#define ADAPTERDESC_SIZE    (MAX_ADAPTER_DESCRIPTION_LENGTH + 4)
#define IPSTRING_SIZE        (16)

typedef struct {
    DWORD downBytes;
    DWORD upBytes;
    DWORD downBits;
    DWORD upBits;
    DWORD maxRateMbps;
} NETSTATE, *PNETSTATE;

class CNetworkIf {
private:
    DWORD _index;
    char _desc[ADAPTERDESC_SIZE];
    char _guid[ADAPTERNAME_SIZE];
    int  _macAddressLen;
    UCHAR _mac[MAX_ADAPTER_ADDRESS_LENGTH];
    char _curIpStr[IPSTRING_SIZE];
    char _curIpMaskStr[IPSTRING_SIZE];
    char _gatewayIpStr[IPSTRING_SIZE];
    int _rateMbps;
    bool _dhcpEnable;
    void *_pIfRow;

    void Init();
    void ParseInfo(PIP_ADAPTER_INFO pAdapter, MIB_IFROW *pIfRow);

public:
    CNetworkIf(const char *guid);
    ~CNetworkIf();
    DWORD UpdateIfInfo();
    bool IsValid();
    UCHAR *GetMacAddr();
    char *GetDesc();
    char *GetCurIpStr();
    char *GetCurIpMaskStr();
    char *GetGatewayIpStr();
    int GetMaxRateMbps();
    bool IsDhcpEnable();
    DWORD GetNetState(PNETSTATE pst);
    DWORD GetIPSt(MIB_IPSTATS *pst);
    DWORD EnableIf(bool enable);
};
