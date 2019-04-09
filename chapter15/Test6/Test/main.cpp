#include <windows.h>
#include <stdio.h>
//ʹ��CTL_CODE�������winioctl.h
#include <winioctl.h>
#include "..\NT_Driver\Ioctls.h"

UCHAR In_8 (PUCHAR Port)
{
   UCHAR Value;
	__asm
	{
		mov edx, Port
		in al, dx
		mov Value, al
		//���뼸����ָ��
		nop
		nop
	}

   return(Value);
}

USHORT In_16 (PUSHORT Port)
{
   USHORT Value;

	__asm
	{
		mov edx, Port
		in ax, dx
		mov Value, ax
		//���뼸����ָ��
		nop
		nop
	}
   return(Value);
}

ULONG In_32 (PULONG Port)
{
   ULONG Value;
	__asm
	{
		mov edx, Port
		in eax, dx
		mov Value, eax
		//���뼸����ָ��
		nop
		nop
	}
   return(Value);
}

void Out_32(PULONG Port,ULONG Value)
{
	__asm
	{
		mov edx, Port
		mov eax, Value
		out dx,eax
		//���뼸����ָ��
		nop
		nop
	}
}
void Out_16 (PUSHORT Port,USHORT Value)
{
	__asm
	{
		mov edx, Port
		mov ax, Value
		out dx,ax
		//���뼸����ָ��
		nop
		nop
	}
}

void Out_8 (PUCHAR Port,UCHAR Value)
{
	__asm
	{
		mov edx, Port
		mov al, Value
		out dx,al
		//���뼸����ָ��
		nop
		nop
	}
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

	DWORD dwOutput ;
	//����ֱ��IO
	DeviceIoControl(hDevice, IOCTL_ENABLEDIRECTIO, NULL,0, NULL, 0, &dwOutput, NULL);

	Out_8((PUCHAR)0x378,0);

	//�ر�ֱ��IO
	DeviceIoControl(hDevice, IOCTL_DISABLEDIRECTIO, NULL,0, NULL, 0, &dwOutput, NULL);

	CloseHandle(hDevice);

	return 0;
}