#include <windows.h>
#include <stdio.h>
//ʹ��CTL_CODE�������winioctl.h
#include <winioctl.h>
#include "..\NT_Driver\Ioctls.h"

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

	DWORD dwOutput ;
	DWORD inputBuffer[3] =
	{
		0x378,//��0x378���в���
		1,//1����8λ������2����16λ������4����32λ����
		2//����ֽ�
	};

	//����������200����������ź�
	for (;;)
	{
		DeviceIoControl(hDevice, WRITE_PORT, inputBuffer, sizeof(inputBuffer), NULL, 0, &dwOutput, NULL);
		inputBuffer[2] = inputBuffer[2] ^3;//�õ�0��1λ��
		Sleep(100);//��ͣ100����
	}

	CloseHandle(hDevice);

	return 0;
}