/*
 * Copyright(C) 2016 WIS Team WIFI Detector. ALL rights reserved.
 */
/*
 * wlanMidware.cpp
 * Original Author: han	2016-08-05
 * operations for wlanapi
 */

#include "Wlan.h"
#include "WlanInterface.h"
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

CWlan::CWlan()
{
	this->_dwMaxClient = 2;
	this->_dwCurVersion = 0;
	this->_hWlan = NULL;
}

CWlan::~CWlan()
{
	this->WlanDeInit();
}

static void WINAPI WlanCallbackNotify(PWLAN_NOTIFICATION_DATA data, PVOID context)
{
	CWlan *wlan = (CWlan *)context;

	wlan->OnWlanNotify(data, context);
}

static bool WINAPI ReleaseInterface(CWlanInterface *intf, PVOID context)
{
	CWlan *wlan = (CWlan *)context;

	wlan->DeletIntf(intf);

	return false;
}

static bool WINAPI GuidCompare(CWlanInterface *intf, PVOID context)
{
	CVOIDCONPARAM *con = (CVOIDCONPARAM *)context;
	GUID *guid = (GUID *)con->context;
	CWlanInterface **pIntf = (CWlanInterface **)con->param;

	if (intf->CompareGUID(guid)) {
		*pIntf = intf;
		return true;
	}
	else {
		return false;
	}
}

static bool WINAPI GuidStrCompare(CWlanInterface *intf, PVOID context)
{
	CVOIDCONPARAM *con = (CVOIDCONPARAM *)context;
	char *guidStr = (char *)con->context;
	CWlanInterface **pIntf = (CWlanInterface **)con->param;

	if (intf->CompareGUID(guidStr)) {
		*pIntf = intf;
		return true;
	} else {
		return false;
	}
}

DWORD CWlan::WlanInit()
{
	DWORD dwRes = ERROR_SUCCESS;

	dwRes = WlanOpenHandle(this->_dwMaxClient, NULL, &this->_dwCurVersion, &this->_hWlan);
	if (dwRes != ERROR_SUCCESS) {
		return dwRes;
	}

	DWORD prevSource;
	dwRes = WlanRegisterNotification(this->_hWlan, WLAN_NOTIFICATION_SOURCE_ALL, FALSE, WlanCallbackNotify, this, NULL, &prevSource);
	if (dwRes != ERROR_SUCCESS) {
		WlanDeInit();
		return dwRes;
	}

	return dwRes;
}

void CWlan::WlanDeInit()
{
	if (this->_hWlan != NULL) {
		WlanCloseHandle(this->_hWlan, NULL);
		this->_hWlan = NULL;
	}

	if (this->GetInterfacesNum()) {
		this->ClearInterfaces();
	}
}

void CWlan::DeletIntf(CWlanInterface *intf)
{
	if (intf) {
		delete intf;
	}
}

void CWlan::ClearInterfaces()
{
	if (this->_pIntfList.empty()) {
		return;
	}

	this->ForeachInterface(ReleaseInterface, this);

	this->_pIntfList.clear();
}

INTFLIST *CWlan::GetInterfaces()
{
	DWORD dwRes;
	PWLAN_INTERFACE_INFO_LIST pIfList;
	DWORD i;

	dwRes = WlanEnumInterfaces(this->_hWlan, NULL, &pIfList);
	if (dwRes != ERROR_SUCCESS) {
		return NULL;
	}

	for (i = 0; i < pIfList->dwNumberOfItems; i++) {
		CWlanInterface *intf = new CWlanInterface(this->_hWlan, &pIfList->InterfaceInfo[i]);
		this->_pIntfList.push_back(intf);
	}

	WlanFreeMemory(pIfList);

	return &this->_pIntfList;
}

int CWlan::GetInterfacesNum()
{
	return (int)this->_pIntfList.size();
}

INTFLIST *CWlan::RefreshInterfaces()
{
	this->ClearInterfaces();

	return this->GetInterfaces();
}

void CWlan::ForeachInterface(FOREACHINTERFACES func, PVOID context)
{
	INTFLIST::iterator intf;
	bool res;

	if (func == NULL) {
		return;
	}

	for (intf = this->_pIntfList.begin(); intf != this->_pIntfList.end(); ++intf) {
		res = func(*intf, context);
		if (res) {
			break;
		}
	}
}

CWlanInterface *CWlan::FindInterfaceByGUID(GUID *guid)
{
	CVOIDCONPARAM *con = new CVOIDCONPARAM;
	CWlanInterface *intf = NULL;

	con->context = (PVOID)guid;
	con->param = &intf;

	this->ForeachInterface(GuidCompare, con);

	delete con;

	return intf;
}

CWlanInterface *CWlan::FindInterfaceByGUID(const char *guidStr)
{
	CVOIDCONPARAM *con = new CVOIDCONPARAM;
	CWlanInterface *intf = NULL;

	con->context = (PVOID)guidStr;
	con->param = &intf;

	this->ForeachInterface(GuidStrCompare, con);

	delete con;

	return intf;
}

CWlanInterface *CWlan::GetFirstInterface()
{
	INTFLIST::iterator intf;

	if (this->_pIntfList.size() == 0) {
		return NULL;
	}

	intf = this->_pIntfList.begin();
	return (*intf);
}

void CWlan::OnWlanNotify(PWLAN_NOTIFICATION_DATA data, PVOID context)
{
	CWlanInterface *intf;

	intf = this->FindInterfaceByGUID(&data->InterfaceGuid);
	if (intf == NULL) {
		return;
	}

	intf->OnNotification(data);
}
