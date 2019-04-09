/************************************************************************
* �ļ�����:Driver.cpp                                                 
* ��    ��:�ŷ�
* �������:2007-11-1
*************************************************************************/

#include "Driver.h"

#pragma INITCODE
VOID StringInitTest() 
{
	//(1)��RtlInitAnsiString��ʼ���ַ���
	ANSI_STRING  AnsiString1;
	CHAR * string1= "hello";
	//��ʼ��ANSI_STRING�ַ���
	RtlInitAnsiString(&AnsiString1,string1);
	KdPrint(("AnsiString1:%Z\n",&AnsiString1));//��ӡhello

	string1[0]='H';
	string1[1]='E';
	string1[2]='L';
	string1[3]='L';
	string1[4]='O';
	//�ı�string1��AnsiString1ͬ���ᵼ�±仯
	KdPrint(("AnsiString1:%Z\n",&AnsiString1));//��ӡHELLO

	//(2)����Ա�Լ���ʼ���ַ���
#define BUFFER_SIZE 1024
	UNICODE_STRING UnicodeString1 = {0};
	//���û�������С
	UnicodeString1.MaximumLength = BUFFER_SIZE;
	//�����ڴ�
	UnicodeString1.Buffer = (PWSTR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	WCHAR* wideString = L"hello";

	//�����ַ�����,��Ϊ�ǿ��ַ����������ַ����ȵ�2��
	UnicodeString1.Length = 2*wcslen(wideString);

	//��֤�������㹻�󣬷��������ֹ
	ASSERT(UnicodeString1.MaximumLength>=UnicodeString1.Length);
	//�ڴ濽����
	RtlCopyMemory(UnicodeString1.Buffer,wideString,UnicodeString1.Length);
	//�����ַ�����
	UnicodeString1.Length = 2*wcslen(wideString);

	KdPrint(("UnicodeString:%wZ\n",&UnicodeString1));

	//�����ڴ�
	ExFreePool(UnicodeString1.Buffer);
	UnicodeString1.Buffer = NULL;
	UnicodeString1.Length = UnicodeString1.MaximumLength = 0;
}

#pragma INITCODE
VOID StringCopyTest() 
{
	//��ʼ��UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"Hello World");

	//��ʼ��UnicodeString2
	UNICODE_STRING UnicodeString2={0};
	UnicodeString2.Buffer = (PWSTR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	UnicodeString2.MaximumLength = BUFFER_SIZE;

	//����ʼ��UnicodeString2������UnicodeString1
	RtlCopyUnicodeString(&UnicodeString2,&UnicodeString1);

	//�ֱ���ʾUnicodeString1��UnicodeString2
	KdPrint(("UnicodeString1:%wZ\n",&UnicodeString1));
	KdPrint(("UnicodeString2:%wZ\n",&UnicodeString2));

	//����UnicodeString2
	//ע��!!UnicodeString1��������
	RtlFreeUnicodeString(&UnicodeString2);
		
}

#pragma INITCODE
VOID StringCompareTest() 
{
	//��ʼ��UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"Hello World");

	//��ʼ��UnicodeString2
	UNICODE_STRING UnicodeString2;
	RtlInitUnicodeString(&UnicodeString1,L"Hello");

	if (RtlEqualUnicodeString(&UnicodeString1,&UnicodeString2,TRUE))
	{
		KdPrint(("UnicodeString1 and UnicodeString2 are equal\n"));
	}else
	{
		KdPrint(("UnicodeString1 and UnicodeString2 are NOT equal\n"));
	}

}

#pragma INITCODE
VOID StringToUpperTest() 
{
	//��ʼ��UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"Hello World");

	//�仯ǰ
	KdPrint(("UnicodeString1:%wZ\n",&UnicodeString1));

	//���д
	RtlUpcaseUnicodeString(&UnicodeString1,&UnicodeString1,FALSE);

	//�仯��
	KdPrint(("UnicodeString1:%wZ\n",&UnicodeString1));
}

#pragma INITCODE
VOID StringToIntegerTest() 
{
	//(1)�ַ���ת��������
	//��ʼ��UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"-100");

	ULONG lNumber;
	NTSTATUS nStatus = RtlUnicodeStringToInteger(&UnicodeString1,10,&lNumber);
	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("Conver to integer succussfully!\n"));
		KdPrint(("Result:%d\n",lNumber));
	}else
	{
		KdPrint(("Conver to integer unsuccessfully!\n"));
	}

	//(2)����ת�����ַ���
	//��ʼ��UnicodeString2
	UNICODE_STRING UnicodeString2={0};
	UnicodeString2.Buffer = (PWSTR)ExAllocatePool(PagedPool,BUFFER_SIZE);
	UnicodeString2.MaximumLength = BUFFER_SIZE;
	nStatus = RtlIntegerToUnicodeString(200,10,&UnicodeString2);

	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("Conver to string succussfully!\n"));
		KdPrint(("Result:%wZ\n",&UnicodeString2));
	}else
	{
		KdPrint(("Conver to string unsuccessfully!\n"));
	}

	//����UnicodeString2
	//ע��!!UnicodeString1��������
	RtlFreeUnicodeString(&UnicodeString2);

}

#pragma INITCODE
VOID StringConverTest() 
{
	//(1)��UNICODE_STRING�ַ���ת����ANSI_STRING�ַ���
	//��ʼ��UnicodeString1
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1,L"Hello World");

	ANSI_STRING AnsiString1;
	NTSTATUS nStatus = RtlUnicodeStringToAnsiString(&AnsiString1,&UnicodeString1,TRUE);
	
	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("Conver succussfully!\n"));
		KdPrint(("Result:%Z\n",&AnsiString1));
	}else
	{
		KdPrint(("Conver unsuccessfully!\n"));
	}

 	//����AnsiString1
	RtlFreeAnsiString(&AnsiString1);

	//(2)��ANSI_STRING�ַ���ת����UNICODE_STRING�ַ���
	//��ʼ��AnsiString2
	ANSI_STRING AnsiString2;
	RtlInitString(&AnsiString2,"Hello World");

	UNICODE_STRING UnicodeString2;
	nStatus = RtlAnsiStringToUnicodeString(&UnicodeString2,&AnsiString2,TRUE);
	
	if ( NT_SUCCESS(nStatus))
	{
		KdPrint(("Conver succussfully!\n"));
		KdPrint(("Result:%wZ\n",&UnicodeString2));
	}else
	{
		KdPrint(("Conver unsuccessfully!\n"));
	}

 	//����UnicodeString2
	RtlFreeUnicodeString(&UnicodeString2);
}


VOID StringTest()
{
	//�ַ�����ʼ��ʵ��
//	StringInitTest();

	//�ַ�������ʵ��
//	StringCopyTest();

	//�ַ����Ƚ�ʵ��
//	StringCompareTest();

	//�ַ������дʵ��
//	StringToUpperTest();

	//�ַ����������໥ת��ʵ��
//	StringToIntegerTest();

	//ANSI_STRING�ַ�����UNICODE_STRING�ַ����໥ת��ʵ��
	StringConverTest();

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

	StringTest();

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