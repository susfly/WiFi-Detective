#pragma once

#ifndef _WLAN_H_
#define _WLAN_H_

#include <windows.h>
#include <wlanapi.h>
#include <list>
#include "wlan_def.h"

typedef std::list<CWlanInterface *> INTFLIST;

typedef bool (WINAPI *FOREACHINTERFACES)(CWlanInterface *intf, PVOID context);

class CWlan
{
public:
	CWlan();
	~CWlan();

/* Todo: add here */
private:
	DWORD _dwMaxClient;
	DWORD _dwCurVersion;
	INTFLIST _pIntfList;
	HANDLE _hWlan;

	void ClearInterfaces();

public:
	void DeletIntf(CWlanInterface *intf);
	DWORD WlanInit();
	void WlanDeInit();
	INTFLIST *GetInterfaces();
	INTFLIST *RefreshInterfaces();
	int GetInterfacesNum();
	/* if func return true, end foreach */
	void ForeachInterface(FOREACHINTERFACES func, PVOID context);
	CWlanInterface *FindInterfaceByGUID(const char *guidStr);
	CWlanInterface *FindInterfaceByGUID(GUID *guid);
	CWlanInterface *GetFirstInterface();
	void OnWlanNotify(PWLAN_NOTIFICATION_DATA data, PVOID context);
};

#endif /* _WLAN_H_ */
