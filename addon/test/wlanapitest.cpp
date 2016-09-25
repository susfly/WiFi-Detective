// wlanapitest.cpp : 定义控制台应用程序的入口点。
//

// wlanapitest.cpp : 定义控制台应用程序的入口点。
//

#include "../header/stdafx.h"
#include <stdlib.h>
#include "../wlan/Wlan.h"
#include "../wlan/WlanInterface.h"
#include "../network/Network.h"

#include <iostream>
using namespace std;

void Callback(WLANINTERFACE_NOTIFICATION *data, CWlanInterface *intf)
{
	cout << intf->GetGUIDStr() << "   " << intf->GetDescription() << "callback" << data->source << data->code << endl;
	cout << intf->FormatNotificationStr(data->source, data->code) << endl;
}

bool WINAPI print(CWlanInterface *intf, LPVOID context)
{
	cout << intf->GetDescription() << endl;
	cout << intf->GetGUIDStr() << endl;

	return false;
}

const char *str =  "{833DFD79-E162-424B-9E86-AE87391F5033}";
const char *strxx =  "{DBE75F42-B38D-43BB-8971-E0E44B2EB4BE}";
const char *strxxx =  "{5D072CD9-E286-46D8-90A8-AA7D541B6E9E}";

int _tmain(int argc, _TCHAR* argv[])
{
#if 0
	CNetworkIf *ifs = new CNetworkIf(str);

	cout << ifs->UpdateIfInfo() << endl;

	NETSTATE ss;
	bool first = true;
	DWORD lastDownBits;
	DWORD lastdownBytes;
	DWORD lastupBits;
	DWORD lastupBytes;

	while (1) {
		Sleep(1000);
		memset(&ss, 0 ,sizeof(ss));
		cout << ifs->GetNetState(&ss) << endl;
		cout << ss.maxRateMbps <<endl;
		//cout << "downBits " << ss.downBits<<endl;
		//cout << "downBytes " << ss.downBytes<<endl;
		//cout << "upBits " << ss.upBits<<endl;
		//cout << "upBytes " << ss.upBytes<<endl;
		if (first) {
			first = false;
			lastDownBits = ss.downBits;
			lastdownBytes = ss.downBytes;
			lastupBits = ss.upBits;
			lastupBytes = ss.upBytes;
			continue;
		}
		
		cout << "downspeed " << (ss.downBits - lastDownBits) / 1000.0 << " kbps" << endl;
		cout << "downspeed " << (ss.downBytes - lastdownBytes) / 1000.0 << " kBps" << endl;
		cout << "upspeed " << (ss.upBits - lastupBits) / 1000.0 << " kbps" << endl;
		cout << "upspeed " << (ss.upBytes - lastupBytes) / 1000.0 << " kBps" << endl;
		lastDownBits = ss.downBits;
		lastdownBytes = ss.downBytes;
		lastupBits = ss.upBits;
		lastupBytes = ss.upBytes;
		//cout << ifs->GetMaxRateMbps()<<endl;
	}
#else
	CWlan wlan;
	INTFLIST *ifList;
	CWlanInterface *intf;
	PBSS_INFO p;
	NETSTATE ss;

	CNetworkIf *ifs = new CNetworkIf(str);
	cout << ifs->UpdateIfInfo() << endl;
	cout << ifs->GetCurIpStr() << endl;
	UCHAR *mac = ifs->GetMacAddr();
	unsigned char buf[8];
	char str[16], str1[16], str2[16];
	memset(str, 0, sizeof(str));
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memcpy(buf, mac, 6);
    sprintf(str, "%2x%2x.%2x%2x.%2x%2x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	sprintf(str1, "%2x%2x.%2x%2x.%2x%2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	sprintf(str2, "%02x%02x.%02x%02x.%02x%02x", 0, 1, 2, 3, 4, 5);
	cout << str << endl;
	cout << str1 << endl;
	cout << str2 << endl;
#if 1
	wlan.WlanInit();

	ifList = wlan.GetInterfaces();

	cout << wlan.GetInterfacesNum() << endl;

	wlan.ForeachInterface(print, NULL);

	ifList = wlan.RefreshInterfaces();

	cout << wlan.GetInterfacesNum() << endl;

	wlan.ForeachInterface(print, NULL);
	const char *aaa="{833DFD79-E162-424b-9E86-AE87391F5033}";
	intf = wlan.FindInterfaceByGUID(aaa);

	//intf = *ifList->begin();
	if (intf == NULL) {
		system("pause");
		return 0;
	}

	intf->SetNotifyCallback(Callback);
	
	int num = 200;
	PBSS_INFO pBss = (PBSS_INFO)malloc(sizeof(BSS_INFO) * num);
    if (pBss == NULL) {
        return 0;
    }
	//cout << intf->Connect("ruijie-web");

	while (1) {
		Sleep(1000);
		intf->UpdateCurBssInfo();
		p = intf->GetCurBssInfo();
		cout << p->ssid << "+" << p->lRssi << "+" << p->ulChCenterFrequency << endl;
		cout << "rxrate " << p->ulRxRate << " txrate " << p->ulTxRate  << " uLinkQuality " << p->uLinkQuality << endl;
		memset(&ss, 0 ,sizeof(ss));
		cout << ifs->GetNetState(&ss) << endl;
		cout << ss.maxRateMbps <<endl;
		int ret = intf->GetBssList(NULL, pBss, &num);
		cout << "getbsslist ret " << ret << endl;
		if (ret == ERROR_SUCCESS) {
			for (int i = 0; i < num; i++) {
				char buf[1024];
				memset(buf, 0, 1024); 
				cout << i  << " " << pBss[i].ssid << " " << pBss[i].ulChCenterFrequency << " " <<endl;
			}
		}
	}
#endif
#endif
	system("pause");

	return 0;
}



