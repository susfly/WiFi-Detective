// wlanapitest.cpp : 定义控制台应用程序的入口点。
//

// wlanapitest.cpp : 定义控制台应用程序的入口点。
//

#include "header/stdafx.h"
#include <stdlib.h>
#include "Wlan.h"
#include "WlanInterface.h"
#include "Network.h"

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
	CNetworkIf *ifs = new CNetworkIf(strxxx);

	cout << ifs->UpdateIfInfo() << endl;

	IFINOUTST ss;

	while (1) {
		Sleep(1000);
		memset(&ss, 0 ,sizeof(ss));
		cout << ifs->GetNetState(&ss) << endl;
		cout << ss.aa<<endl;
		//cout << ifs->UpdateIfInfo()<<endl;
		//cout << ifs->GetMaxRateMbps()<<endl;
	}
#if 0
	CWlan wlan;
	INTFLIST *ifList;
	CWlanInterface *intf;
	PBSS_INFO p;

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

	cout << intf->Connect("ruijie-web");

	while (1) {
		Sleep(1000);

		p = intf->GetCurBssInfo();
		cout << p->ssid << "+" << p->lRssi << "+" << p->ulChCenterFrequency << endl;
	}
#endif
	system("pause");

	return 0;
}



