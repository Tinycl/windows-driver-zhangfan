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

	WCHAR* InputBuffer = L"\\Driver\\ACPI";

	DWORD dwOutput;
	//���뻺������Ϊ���룬�����������Ϊ���

	BOOL bRet;
	bRet = DeviceIoControl(hDevice, IOCTL_DUMP_DEVICE_STACK, InputBuffer, wcslen(InputBuffer)*2+2, NULL, 0, &dwOutput, NULL);

	CloseHandle(hDevice);

	return 0;
}