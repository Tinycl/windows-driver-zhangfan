/************************************************************************
* �ļ�����:HelloWDM.cpp                                                 
* ��    ��:�ŷ�
* �������:2007-11-1
*************************************************************************/
#include "HelloWDM.h"

#include <initguid.h>
#include "guid.h"
#include "Ioctls.h"

/************************************************************************
* ��������:DriverEntry
* ��������:��ʼ���������򣬶�λ������Ӳ����Դ�������ں˶���
* �����б�:
      pDriverObject:��I/O�������д���������������
      pRegistryPath:����������ע�����е�·��
* ���� ֵ:���س�ʼ������״̬
*************************************************************************/
#pragma INITCODE 
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,
								IN PUNICODE_STRING pRegistryPath)
{
	KdPrint(("Enter DriverEntry\n"));

	pDriverObject->DriverExtension->AddDevice = HelloWDMAddDevice;
	pDriverObject->MajorFunction[IRP_MJ_PNP] = HelloWDMPnp;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = 
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = 
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = 
	pDriverObject->MajorFunction[IRP_MJ_READ] = 
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloWDMDispatchRoutine;
	pDriverObject->DriverUnload = HelloWDMUnload;

	KdPrint(("Leave DriverEntry\n"));
	return STATUS_SUCCESS;
}

/************************************************************************
* ��������:HelloWDMAddDevice
* ��������:������豸
* �����б�:
      DriverObject:��I/O�������д���������������
      PhysicalDeviceObject:��I/O�������д������������豸����
* ���� ֵ:����������豸״̬
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HelloWDMAddDevice(IN PDRIVER_OBJECT DriverObject,
                           IN PDEVICE_OBJECT PhysicalDeviceObject)
{ 
	PAGED_CODE();
	KdPrint(("Enter HelloWDMAddDevice\n"));

	NTSTATUS status;
	PDEVICE_OBJECT fdo;
	status = IoCreateDevice(
		DriverObject,
		sizeof(DEVICE_EXTENSION),
		NULL,//û��ָ���豸��
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&fdo);
	if( !NT_SUCCESS(status))
		return status;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
	pdx->fdo = fdo;
	pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);

	//�����豸�ӿ�
	status = IoRegisterDeviceInterface(PhysicalDeviceObject, &MY_WDM_DEVICE, NULL, &pdx->interfaceName);
	if( !NT_SUCCESS(status))
	{
		IoDeleteDevice(fdo);
		return status;
	}
	KdPrint(("%wZ\n",&pdx->interfaceName));
	IoSetDeviceInterfaceState(&pdx->interfaceName, TRUE);

	if( !NT_SUCCESS(status))
	{
		if( !NT_SUCCESS(status))
		{
			return status;
		}
	}

	fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;

	KdPrint(("Leave HelloWDMAddDevice\n"));
	return STATUS_SUCCESS;
}

/************************************************************************
* ��������:DefaultPnpHandler
* ��������:��PNP IRP����ȱʡ����
* �����б�:
      pdx:�豸�������չ
      Irp:��IO�����
* ���� ֵ:����״̬
*************************************************************************/ 
#pragma PAGEDCODE
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter DefaultPnpHandler\n"));
	IoSkipCurrentIrpStackLocation(Irp);
	KdPrint(("Leave DefaultPnpHandler\n"));
	return IoCallDriver(pdx->NextStackDevice, Irp);
}

#pragma LOCKEDCODE
NTSTATUS OnRequestComplete(PDEVICE_OBJECT junk, PIRP Irp, PKEVENT pev)
{							// OnRequestComplete
	//��������������õȴ��¼�
	KeSetEvent(pev, 0, FALSE);
	//��־��IRP����Ҫ�ٴα����
	return STATUS_MORE_PROCESSING_REQUIRED;
}

///////////////////////////////////////////////////////////////////////////////

