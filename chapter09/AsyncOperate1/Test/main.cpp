#include <windows.h>
#include <stdio.h>

#define BUFFER_SIZE	512
//������ļ����ڻ����BUFFER_SIZE

#define DEVICE_NAME	"test.dat"
int main()
{
	HANDLE hDevice = 
		CreateFile("test.dat",
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,//�˴�����FILE_FLAG_OVERLAPPED
					NULL );

	if (hDevice == INVALID_HANDLE_VALUE) 
	{
		printf("Read Error\n");
		return 1;
	}

	UCHAR buffer[BUFFER_SIZE];
	DWORD dwRead;

	//��ʼ��overlapʹ���ڲ�ȫ��Ϊ��
	OVERLAPPED overlap={0};

	//����overlap�¼�
	overlap.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	//����û������OVERLAP������������첽����
	ReadFile(hDevice,buffer,BUFFER_SIZE,&dwRead,&overlap);

	//��һЩ������������Щ����������豸����ִ��

	//�ȴ����豸����
	WaitForSingleObject(overlap.hEvent,INFINITE);

	CloseHandle(hDevice);

	return 0;
}

