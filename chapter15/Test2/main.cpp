#include <Windows.h>
#include <stdio.h>

#include ".\winiolib\WinIo.h"

int main()
{
	//��WinIO����
	bool bRet = InitializeWinIo();
	if (bRet)
	{
		printf("Load Dirver successfully!\n");

		//��0x378�˿ڽ����������,8λ����
		SetPortVal(0x378,0,1);

		//�ر�WinIO����
		ShutdownWinIo();
	}

	
	return 0;
}