#pragma PAGEDCODE
NTSTATUS ForwardAndWait(PDEVICE_EXTENSION pdx, PIRP Irp)
{							// ForwardAndWait
	PAGED_CODE();
	
	KEVENT event;
	//��ʼ���¼�
	KeInitializeEvent(&event, NotificationEvent, FALSE);

	//�������ջ��������һ���ջ
	IoCopyCurrentIrpStackLocationToNext(Irp);
	//�����������
	IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE) OnRequestComplete,
		(PVOID) &event, TRUE, TRUE, TRUE);

	//���õײ���������PDO
	IoCallDriver(pdx->NextStackDevice, Irp);
	//�ȴ�PDO���
	KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
	return Irp->IoStatus.Status;
}							// ForwardAndWait


/************************************************************************
* ��������:HandleRemoveDevice
* ��������:��IRP_MN_REMOVE_DEVICE IRP���д���
* �����б�:
      fdo:�����豸����
      Irp:��IO�����
* ���� ֵ:����״̬
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter HandleRemoveDevice\n"));

	Irp->IoStatus.Status = STATUS_SUCCESS;
	NTSTATUS status = DefaultPnpHandler(pdx, Irp);

	IoSetDeviceInterfaceState(&pdx->interfaceName, FALSE);
	RtlFreeUnicodeString(&pdx->interfaceName);

    //����IoDetachDevice()��fdo���豸ջ���ѿ���
    if (pdx->NextStackDevice)
        IoDetachDevice(pdx->NextStackDevice);
	
    //ɾ��fdo��
    IoDeleteDevice(pdx->fdo);

	//ɾ���ж�
	IoDisconnectInterrupt(pdx->InterruptObject);
	KdPrint(("Leave HandleRemoveDevice\n"));
	return status;
}

BOOLEAN OnInterrupt(PKINTERRUPT InterruptObject, PDEVICE_EXTENSION pdx)
{							// OnInterrupt

	//���ж�
	UCHAR HSR  = READ_PORT_UCHAR(pdx->portbase);
	HSR = HSR | 0x4;
	WRITE_PORT_UCHAR(pdx->portbase,HSR);

	KdPrint(("==============interrupt!!!\n"));
	
	//�ָ��ж��źŵ�ƽ
	WRITE_REGISTER_UCHAR((PUCHAR)pdx->MemBar1+0x400000,0x10);

	IoRequestDpc(pdx->fdo, NULL, pdx);

	return TRUE;
}		

#pragma PAGEDCODE
NTSTATUS InitMyPCI(IN PDEVICE_EXTENSION pdx,IN PCM_PARTIAL_RESOURCE_LIST list)
{
	PDEVICE_OBJECT fdo = pdx->fdo;

	ULONG vector;
	KIRQL irql;
	KINTERRUPT_MODE mode;
	KAFFINITY affinity;
	BOOLEAN irqshare;
	BOOLEAN gotinterrupt = FALSE;

	PHYSICAL_ADDRESS portbase;
	BOOLEAN gotport = FALSE;
	 
	PCM_PARTIAL_RESOURCE_DESCRIPTOR resource = &list->PartialDescriptors[0];
	ULONG nres = list->Count;
	BOOLEAN IsMem0 = TRUE;
	for (ULONG i = 0; i < nres; ++i, ++resource)
		{						// for each resource
		switch (resource->Type)
			{					// switch on resource type
		case CmResourceTypePort:
			portbase = resource->u.Port.Start;
			pdx->nports = resource->u.Port.Length;
			pdx->mappedport = (resource->Flags & CM_RESOURCE_PORT_IO) == 0;
			gotport = TRUE;
			break;

		case CmResourceTypeMemory:
			if (IsMem0) 
			{
				pdx->MemBar0 = (PUCHAR)MmMapIoSpace(resource->u.Memory.Start,
					resource->u.Memory.Length,
					MmNonCached);
				pdx->nMem0 = resource->u.Memory.Length;
				IsMem0 = FALSE;
			}else
			{
				pdx->MemBar1 = (PUCHAR)MmMapIoSpace(resource->u.Memory.Start,
					resource->u.Memory.Length,
					MmNonCached);
				pdx->nMem1 = resource->u.Memory.Length;
			}

			break;

		case CmResourceTypeInterrupt:
			irql = (KIRQL) resource->u.Interrupt.Level;
			vector = resource->u.Interrupt.Vector;
			affinity = resource->u.Interrupt.Affinity;
			mode = (resource->Flags == CM_RESOURCE_INTERRUPT_LATCHED)
				? Latched : LevelSensitive;
			irqshare = resource->ShareDisposition == CmResourceShareShared;
			gotinterrupt = TRUE;
			
			break;

		default:
			KdPrint(("Unexpected I/O resource type %d\n", resource->Type));
			break;
			}					// switch on resource type
		}						// for each resource

	if (!(TRUE&& gotport&& gotinterrupt	))
		{
		KdPrint((" Didn't get expected I/O resources\n"));
		return STATUS_DEVICE_CONFIGURATION_ERROR;
		}

	if (pdx->mappedport)
		{						// map port address for RISC platform
		pdx->portbase = (PUCHAR) MmMapIoSpace(portbase, pdx->nports, MmNonCached);
		if (!pdx->mappedport)
			{
			KdPrint(("Unable to map port range %I64X, length %X\n", portbase, pdx->nports));
			return STATUS_INSUFFICIENT_RESOURCES;
			}
		}						// map port address for RISC platform
	else
		pdx->portbase = (PUCHAR) portbase.QuadPart;

	NTSTATUS status = IoConnectInterrupt(&pdx->InterruptObject, (PKSERVICE_ROUTINE) OnInterrupt,
		(PVOID) pdx, NULL, vector, irql, irql, LevelSensitive, TRUE, affinity, FALSE);
	if (!NT_SUCCESS(status))
		{
		KdPrint(("IoConnectInterrupt failed - %X\n", status));
		if (pdx->portbase && pdx->mappedport)
			MmUnmapIoSpace(pdx->portbase, pdx->nports);
		pdx->portbase = NULL;
		return status;
		}

#define IMAGE_LENGTH (640*480)
	//����һ�����������ַ����ȡͼ��
	PHYSICAL_ADDRESS maxAddress;
	maxAddress.u.LowPart = 0xFFFFFFFF;
	maxAddress.u.HighPart = 0;

	pdx->MemForImage = MmAllocateContiguousMemory(IMAGE_LENGTH,maxAddress);

	PHYSICAL_ADDRESS pycialAddressForImage = MmGetPhysicalAddress(pdx->MemForImage);

	WRITE_REGISTER_BUFFER_UCHAR((PUCHAR)pdx->MemBar0+0x10000,
		(PUCHAR)&pycialAddressForImage.u.LowPart,4);

	return STATUS_SUCCESS;	
}

#pragma PAGEDCODE
NTSTATUS HandleStartDevice(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter HandleStartDevice\n"));

	//ת��IRP���ȴ�����
	NTSTATUS status = ForwardAndWait(pdx,Irp);
	if (!NT_SUCCESS(status))
	{
		Irp->IoStatus.Status = status;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return status;
	}

	//�õ���ǰ��ջ
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	//�ӵ�ǰ��ջ�õ�������Ϣ
	PCM_PARTIAL_RESOURCE_LIST translated;
	if (stack->Parameters.StartDevice.AllocatedResourcesTranslated)
		translated = &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList;
	else
		translated = NULL;

	KdPrint(("Init the PCI card!\n"));
	InitMyPCI(pdx,translated);

	//���IRP
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	KdPrint(("Leave HandleStartDevice\n"));
	return status;
}

/************************************************************************
* ��������:HelloWDMPnp
* ��������:�Լ��弴��IRP���д���
* �����б�:
      fdo:�����豸����
      Irp:��IO�����
* ���� ֵ:����״̬
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo,
                        IN PIRP Irp)
{
	PAGED_CODE();

	KdPrint(("Enter HelloWDMPnp\n"));
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	static NTSTATUS (*fcntab[])(PDEVICE_EXTENSION pdx, PIRP Irp) = 
	{
		HandleStartDevice,		// IRP_MN_START_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_REMOVE_DEVICE
		HandleRemoveDevice,		// IRP_MN_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_RELATIONS
		DefaultPnpHandler,		// IRP_MN_QUERY_INTERFACE
		DefaultPnpHandler,		// IRP_MN_QUERY_CAPABILITIES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_TEXT
		DefaultPnpHandler,		// IRP_MN_FILTER_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// 
		DefaultPnpHandler,		// IRP_MN_READ_CONFIG
		DefaultPnpHandler,		// IRP_MN_WRITE_CONFIG
		DefaultPnpHandler,		// IRP_MN_EJECT
		DefaultPnpHandler,		// IRP_MN_SET_LOCK
		DefaultPnpHandler,		// IRP_MN_QUERY_ID
		DefaultPnpHandler,		// IRP_MN_QUERY_PNP_DEVICE_STATE
		DefaultPnpHandler,		// IRP_MN_QUERY_BUS_INFORMATION
		DefaultPnpHandler,		// IRP_MN_DEVICE_USAGE_NOTIFICATION
		DefaultPnpHandler,		// IRP_MN_SURPRISE_REMOVAL
	};

	ULONG fcn = stack->MinorFunction;
	if (fcn >= arraysize(fcntab))
	{						// δ֪���ӹ��ܴ���
		status = DefaultPnpHandler(pdx, Irp); // some function we don't know about
		return status;
	}						

#if DBG
	static char* fcnname[] = 
	{
		"IRP_MN_START_DEVICE",
		"IRP_MN_QUERY_REMOVE_DEVICE",
		"IRP_MN_REMOVE_DEVICE",
		"IRP_MN_CANCEL_REMOVE_DEVICE",
		"IRP_MN_STOP_DEVICE",
		"IRP_MN_QUERY_STOP_DEVICE",
		"IRP_MN_CANCEL_STOP_DEVICE",
		"IRP_MN_QUERY_DEVICE_RELATIONS",
		"IRP_MN_QUERY_INTERFACE",
		"IRP_MN_QUERY_CAPABILITIES",
		"IRP_MN_QUERY_RESOURCES",
		"IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
		"IRP_MN_QUERY_DEVICE_TEXT",
		"IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
		"",
		"IRP_MN_READ_CONFIG",
		"IRP_MN_WRITE_CONFIG",
		"IRP_MN_EJECT",
		"IRP_MN_SET_LOCK",
		"IRP_MN_QUERY_ID",
		"IRP_MN_QUERY_PNP_DEVICE_STATE",
		"IRP_MN_QUERY_BUS_INFORMATION",
		"IRP_MN_DEVICE_USAGE_NOTIFICATION",
		"IRP_MN_SURPRISE_REMOVAL",
	};

	KdPrint(("PNP Request (%s)\n", fcnname[fcn]));
#endif // DBG

	status = (*fcntab[fcn])(pdx, Irp);
	KdPrint(("Leave HelloWDMPnp\n"));
	return status;
}

/************************************************************************
* ��������:HelloWDMDispatchRoutine
* ��������:��ȱʡIRP���д���
* �����б�:
      fdo:�����豸����
      Irp:��IO�����
* ���� ֵ:����״̬
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT fdo,
								 IN PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter HelloWDMDispatchRoutine\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;	// no bytes xfered
	IoCompleteRequest( Irp, IO_NO_INCREMENT );
	KdPrint(("Leave HelloWDMDispatchRoutine\n"));
	return STATUS_SUCCESS;
}

/************************************************************************
* ��������:HelloWDMUnload
* ��������:�������������ж�ز���
* �����б�:
      DriverObject:��������
* ���� ֵ:����״̬
*************************************************************************/
#pragma PAGEDCODE
void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject)
{
	PAGED_CODE();
	KdPrint(("Enter HelloWDMUnload\n"));
	KdPrint(("Leave HelloWDMUnload\n"));
}

