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
	NTSTATUS ntStatus;
	KdPrint(("DriverB:Enter B DriverEntry\n"));

	//ע�������������ú������
	pDriverObject->DriverUnload = HelloDDKUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKClose;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKRead;
	
	UNICODE_STRING DeviceName;
	RtlInitUnicodeString( &DeviceName, L"\\Device\\MyDDKDeviceA" );

	PDEVICE_OBJECT DeviceObject = NULL;
	PFILE_OBJECT FileObject = NULL;
	//Ѱ��DriverA�������豸����
	ntStatus = IoGetDeviceObjectPointer(&DeviceName,FILE_ALL_ACCESS,&FileObject,&DeviceObject);

	if (!NT_SUCCESS(ntStatus))
	{
		KdPrint(("DriverB:IoGetDeviceObjectPointer() 0x%x\n", ntStatus ));
		return ntStatus;
	}

	//�����Լ��������豸����
	ntStatus = CreateDevice(pDriverObject);

	if ( !NT_SUCCESS( ntStatus ) )
	{
		ObDereferenceObject( FileObject );
		DbgPrint( "IoCreateDevice() 0x%x!\n", ntStatus );
		return ntStatus;
	}

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) pDriverObject->DeviceObject->DeviceExtension;

	PDEVICE_OBJECT FilterDeviceObject = pdx->pDevice;

	//���Լ����豸���������DriverA���豸������
	PDEVICE_OBJECT TargetDevice = IoAttachDeviceToDeviceStack( FilterDeviceObject,
										  DeviceObject );
	//���ײ��豸�����¼����
	pdx->TargetDevice = TargetDevice;
	
	if ( !TargetDevice )
	{
		ObDereferenceObject( FileObject );
		IoDeleteDevice( FilterDeviceObject );
		DbgPrint( "IoAttachDeviceToDeviceStack() 0x%x!\n", ntStatus );
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	FilterDeviceObject->DeviceType = TargetDevice->DeviceType;
	FilterDeviceObject->Characteristics = TargetDevice->Characteristics;
	FilterDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	FilterDeviceObject->Flags |= ( TargetDevice->Flags & ( DO_DIRECT_IO |
														 DO_BUFFERED_IO ) );
	ObDereferenceObject( FileObject );

	KdPrint(("DriverB:B attached A successfully!\n"));
	
	KdPrint(("DriverB:Leave B DriverEntry\n"));
	return ntStatus;
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
	NTSTATUS ntStatus;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
	
	//�����豸����
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName,L"\\Device\\MyDDKDeviceB");
	
	//�����豸
	ntStatus = IoCreateDevice( pDriverObject,
						sizeof(DEVICE_EXTENSION),
						&(UNICODE_STRING)devName,
						FILE_DEVICE_UNKNOWN,
						0, TRUE,
						&pDevObj );
	if (!NT_SUCCESS(ntStatus))
		return ntStatus;

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDevice = pDevObj;
	pDevExt->ustrDeviceName = devName;
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
	KdPrint(("DriverB:Enter B DriverUnload\n"));
	pNextObj = pDriverObject->DeviceObject;

	while (pNextObj != NULL) 
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
			pNextObj->DeviceExtension;
		pNextObj = pNextObj->NextDevice;
		//���豸ջ�е���
		IoDetachDevice( pDevExt->TargetDevice);
		//ɾ�����豸����
		IoDeleteDevice( pDevExt->pDevice );
	}
	KdPrint(("DriverB:Enter B DriverUnload\n"));
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
	KdPrint(("DriverB:Enter B HelloDDKDispatchRoutine\n"));
	NTSTATUS ntStatus = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = ntStatus;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("DriverB:Leave B HelloDDKDispatchRoutine\n"));
	return ntStatus;
}

#pragma PAGEDCODE
NTSTATUS HelloDDKCreate(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("DriverB:Enter B HelloDDKCreate\n"));
	NTSTATUS ntStatus = STATUS_SUCCESS;
	//
// 	// ���IRP
// 	pIrp->IoStatus.Status = ntStatus;
// 	pIrp->IoStatus.Information = 0;	// bytes xfered
// 	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

    IoSkipCurrentIrpStackLocation (pIrp);

    ntStatus = IoCallDriver(pdx->TargetDevice, pIrp);

	KdPrint(("DriverB:Leave B HelloDDKCreate\n"));

	return ntStatus;
}

NTSTATUS
  MyIoCompletion(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP  Irp,
    IN PVOID  Context
    )
{
	//����˺�����־�ײ������豸��IRP���
	KdPrint(("Enter MyIoCompletion\n"));
    if (Irp->PendingReturned) 
	{
		//����pendingλ
        IoMarkIrpPending( Irp );
    }
	return STATUS_SUCCESS;//ͬSTATUS_CONTINUE_COMPLETION
}

#pragma PAGEDCODE
NTSTATUS HelloDDKRead(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("DriverB:Enter B HelloDDKRead\n"));
	NTSTATUS ntStatus = STATUS_SUCCESS;
	//���Լ����IRP���ĳ��ɵײ���������

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

	//����ǰIRP��ջ�����ײ��ջ
	IoCopyCurrentIrpStackLocationToNext(pIrp);

	//�����������
	IoSetCompletionRoutine(pIrp,MyIoCompletion,NULL,TRUE,TRUE,TRUE);

	//���õײ�����
    ntStatus = IoCallDriver(pdx->TargetDevice, pIrp);

	//��IoCallDriver�󣬲���������̷��ص���STATUS_SUCCESS
	//IRP�Ͳ���������ǲ�����ˣ��Ͳ��ܶ�IRP���в�����
	if (ntStatus == STATUS_PENDING)
	{
		KdPrint(("STATUS_PENDING\n"));
	}
	ntStatus = STATUS_PENDING; 

	KdPrint(("DriverB:Leave B HelloDDKRead\n"));

	return ntStatus;
}

#pragma PAGEDCODE
NTSTATUS HelloDDKClose(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("DriverB:Enter B HelloDDKClose\n"));
	NTSTATUS ntStatus = STATUS_SUCCESS;

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

    IoSkipCurrentIrpStackLocation (pIrp);

    ntStatus = IoCallDriver(pdx->TargetDevice, pIrp);	
	
	KdPrint(("DriverB:Leave B HelloDDKClose\n"));

	return ntStatus;
}