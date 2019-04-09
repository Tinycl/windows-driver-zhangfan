/************************************************************************
* �ļ�����:Driver.cpp                                                 
* ��    ��:�ŷ�
* �������:2007-11-1
*************************************************************************/

#include "Driver.h"

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

	//����ж�غ���
	pDriverObject->DriverUnload = HelloDDKUnload;

	//������ǲ����
	for (int i = 0; i < arraysize(pDriverObject->MajorFunction); ++i)
		pDriverObject->MajorFunction[i] = HelloDDKDispatchRoutin;

	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloDDKDeviceIOControl;
	
	//���������豸����
	status = CreateDevice(pDriverObject);

	KdPrint(("Leave DriverEntry\n"));
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

	pDevObj->Flags |= DO_DIRECT_IO;
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
* ��������:HelloDDKDispatchRoutin
* ��������:�Զ�IRP���д���
* �����б�:
      pDevObj:�����豸����
      pIrp:��IO�����
* ���� ֵ:����״̬
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HelloDDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("Enter HelloDDKDispatchRoutin\n"));

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//����һ���ַ���������IRP���Ͷ�Ӧ����
	static char* irpname[] = 
	{
		"IRP_MJ_CREATE",
		"IRP_MJ_CREATE_NAMED_PIPE",
		"IRP_MJ_CLOSE",
		"IRP_MJ_READ",
		"IRP_MJ_WRITE",
		"IRP_MJ_QUERY_INFORMATION",
		"IRP_MJ_SET_INFORMATION",
		"IRP_MJ_QUERY_EA",
		"IRP_MJ_SET_EA",
		"IRP_MJ_FLUSH_BUFFERS",
		"IRP_MJ_QUERY_VOLUME_INFORMATION",
		"IRP_MJ_SET_VOLUME_INFORMATION",
		"IRP_MJ_DIRECTORY_CONTROL",
		"IRP_MJ_FILE_SYSTEM_CONTROL",
		"IRP_MJ_DEVICE_CONTROL",
		"IRP_MJ_INTERNAL_DEVICE_CONTROL",
		"IRP_MJ_SHUTDOWN",
		"IRP_MJ_LOCK_CONTROL",
		"IRP_MJ_CLEANUP",
		"IRP_MJ_CREATE_MAILSLOT",
		"IRP_MJ_QUERY_SECURITY",
		"IRP_MJ_SET_SECURITY",
		"IRP_MJ_POWER",
		"IRP_MJ_SYSTEM_CONTROL",
		"IRP_MJ_DEVICE_CHANGE",
		"IRP_MJ_QUERY_QUOTA",
		"IRP_MJ_SET_QUOTA",
		"IRP_MJ_PNP",
	};

	UCHAR type = stack->MajorFunction;
	if (type >= arraysize(irpname))
		KdPrint((" - Unknown IRP, major type %X\n", type));
	else
		KdPrint(("\t%s\n", irpname[type]));

	NTSTATUS status = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	KdPrint(("Leave HelloDDKDispatchRoutin\n"));

	return status;
}

#pragma PAGEDCODE
VOID WaitMicroSecond1(ULONG ulMircoSecond)
{
	KEVENT kEvent;

	KdPrint(("Thread suspends %d MircoSeconds...",ulMircoSecond));

	//��ʼ��һ��δ�������ں��¼�
	KeInitializeEvent(&kEvent,SynchronizationEvent,FALSE);

	//�ȴ�ʱ��ĵ�λ��100���룬��΢��ת���������λ
	//���������ǴӴ˿̵�δ����ĳ��ʱ��
	LARGE_INTEGER timeout = RtlConvertLongToLargeInteger(-10*ulMircoSecond);

	//�ھ���timeout���̼߳�������
	KeWaitForSingleObject(&kEvent,
		Executive,
		KernelMode,
		FALSE,
		&timeout);

	KdPrint(("Thread is running again!\n"));
}

#pragma PAGEDCODE
VOID WaitMicroSecond2(ULONG ulMircoSecond)
{
	KdPrint(("Thread suspends %d MircoSeconds...",ulMircoSecond));

	//�ȴ�ʱ��ĵ�λ��100���룬��΢��ת���������λ
	//���������ǴӴ˿̵�δ����ĳ��ʱ��
	LARGE_INTEGER timeout = RtlConvertLongToLargeInteger(-10*ulMircoSecond);

	//���ַ���������KeWaitForSingleObject
	//����ǰ�߳̽���˯��״̬�����ʱ�䵽ת������״̬
	KeDelayExecutionThread(KernelMode,FALSE,&timeout);

	KdPrint(("Thread is running again!\n"));
}

#pragma PAGEDCODE
VOID WaitMicroSecond3(ULONG ulMircoSecond)
{
	KdPrint(("Thread suspends %d MircoSeconds...",ulMircoSecond));

	//æ�ȴ������ַ�������æ�ȴ����Ƚ��˷�CPUʱ��
	//���ʹ�ø÷������˳���50΢��
	KeStallExecutionProcessor(ulMircoSecond);

	KdPrint(("Thread is running again!\n"));
}

#pragma PAGEDCODE
VOID WaitMicroSecond4(ULONG ulMircoSecond)
{
	//ʹ�ü�ʱ��

	KTIMER kTimer;//�ں˼�ʱ��

	//��ʼ����ʱ��
	KeInitializeTimer(&kTimer);

	LARGE_INTEGER timeout = RtlConvertLongToLargeInteger( ulMircoSecond * -10 );

	//ע�������ʱ��û�к�DPC�������
	KeSetTimer(&kTimer,timeout,	NULL);
	KdPrint(("Thread suspends %d MircoSeconds...",ulMircoSecond));

	KeWaitForSingleObject(&kTimer,Executive,KernelMode,FALSE,NULL);

	KdPrint(("Thread is running again!\n"));
}

#pragma PAGEDCODE
NTSTATUS HelloDDKDeviceIOControl(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	KdPrint(("Enter HelloDDKDeviceIOControl\n"));

	//�õ���ǰ��ջ
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//�õ����뻺������С
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	//�õ������������С
	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
	//�õ�IOCTL��
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
		pDevObj->DeviceExtension;

	ULONG info = 0;

	//�õ��û����򴫽�����΢����
	ULONG ulMircoSecond = *(PULONG)pIrp->AssociatedIrp.SystemBuffer;

	switch (code)
	{						// process request
		case IOCTL_WAIT_METHOD1:
		{
			KdPrint(("IOCTL_WAIT_METHOD1\n"));
			WaitMicroSecond1(ulMircoSecond);
			break;
		}
		case IOCTL_WAIT_METHOD2:
		{
			KdPrint(("IOCTL_WAIT_METHOD2\n"));
			WaitMicroSecond2(ulMircoSecond);
			break;
		}
		case IOCTL_WAIT_METHOD3:
		{
			KdPrint(("IOCTL_WAIT_METHOD3\n"));
			WaitMicroSecond3(ulMircoSecond);
			break;
		}
		case IOCTL_WAIT_METHOD4:
		{
			KdPrint(("IOCTL_WAIT_METHOD4\n"));
			WaitMicroSecond4(ulMircoSecond);
			break;
		}
		default:
			status = STATUS_INVALID_VARIANT;
	}

	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = info;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	KdPrint(("Leave HelloDDKDeviceIOControl\n"));

	return status;
}