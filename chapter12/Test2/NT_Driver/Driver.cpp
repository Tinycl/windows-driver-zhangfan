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

	NTSTATUS status = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	KdPrint(("Leave HelloDDKDispatchRoutin\n"));

	return status;
}

VOID
GetDeviceObjectInfo( PDEVICE_OBJECT DevObj )
{
  POBJECT_HEADER ObjectHeader;
  POBJECT_HEADER_NAME_INFO ObjectNameInfo; 

  if ( DevObj == NULL )
  {
    DbgPrint( "DevObj is NULL!\n" );
    return;
  }
  // �õ�����ͷ
  ObjectHeader = OBJECT_TO_OBJECT_HEADER( DevObj );

  if ( ObjectHeader )
  {
    // ��ѯ�豸���Ʋ���ӡ
    ObjectNameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

    if ( ObjectNameInfo && ObjectNameInfo->Name.Buffer )
    {
      DbgPrint( "Driver Name:%wZ - Device Name:%wZ - Driver Address:0x%x - Device Address:0x%x\n",
                &DevObj->DriverObject->DriverName,
                &ObjectNameInfo->Name,
                DevObj->DriverObject,
                DevObj );
    }

    // ����û�����Ƶ��豸�����ӡ NULL
    else if ( DevObj->DriverObject )
    {
      DbgPrint( "Driver Name:%wZ - Device Name:%S - Driver Address:0x%x - Device Address:0x%x\n",
                &DevObj->DriverObject->DriverName,
                L"NULL",
                DevObj->DriverObject,
                DevObj );
    }
  }
}

VOID
GetAttachedDeviceInfo( PDEVICE_OBJECT DevObj )
{
  PDEVICE_OBJECT DeviceObject;

  if ( DevObj == NULL )
  {
    DbgPrint( "DevObj is NULL!\n" );
    return;
  }

  DeviceObject = DevObj->AttachedDevice;

  while ( DeviceObject )
  {
    DbgPrint( "Attached Driver Name:%wZ,Attached Driver Address:0x%x,Attached DeviceAddress:0x%x\n",
              &DeviceObject->DriverObject->DriverName,
              DeviceObject->DriverObject,
              DeviceObject );

    DeviceObject = DeviceObject->AttachedDevice;
  }
}

PDRIVER_OBJECT
EnumDeviceStack( PWSTR pwszDeviceName )
{
  UNICODE_STRING DriverName;
  PDRIVER_OBJECT DriverObject = NULL;
  PDEVICE_OBJECT DeviceObject = NULL;


  RtlInitUnicodeString( &DriverName, pwszDeviceName );


  ObReferenceObjectByName( &DriverName,
                           OBJ_CASE_INSENSITIVE,
                           NULL,
                           0,
                           ( POBJECT_TYPE ) IoDriverObjectType,
                           KernelMode,
                           NULL,
                           (PVOID*)&DriverObject );

  if ( DriverObject == NULL )
  {
    return NULL;
  }

  DeviceObject = DriverObject->DeviceObject;

  while ( DeviceObject )
  {
    GetDeviceObjectInfo( DeviceObject );

    // �жϵ�ǰ�豸���Ƿ��й���������Filter Driver��
    if ( DeviceObject->AttachedDevice )
    {
      GetAttachedDeviceInfo( DeviceObject );
    }

    // ��һ���жϵ�ǰ�豸�� VPB �е��豸
    if ( DeviceObject->Vpb && DeviceObject->Vpb->DeviceObject )
    {
      GetDeviceObjectInfo( DeviceObject->Vpb->DeviceObject );

      if ( DeviceObject->Vpb->DeviceObject->AttachedDevice )
      {
        GetAttachedDeviceInfo( DeviceObject->Vpb->DeviceObject );
      }
    }

    // �õ������ڴ������ϵ���һ���豸 DEVICE_OBJECT 
    DeviceObject = DeviceObject->NextDevice;
  }

  return DriverObject;
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

	ULONG info = 0;

	switch (code)
	{						// process request
		case IOCTL_DUMP_DEVICE_STACK:
		{
			KdPrint(("IOCTL_DUMP_DEVICE_STACK\n"));
			//��������ʽIOCTL
			//��ʾ���뻺��������
 			WCHAR* InputBuffer = (WCHAR*)pIrp->AssociatedIrp.SystemBuffer;
			KdPrint(("%ws\n",InputBuffer));

			EnumDeviceStack( InputBuffer );

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