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
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloDDWrite;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKRead;
	pDriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = HelloDDKQueryInfomation;
	
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

	//����ģ���ļ��Ļ�����
	pDevExt->buffer = (PUCHAR)ExAllocatePool(PagedPool,MAX_FILE_LENGTH);
	//����ģ���ļ���С
	pDevExt->file_length = 0;

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
		if (pDevExt->buffer)
		{
			ExFreePool(pDevExt->buffer);
			pDevExt->buffer = NULL;
		}

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

NTSTATUS HelloDDKRead(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("Enter HelloDDKRead\n"));

	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulReadLength = stack->Parameters.Read.Length;
	ULONG ulReadOffset = (ULONG)stack->Parameters.Read.ByteOffset.QuadPart;

	if (ulReadOffset+ulReadLength>MAX_FILE_LENGTH)
	{
		status = STATUS_FILE_INVALID;
		ulReadLength = 0;
	}else
	{
		//�����ݴ洢��AssociatedIrp.SystemBuffer���Ա�Ӧ�ó���ʹ��
		memcpy(pIrp->AssociatedIrp.SystemBuffer,pDevExt->buffer+ulReadOffset,ulReadLength);
		status = STATUS_SUCCESS;
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulReadLength;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("Leave HelloDDKRead\n"));

	return status;
}

NTSTATUS HelloDDWrite(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("Enter HelloDDWrite\n"));

	NTSTATUS status = STATUS_SUCCESS;

	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//��ȡ�洢�ĳ���
	ULONG ulWriteLength = stack->Parameters.Write.Length;
	//��ȡ�洢��ƫ����
	ULONG ulWriteOffset = (ULONG)stack->Parameters.Write.ByteOffset.QuadPart;
	
	if (ulWriteOffset+ulWriteLength>MAX_FILE_LENGTH)
	{
		//����洢����+ƫ�������ڻ��������ȣ��򷵻���Ч
		status = STATUS_FILE_INVALID;
		ulWriteLength = 0;
	}else
	{
		//��д������ݣ��洢�ڻ�������
		memcpy(pDevExt->buffer+ulWriteOffset,pIrp->AssociatedIrp.SystemBuffer,ulWriteLength);
		status = STATUS_SUCCESS;
		//�����µ��ļ�����
		if (ulWriteLength+ulWriteOffset>pDevExt->file_length)
		{
			pDevExt->file_length = ulWriteLength+ulWriteOffset;
		}
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulWriteLength;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("Leave HelloDDWrite\n"));
	return status;
}



NTSTATUS HelloDDKQueryInfomation(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("Enter HelloDDKQueryInfomation\n"));

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);

	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

	FILE_INFORMATION_CLASS info = stack->Parameters.QueryFile.FileInformationClass;
	if (info==FileStandardInformation )
	{
		KdPrint(("FileStandardInformation\n"));
		PFILE_STANDARD_INFORMATION file_info  = 
			(PFILE_STANDARD_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
		file_info->EndOfFile = RtlConvertLongToLargeInteger(pDevExt->file_length);
	}

	NTSTATUS status = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = stack->Parameters.QueryFile.Length;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	KdPrint(("Leave HelloDDKQueryInfomation\n"));

	return status;
}