/************************************************************************
* �ļ�����:Driver.cpp                                                 
* ��    ��:�ŷ�
* �������:2007-11-1
*************************************************************************/

#include "Driver.h"

#pragma INITCODE
VOID CreateFileTest() 
{
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK iostatus;
	HANDLE hfile;
	UNICODE_STRING logFileUnicodeString;

	//��ʼ��UNICODE_STRING�ַ���
	RtlInitUnicodeString( &logFileUnicodeString, 
		L"\\??\\C:\\1.log");
	//����д�� "\\Device\\HarddiskVolume1\\1.LOG"


	//��ʼ��objectAttributes
	InitializeObjectAttributes(&objectAttributes, 
							&logFileUnicodeString,
							OBJ_CASE_INSENSITIVE, 
							NULL, 
							NULL );

	//�����ļ�
	NTSTATUS ntStatus = ZwCreateFile( &hfile, 
							GENERIC_WRITE,
							&objectAttributes, 
							&iostatus, 
							NULL,
							FILE_ATTRIBUTE_NORMAL, 
							FILE_SHARE_READ,
							FILE_OPEN_IF,//��ʹ���ڸ��ļ���Ҳ���� 
							FILE_SYNCHRONOUS_IO_NONALERT, 
							NULL, 
							0 );
	if ( NT_SUCCESS(ntStatus))
	{
		KdPrint(("Create file succussfully!\n"));
	}else
	{
		KdPrint(("Create file  unsuccessfully!\n"));
	}

	//�ļ�����
	//.......

	//�ر��ļ����
	ZwClose(hfile);
}


#pragma INITCODE
VOID OpenFileTest2() 
{
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK iostatus;
	HANDLE hfile;
	UNICODE_STRING logFileUnicodeString;

	//��ʼ��UNICODE_STRING�ַ���
	RtlInitUnicodeString( &logFileUnicodeString, 
		L"\\??\\C:\\1.log");
	//����д�� "\\Device\\HarddiskVolume1\\1.LOG"


	//��ʼ��objectAttributes
	InitializeObjectAttributes(&objectAttributes, 
							&logFileUnicodeString,
							OBJ_CASE_INSENSITIVE, 
							NULL, 
							NULL );

	//�����ļ�
	NTSTATUS ntStatus = ZwOpenFile( &hfile, 
							GENERIC_ALL,
							&objectAttributes, 
							&iostatus, 
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							FILE_SYNCHRONOUS_IO_NONALERT);
	if ( NT_SUCCESS(ntStatus))
	{
		KdPrint(("Create file succussfully!\n"));
	}else
	{
		KdPrint(("Create file  unsuccessfully!\n"));
	}

	//�ļ�����
	//.......

	//�ر��ļ����
	ZwClose(hfile);
}


#pragma INITCODE
VOID OpenFileTest1() 
{
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK iostatus;
	HANDLE hfile;
	UNICODE_STRING logFileUnicodeString;

	//��ʼ��UNICODE_STRING�ַ���
	RtlInitUnicodeString( &logFileUnicodeString, 
		L"\\??\\C:\\1.log");
	//����д�� "\\Device\\HarddiskVolume1\\1.LOG"

	//��ʼ��objectAttributes
	InitializeObjectAttributes(&objectAttributes,
							&logFileUnicodeString,
							OBJ_CASE_INSENSITIVE,//�Դ�Сд���� 
							NULL, 
							NULL );

	//�����ļ�
	NTSTATUS ntStatus = ZwCreateFile( &hfile, 
							GENERIC_READ,
							&objectAttributes, 
							&iostatus, 
							NULL,
							FILE_ATTRIBUTE_NORMAL, 
							FILE_SHARE_WRITE,
							FILE_OPEN,//���ļ��򿪣�����������򷵻ش��� 
							FILE_SYNCHRONOUS_IO_NONALERT, 
							NULL, 
							0 );
	if ( NT_SUCCESS(ntStatus))
	{
		KdPrint(("Open file succussfully!\n"));
	}else
	{
		KdPrint(("Open file  unsuccessfully!\n"));
	}

	//�ļ�����
	//.......

	//�ر��ļ����
	ZwClose(hfile);
}


