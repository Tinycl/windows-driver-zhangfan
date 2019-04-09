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
	
	//���������豸����
	ntStatus = CreateDevice(pDriverObject);

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
	RtlInitUnicodeString(&devName,L"\\Device\\MyDDKDevicB");
	
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

	//������������
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName,L"\\??\\HelloDDKB");
	pDevExt->ustrSymLinkName = symLinkName;
	NTSTATUS status = IoCreateSymbolicLink( &symLinkName,&devName );
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
	KdPrint(("DriverB:Enter B DriverUnload\n"));
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
	KdPrint(("DriverB:Enter B DriverUnload\n"));
}

#pragma PAGEDCODE
NTSTATUS HelloDDKRead(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("DriverB:Enter B HelloDDKRead\n"));
	NTSTATUS ntStatus = STATUS_SUCCESS;

	UNICODE_STRING DeviceName;
	RtlInitUnicodeString( &DeviceName, L"\\Device\\MyDDKDeviceA" );

	PDEVICE_OBJECT DeviceObject = NULL;
	PFILE_OBJECT FileObject = NULL;
	//�õ��豸����ָ��
	ntStatus = IoGetDeviceObjectPointer(&DeviceName,FILE_ALL_ACCESS,&FileObject,&DeviceObject);

	KdPrint(("DriverB:FileObject:%x\n",FileObject));
	KdPrint(("DriverB:DeviceObject:%x\n",DeviceObject));

	if (!NT_SUCCESS(ntStatus))
	{
		KdPrint(("DriverB:IoGetDeviceObjectPointer() 0x%x\n", ntStatus ));
		ntStatus = STATUS_UNSUCCESSFUL;
		// ���IRP
		pIrp->IoStatus.Status = ntStatus;
		pIrp->IoStatus.Information = 0;	// bytes xfered
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		KdPrint(("DriverB:Leave B HelloDDKRead\n"));

		return ntStatus;
	}

	KEVENT event;
	KeInitializeEvent(&event,NotificationEvent,FALSE);

	PIRP pNewIrp = IoAllocateIrp(DeviceObject->StackSize,FALSE);
	KdPrint(("pNewIrp->UserEvent :%x\n",pNewIrp->UserEvent));
	pNewIrp->UserEvent = &event;

	IO_STATUS_BLOCK status_block;
    pNewIrp->UserIosb = &status_block;
    pNewIrp->Tail.Overlay.Thread = PsGetCurrentThread();

	//��ΪDriverA��BUFFER IO�豸
	pNewIrp->AssociatedIrp.SystemBuffer = NULL;
	
 	KdPrint(("DriverB:pNewIrp:%x\n",pNewIrp));

	PIO_STACK_LOCATION stack = IoGetNextIrpStackLocation(pNewIrp);
	stack->MajorFunction = IRP_MJ_READ;
	stack->MinorFunction=IRP_MN_NORMAL;//0
	stack->FileObject = FileObject;

	//����DriverA����
	NTSTATUS status = IoCallDriver(DeviceObject,pNewIrp);

    if (status == STATUS_PENDING) {
       status = KeWaitForSingleObject(
                            &event,
                            Executive,
                            KernelMode,
                            FALSE, // Not alertable
							NULL);
	   KdPrint(("STATUS_PENDING\n"));
    }

 	ObDereferenceObject( FileObject );
	IoFreeIrp(pNewIrp);

	ntStatus = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = ntStatus;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("DriverB:Leave B HelloDDKRead\n"));
	return ntStatus;
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
	// ���IRP
	pIrp->IoStatus.Status = ntStatus;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	KdPrint(("DriverB:Leave B HelloDDKCreate\n"));

	return ntStatus;
}

#pragma PAGEDCODE
NTSTATUS HelloDDKClose(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("DriverB:Enter B HelloDDKClose\n"));
	NTSTATUS ntStatus = STATUS_SUCCESS;

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

	// ���IRP
	pIrp->IoStatus.Status = ntStatus;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	
	KdPrint(("DriverB:Leave B HelloDDKClose\n"));

	return ntStatus;
}