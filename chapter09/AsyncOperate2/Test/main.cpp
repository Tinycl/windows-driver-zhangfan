#include <windows.h>
#include <stdio.h>

#define DEVICE_NAME	"test.dat"
#define BUFFER_SIZE	512
//������ļ����ڻ����BUFFER_SIZE

VOID CALLBACK MyFileIOCompletionRoutine(
  DWORD dwErrorCode,                // ���ڴ˴β������ص�״̬
  DWORD dwNumberOfBytesTransfered,  // �����Ѿ������˶����ֽ�,Ҳ������IRP���Infomation
  LPOVERLAPPED lpOverlapped         // ������ݽṹ
)
{
	printf("IO operation end!\n");
}

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

	//��ʼ��overlapʹ���ڲ�ȫ��Ϊ��
	//���ó�ʼ���¼�!!
	OVERLAPPED overlap={0};

	//����û������OVERLAP������������첽����
	ReadFileEx(hDevice, buffer, BUFFER_SIZE,&overlap,MyFileIOCompletionRoutine);

	//��һЩ������������Щ����������豸����ִ��

	//����alterable
	SleepEx(0,TRUE);

	CloseHandle(hDevice);

	return 0;
}