#pragma INITCODE
VOID FileAttributeTest() 
{
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK iostatus;
	HANDLE hfile;
	UNICODE_STRING logFileUnicodeString;

	//��ʼ��UNICODE_STRING�ַ���
	RtlInitUnicodeString( &logFileUnicodeString, 
		L"\\??\\C:\\1.log");
	//����д�� "\\Device\\HarddiskVolume1\\1.LOG"

	//��ʼ��objectAttributes
	InitializeObjectAttributes(&objectAttributes,
							&logFileUnicodeString,
							OBJ_CASE_INSENSITIVE,//�Դ�Сд���� 
							NULL, 
							NULL );

	//�����ļ�
	NTSTATUS ntStatus = ZwCreateFile( &hfile, 
							GENERIC_READ,
							&objectAttributes, 
							&iostatus, 
							NULL,
							FILE_ATTRIBUTE_NORMAL, 
							0,
							FILE_OPEN,//���ļ��򿪣�����������򷵻ش��� 
							FILE_SYNCHRONOUS_IO_NONALERT, 
							NULL, 
							0 );
	if (NT_SUCCESS(ntStatus))
	{
		KdPrint(("open file successfully.\n"));
	}

	FILE_STANDARD_INFORMATION fsi;
	//��ȡ�ļ�����
	ntStatus = ZwQueryInformationFile(hfile,
									&iostatus,
									&fsi,
									sizeof(FILE_STANDARD_INFORMATION),
									FileStandardInformation);
	if (NT_SUCCESS(ntStatus))
	{
		KdPrint(("file length:%u\n",fsi.EndOfFile.QuadPart));
	}
	
	//�޸ĵ�ǰ�ļ�ָ��
	FILE_POSITION_INFORMATION fpi;
	fpi.CurrentByteOffset.QuadPart = 100i64;
	ntStatus = ZwSetInformationFile(hfile,
								&iostatus,
								&fpi,
								sizeof(FILE_POSITION_INFORMATION),
								FilePositionInformation);
	if (NT_SUCCESS(ntStatus))
	{
		KdPrint(("update the file pointer successfully.\n"));
	}

	//�ر��ļ����
	ZwClose(hfile);
}


