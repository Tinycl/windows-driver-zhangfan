#include <windows.h>
#include <stdio.h>
//ʹ��CTL_CODE�������winioctl.h
#include <winioctl.h>
#include "..\NT_Driver\Ioctls.h"


UCHAR In_8(HANDLE hDevice,USHORT port)
{
	DWORD dwOutput ;
	DWORD inputBuffer[2] =
	{
		port,//��port���в���
		1//1����8λ������2����16λ������4����32λ����
	};
	DWORD dResult;

	DeviceIoControl(hDevice, READ_PORT, inputBuffer, sizeof(inputBuffer), &dResult, sizeof(DWORD), &dwOutput, NULL);

	return (UCHAR) dResult;
	
}
void Out_8(HANDLE hDevice,USHORT port,UCHAR value)
{
	DWORD dwOutput ;
	DWORD inputBuffer[3] =
	{
		port,//��port���в���
		1,//1����8λ������2����16λ������4����32λ����
		value//����ֽ�
	};

	DeviceIoControl(hDevice, WRITE_PORT, inputBuffer, sizeof(inputBuffer), NULL, 0, &dwOutput, NULL);
}

//��������,����f����Ƶ��
void Sound(HANDLE hDevice,int f)
{  
	//����Ϊ1193180/F
	USHORT   B=1193180/f; 

	//�Ӷ˿�0x61ȡ��  
	UCHAR temp = In_8(hDevice,0x61);
	//����λ��1 
	temp = temp | 3; 
	//�����0x61�˿�
	Out_8(hDevice,0x61,temp);

	//�����0x61�˿�
	Out_8(hDevice,0x43,0xB6);
	//�����0x42�˿ڣ�д��8λ
	Out_8(hDevice,0x42,B&0xF);
	//�����0x42�˿ڣ�д��8λ
	Out_8(hDevice,0x42,(B>>8)&0xF);
}

// �ر�����  
void SoundOff(HANDLE hDevice)                               
{
	//ȡ�˿�0x61���ֽ�  
	UCHAR value = In_8(hDevice,0x61);

	//ǿ���������λΪ0 
	value = value & 0xFC;

	//���Ͷ˿�0x61  
	Out_8(hDevice,0x61,value);
}

int main()
{
	HANDLE hDevice = 
		CreateFile("\\\\.\\HelloDDK",
					GENERIC_READ | GENERIC_WRITE,
					0,		// share mode none
					NULL,	// no security
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL );		// no template

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Failed to obtain file handle to device: "
			"%s with Win32 error code: %d\n",
			"MyWDMDevice", GetLastError() );
		return 1;
	}

	//����2KHzƵ�ʵ�����
	Sound(hDevice,2000);
	//����200����
	Sleep(200);
	SoundOff(hDevice);

	CloseHandle(hDevice);

	return 0;
}