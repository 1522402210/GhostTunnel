#pragma once
//payload�ṹ��
struct ie_data	//3���ֽ�
{
	unsigned char id;
	unsigned char len;
	unsigned char val[1];
};

struct STATUS_WLAN {
	DWORD STATUS_handle;
	DWORD STATUS_wlanList;
	DWORD STATUS_sendRequest;
};