NTSTATUS CompleteRequest(IN PIRP Irp, IN NTSTATUS status, IN ULONG_PTR info)
{							// CompleteRequest
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}
	
NTSTATUS DispatchControl(PDEVICE_OBJECT fdo, PIRP Irp)
{							// DispatchControl
	PAGED_CODE();
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;

	NTSTATUS status;

	ULONG info = 0;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

	switch (code)
	{						// process request
	case IOCTL_READ_BASE_BAR0:
		{
			ULONG offset = *(ULONG*)(Irp->AssociatedIrp.SystemBuffer);
			PUCHAR buff = (PUCHAR)ExAllocatePool(NonPagedPool,cbout);
			READ_REGISTER_BUFFER_UCHAR((PUCHAR)pdx->MemBar0+offset,buff,cbout);
			RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, buff, cbout);
			ExFreePool(buff);
			info = cbout;
			break;
		}
	case IOCTL_WRITE_BASE_BAR0:
		{
			int* tempPointer = (int*)Irp->AssociatedIrp.SystemBuffer;
			ULONG offset = *(ULONG*)(tempPointer);
			tempPointer++;
			PUCHAR buff = *(PUCHAR*)(tempPointer);
			tempPointer++;
			ULONG nInputNumber = *(ULONG*)(tempPointer);
			WRITE_REGISTER_BUFFER_UCHAR((PUCHAR)pdx->MemBar0+offset,buff,nInputNumber);
			break;
		}
	case IOCTL_READ_BASE_BAR2:
		{
			ULONG offset = *(ULONG*)(Irp->AssociatedIrp.SystemBuffer);
			PUCHAR buff = (PUCHAR)ExAllocatePool(NonPagedPool,cbout);
			READ_PORT_BUFFER_UCHAR(pdx->portbase+offset,buff,cbout);
			RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, buff, cbout);
			ExFreePool(buff);
			info = cbout;
			break;
		}
	case IOCTL_WRITE_BASE_BAR2:
		{
			int* tempPointer = (int*)Irp->AssociatedIrp.SystemBuffer;
			ULONG offset = *(ULONG*)(tempPointer);
			tempPointer++;
			PUCHAR buff = *(PUCHAR*)(tempPointer);
			tempPointer++;
			ULONG nInputNumber = *(ULONG*)(tempPointer);
			WRITE_PORT_BUFFER_UCHAR(pdx->portbase+offset,buff,nInputNumber);
			break;
		}
	case IOCTL_READ_IMAGE:
		{
			
			PUCHAR buff = (PUCHAR)ExAllocatePool(NonPagedPool,cbout);
			READ_REGISTER_BUFFER_UCHAR((PUCHAR)pdx->MemForImage,buff,cbout);
			RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, buff, cbout);
			ExFreePool(buff);
			info = cbout;
			break; 
		}
	case IOCTL_WRITE_IMAGE:
		{
			PUCHAR buff = (PUCHAR)ExAllocatePool(NonPagedPool,cbin);
			RtlCopyMemory(buff,Irp->AssociatedIrp.SystemBuffer,cbin);
			WRITE_REGISTER_BUFFER_UCHAR((PUCHAR)pdx->MemForImage,buff,cbin);
			ExFreePool(buff);
			info = cbin;
			break; 
		}
	case IOCTL_ENABLE_INT:
		{
			//�����ж�
			UCHAR HSR = READ_PORT_UCHAR(pdx->portbase);
			HSR = HSR & 0xFB;
			WRITE_PORT_UCHAR(pdx->portbase,HSR);
			break;
		}

	case IOCTL_DISABLE_INT:
		{
			//���ж�
			UCHAR HSR  = READ_PORT_UCHAR(pdx->portbase);
			HSR = HSR | 0x4;
			WRITE_PORT_UCHAR(pdx->portbase,HSR);
			break;
		}

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;

	}						// process request

	return CompleteRequest(Irp, status, info);
}