#pragma INITCODE
VOID WriteFileTest() 
{
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK iostatus;
	HANDLE hfile;
	UNICODE_STRING logFileUnicodeString;

	//��ʼ��UNICODE_STRING�ַ���
	RtlInitUnicodeString( &logFileUnicodeString, 
		L"\\??\\C:\\1.log");
	//����д�� "\\Device\\HarddiskVolume1\\1.LOG"

	//��ʼ��objectAttributes
	InitializeObjectAttributes(&objectAttributes,
							&logFileUnicodeString,
							OBJ_CASE_INSENSITIVE,//�Դ�Сд���� 
							NULL, 
							NULL );

	//�����ļ�
	NTSTATUS ntStatus = ZwCreateFile( &hfile, 
							GENERIC_WRITE,
							&objectAttributes, 
							&iostatus, 
							NULL,
							FILE_ATTRIBUTE_NORMAL, 
							FILE_SHARE_WRITE,
							FILE_OPEN_IF,//��ʹ���ڸ��ļ���Ҳ���� 
							FILE_SYNCHRONOUS_IO_NONALERT, 
							NULL, 
							0 );
#define BUFFER_SIZE 1024
	PUCHAR pBuffer = (PUCHAR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	//����Ҫ��������
	RtlFillMemory(pBuffer,BUFFER_SIZE,0xAA);

	KdPrint(("The program will write %d bytes\n",BUFFER_SIZE));
	//д�ļ�
	ZwWriteFile(hfile,NULL,NULL,NULL,&iostatus,pBuffer,BUFFER_SIZE,NULL,NULL);
	KdPrint(("The program really wrote %d bytes\n",iostatus.Information));


	//����Ҫ��������
	RtlFillMemory(pBuffer,BUFFER_SIZE,0xBB);

	KdPrint(("The program will append %d bytes\n",BUFFER_SIZE));
	//׷������
	LARGE_INTEGER number;
	number.QuadPart = 1024i64;//�����ļ�ָ��
	//���ļ����и���д
	ZwWriteFile(hfile,NULL,NULL,NULL,&iostatus,pBuffer,BUFFER_SIZE,&number,NULL);
	KdPrint(("The program really appended %d bytes\n",iostatus.Information));

	//�ر��ļ����
	ZwClose(hfile);

	ExFreePool(pBuffer);
}
#pragma INITCODE
VOID ReadFileTest() 
{
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK iostatus;
	HANDLE hfile;
	UNICODE_STRING logFileUnicodeString;

	//��ʼ��UNICODE_STRING�ַ���
	RtlInitUnicodeString( &logFileUnicodeString, 
		L"\\??\\C:\\1.log");
	//����д�� "\\Device\\HarddiskVolume1\\1.LOG"

	//��ʼ��objectAttributes
	InitializeObjectAttributes(&objectAttributes,
							&logFileUnicodeString,
							OBJ_CASE_INSENSITIVE,//�Դ�Сд���� 
							NULL, 
							NULL );

	//�����ļ�
	NTSTATUS ntStatus = ZwCreateFile( &hfile, 
							GENERIC_READ,
							&objectAttributes, 
							&iostatus, 
							NULL,
							FILE_ATTRIBUTE_NORMAL, 
							FILE_SHARE_READ,
							FILE_OPEN,//��ʹ���ڸ��ļ���Ҳ���� 
							FILE_SYNCHRONOUS_IO_NONALERT, 
							NULL, 
							0 );

	if (!NT_SUCCESS(ntStatus))
	{
		KdPrint(("The file is not exist!\n"));
		return;
	}

	FILE_STANDARD_INFORMATION fsi;
	//��ȡ�ļ�����
	ntStatus = ZwQueryInformationFile(hfile,
									&iostatus,
									&fsi,
									sizeof(FILE_STANDARD_INFORMATION),
									FileStandardInformation);

	KdPrint(("The program want to read %d bytes\n",fsi.EndOfFile.QuadPart));

	//Ϊ��ȡ���ļ����仺����
 	PUCHAR pBuffer = (PUCHAR)ExAllocatePool(PagedPool,
								(LONG)fsi.EndOfFile.QuadPart);

	//��ȡ�ļ�
	ZwReadFile(hfile,NULL,
				NULL,NULL,
				&iostatus,
				pBuffer,
				(LONG)fsi.EndOfFile.QuadPart,
				NULL,NULL);
	KdPrint(("The program really read %d bytes\n",iostatus.Information));
	//�ر��ļ����
	ZwClose(hfile);

	//�ͷŻ�����
	ExFreePool(pBuffer);
}
#pragma INITCODE
VOID FileTest() 
{
	//�����ļ�ʵ��
//	CreateFileTest();

	//���ļ�ʵ��
//	OpenFileTest1();
	OpenFileTest2();

//	FileAttributeTest();

	//д�ļ���׷���ļ�ʵ��
//	WriteFileTest();

//	ReadFileTest();

}

/************************************************************************
* ��������:DriverEntry
* ��������:��ʼ���������򣬶�λ������Ӳ����Դ�������ں˶���
* �����б�:
      pDriverObject:��I/O�������д���������������
      pRegistryPath:����������ע�����е�·��
* ���� ֵ:���س�ʼ������״̬
*************************************************************************/
#pragma INITCODE
extern "C" NTSTATUS DriverEntry (
			IN PDRIVER_OBJECT pDriverObject,
			IN PUNICODE_STRING pRegistryPath	) 
{
	NTSTATUS status;
	KdPrint(("Enter DriverEntry\n"));

	//ע�������������ú������
	pDriverObject->DriverUnload = HelloDDKUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKDispatchRoutine;
	
	//���������豸����
	status = CreateDevice(pDriverObject);

	FileTest();

	KdPrint(("DriverEntry end\n"));
	return status;
}

/************************************************************************
* ��������:CreateDevice
* ��������:��ʼ���豸����
* �����б�:
      pDriverObject:��I/O�������д���������������
* ���� ֵ:���س�ʼ��״̬
*************************************************************************/
#pragma INITCODE
NTSTATUS CreateDevice (
		IN PDRIVER_OBJECT	pDriverObject) 
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
	
	//�����豸����
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName,L"\\Device\\MyDDKDevice");
	
	//�����豸
	status = IoCreateDevice( pDriverObject,
						sizeof(DEVICE_EXTENSION),
						&(UNICODE_STRING)devName,
						FILE_DEVICE_UNKNOWN,
						0, TRUE,
						&pDevObj );
	if (!NT_SUCCESS(status))
		return status;

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDevice = pDevObj;
	pDevExt->ustrDeviceName = devName;
	//������������
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName,L"\\??\\HelloDDK");
	pDevExt->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink( &symLinkName,&devName );
	if (!NT_SUCCESS(status)) 
	{
		IoDeleteDevice( pDevObj );
		return status;
	}
	return STATUS_SUCCESS;
}

/************************************************************************
* ��������:HelloDDKUnload
* ��������:�������������ж�ز���
* �����б�:
      pDriverObject:��������
* ���� ֵ:����״̬
*************************************************************************/
#pragma PAGEDCODE
VOID HelloDDKUnload (IN PDRIVER_OBJECT pDriverObject) 
{
	PDEVICE_OBJECT	pNextObj;
	KdPrint(("Enter DriverUnload\n"));
	pNextObj = pDriverObject->DeviceObject;
	while (pNextObj != NULL) 
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
			pNextObj->DeviceExtension;

		//ɾ����������
		UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
		IoDeleteSymbolicLink(&pLinkName);
		pNextObj = pNextObj->NextDevice;
		IoDeleteDevice( pDevExt->pDevice );
	}
}

/************************************************************************
* ��������:HelloDDKDispatchRoutine
* ��������:�Զ�IRP���д���
* �����б�:
      pDevObj:�����豸����
      pIrp:��IO�����
* ���� ֵ:����״̬
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("Enter HelloDDKDispatchRoutine\n"));
	NTSTATUS status = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("Leave HelloDDKDispatchRoutine\n"));
	return status;
}