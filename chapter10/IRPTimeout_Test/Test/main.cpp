#include <windows.h>
#include <stdio.h>
#include <process.h>

int main()
{
	HANDLE hDevice = 
		CreateFile("\\\\.\\HelloDDK",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL );

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Open Device failed!");
		return 1;
	}

	DWORD dwRead;

	//�����IRPû�б���ɣ�ReadFileһֱ�������˳�!
	ReadFile(hDevice,NULL,0,&dwRead,NULL);
	
	ReadFile(hDevice,NULL,0,&dwRead,NULL);

	CloseHandle(hDevice);

	return 0;
}