/************************************************************************
* �ļ�����:Driver.cpp                                                 
* ��    ��:�ŷ�
* �������:2007-11-1
*************************************************************************/

#include "Driver.h"


#pragma LOCKEDCODE
VOID
  MyStartIo(
    IN PDEVICE_OBJECT  DeviceObject,
	IN PIRP pFistIrp
    )
{
	KdPrint(("Enter MyStartIo\n"));

	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
		DeviceObject->DeviceExtension;

	PKDEVICE_QUEUE_ENTRY device_entry;

	PIRP Irp = pFistIrp;
	do
	{
		KEVENT event;
		KeInitializeEvent(&event,NotificationEvent,FALSE);

		//��3��
		LARGE_INTEGER timeout;
		timeout.QuadPart = -3*1000*1000*10;

		//����һ��3�����ʱ����Ҫ��Ϊ��ģ���IRP������Ҫ���3������ʱ��
		KeWaitForSingleObject(&event,Executive,KernelMode,FALSE,&timeout);

		KdPrint(("Complete a irp:%x\n",Irp));
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;	// no bytes xfered
		IoCompleteRequest(Irp,IO_NO_INCREMENT);

		device_entry=KeRemoveDeviceQueue(&pDevExt->device_queue);
		KdPrint(("device_entry:%x\n",device_entry));
		if (device_entry==NULL)
		{
			break;
		}

		Irp = CONTAINING_RECORD(device_entry, IRP, Tail.Overlay.DeviceQueueEntry);
	}while(1);

	KdPrint(("Leave MyStartIo\n"));
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

	//����ж�غ���
	pDriverObject->DriverUnload = HelloDDKUnload;

	//������ǲ����
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKRead;
	pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = HelloDDKDispatchRoutin;
	pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = HelloDDKDispatchRoutin;

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

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDevice = pDevObj;
	pDevExt->ustrDeviceName = devName;

	RtlZeroBytes(&pDevExt->device_queue,sizeof(pDevExt->device_queue));
	KeInitializeDeviceQueue(&pDevExt->device_queue);

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

	//��һ��IRP�ļ򵥲������������ܶ�IRP�����ӵĲ���
	NTSTATUS status = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	KdPrint(("Leave HelloDDKDispatchRoutin\n"));

	return status;
}

VOID
OnCancelIRP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
	KdPrint(("Enter CancelReadIRP\n"));

	//�ͷ�Cancel������
	IoReleaseCancelSpinLock(Irp->CancelIrql);
	
	//�������״̬ΪSTATUS_CANCELLED
 	Irp->IoStatus.Status = STATUS_CANCELLED;
 	Irp->IoStatus.Information = 0;	// bytes xfered
 	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	KdPrint(("Leave CancelReadIRP\n"));
}

NTSTATUS HelloDDKRead(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("Enter HelloDDKRead\n"));

	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
			pDevObj->DeviceExtension;

	//��IRP����Ϊ����
	IoMarkIrpPending(pIrp);

	IoSetCancelRoutine(pIrp,OnCancelIRP);
	
	KIRQL oldirql;
	//����IRP��DISPATCH_LEVEL
	KeRaiseIrql(DISPATCH_LEVEL, &oldirql);

	KdPrint(("HelloDDKRead irp :%x\n",pIrp));

	KdPrint(("DeviceQueueEntry:%x\n",&pIrp->Tail.Overlay.DeviceQueueEntry));
	if (!KeInsertDeviceQueue(&pDevExt->device_queue, &pIrp->Tail.Overlay.DeviceQueueEntry))
		MyStartIo(pDevObj,pIrp);

	//��IRP����ԭ��IRQL
	KeLowerIrql(oldirql);

	KdPrint(("Leave HelloDDKRead\n"));

	//����pending״̬
	return STATUS_PENDING;
}

