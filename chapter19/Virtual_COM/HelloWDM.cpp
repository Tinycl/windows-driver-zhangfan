/************************************************************************
* �ļ�����:HelloWDM.cpp                                                 
* ��    ��:�ŷ�
* �������:2007-11-1
*************************************************************************/
#include "HelloWDM.h"

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
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloWDMDispatchControlp;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloWDMCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloWDMClose;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloWDMRead;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloWDMWrite;
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
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName,L"\\Device\\MyWDMDevice");
	status = IoCreateDevice(
		DriverObject,
		sizeof(DEVICE_EXTENSION),
		&(UNICODE_STRING)devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&fdo);
	if( !NT_SUCCESS(status))
		return status;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
	pdx->fdo = fdo;
	pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName,L"\\DosDevices\\COM7");

	pdx->ustrDeviceName = devName;
	pdx->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&(UNICODE_STRING)symLinkName,&(UNICODE_STRING)devName);

	if( !NT_SUCCESS(status))
	{
		IoDeleteSymbolicLink(&pdx->ustrSymLinkName);
		status = IoCreateSymbolicLink(&symLinkName,&devName);
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

#pragma PAGEDCODE
NTSTATUS HelloWDMCreate(IN PDEVICE_OBJECT fdo,
                        IN PIRP Irp)
{
	PAGED_CODE();

	KdPrint(("HelloWDMCreate()\n"));

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
	
	// Init the device extension
	
	pdx->BaudRate = 1200;
	pdx->RTSstate = 0;
	pdx->DTRstate = 0;

	pdx->Timeouts.ReadIntervalTimeout = 0;
	pdx->Timeouts.ReadTotalTimeoutMultiplier = 0;
	pdx->Timeouts.ReadTotalTimeoutConstant = 0;
	pdx->Timeouts.WriteTotalTimeoutMultiplier = 0;
	pdx->Timeouts.WriteTotalTimeoutConstant = 0;
	//pdx->Lc

	pdx->IsOpen = TRUE;

    KeInitializeSpinLock(&pdx->WriteSpinLock);
    KeInitializeSpinLock(&pdx->IoctlSpinLock);

	pdx->EventMask = 0;
	pdx->HistoryEvents = 0;

	pdx->pWaitIrp = NULL;
	pdx->pReadIrp = NULL;

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

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
	IoDeleteSymbolicLink(&(UNICODE_STRING)pdx->ustrSymLinkName);

    //����IoDetachDevice()��fdo���豸ջ���ѿ���
    if (pdx->NextStackDevice)
        IoDetachDevice(pdx->NextStackDevice);
	
    //ɾ��fdo��
    IoDeleteDevice(pdx->fdo);
	KdPrint(("Leave HandleRemoveDevice\n"));
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
		DefaultPnpHandler,		// IRP_MN_START_DEVICE
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

void PrintIoControlCode(ULONG code)
{
	switch(code)
		{
		case IOCTL_SERIAL_CLEAR_STATS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_CLEAR_STATS\n"));
			break;
		case IOCTL_SERIAL_CLR_DTR:
			KdPrint(("IOCTL code: IOCTL_SERIAL_CLR_DTR\n"));
			break;
		case IOCTL_SERIAL_CLR_RTS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_CLR_RTS\n"));
			break;
		case IOCTL_SERIAL_CONFIG_SIZE:
			KdPrint(("IOCTL code: IOCTL_SERIAL_CONFIG_SIZE\n"));
			break;
		case IOCTL_SERIAL_GET_BAUD_RATE:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_BAUD_RATE\n"));
			break;
		case IOCTL_SERIAL_GET_CHARS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_CHARS\n"));
			break;
		case IOCTL_SERIAL_GET_COMMSTATUS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_COMMSTATUS\n"));
			break;
		case IOCTL_SERIAL_GET_DTRRTS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_DTRRTS\n"));
			break;
		case IOCTL_SERIAL_GET_HANDFLOW:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_HANDFLOW\n"));
			break;
		case IOCTL_SERIAL_GET_LINE_CONTROL:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_LINE_CONTROL\n"));
			break;
		case IOCTL_SERIAL_GET_MODEM_CONTROL:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_MODEM_CONTROL\n"));
			break;
		case IOCTL_SERIAL_GET_MODEMSTATUS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_MODEMSTATUS\n"));
			break;
		case IOCTL_SERIAL_GET_PROPERTIES:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_PROPERTIES\n"));
			break;
		case IOCTL_SERIAL_GET_STATS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_STATS\n"));
			break;
		case IOCTL_SERIAL_GET_TIMEOUTS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_TIMEOUTS\n"));
			break;
		case IOCTL_SERIAL_GET_WAIT_MASK:
			KdPrint(("IOCTL code: IOCTL_SERIAL_GET_WAIT_MASK\n"));
			break;
		case IOCTL_SERIAL_IMMEDIATE_CHAR:
			KdPrint(("IOCTL code: IOCTL_SERIAL_IMMEDIATE_CHAR\n"));
			break;
		case IOCTL_SERIAL_LSRMST_INSERT:
			KdPrint(("IOCTL code: IOCTL_SERIAL_LSRMST_INSERT\n"));
			break;
		case IOCTL_SERIAL_PURGE:
			KdPrint(("IOCTL code: IOCTL_SERIAL_PURGE\n"));
			break;
		case IOCTL_SERIAL_RESET_DEVICE:
			KdPrint(("IOCTL code: IOCTL_SERIAL_RESET_DEVICE\n"));
			break;
		case IOCTL_SERIAL_SET_BAUD_RATE:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_BAUD_RATE\n"));
			break;
		case IOCTL_SERIAL_SET_BREAK_OFF:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_BREAK_OFF\n"));
			break;
		case IOCTL_SERIAL_SET_BREAK_ON:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_BREAK_ON\n"));
			break;
		case IOCTL_SERIAL_SET_CHARS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_CHARS\n"));
			break;
		case IOCTL_SERIAL_SET_DTR:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_DTR\n"));
			break;
		case IOCTL_SERIAL_SET_FIFO_CONTROL:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_FIFO_CONTROL\n"));
			break;
		case IOCTL_SERIAL_SET_HANDFLOW:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_HANDFLOW\n"));
			break;
		case IOCTL_SERIAL_SET_LINE_CONTROL:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_LINE_CONTROL\n"));
			break;
		case IOCTL_SERIAL_SET_MODEM_CONTROL:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_MODEM_CONTROL\n"));
			break;
		case IOCTL_SERIAL_SET_QUEUE_SIZE:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_QUEUE_SIZE\n"));
			break;
		case IOCTL_SERIAL_SET_RTS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_RTS\n"));
			break;
		case IOCTL_SERIAL_SET_TIMEOUTS:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_TIMEOUTS\n"));
			break;
		case IOCTL_SERIAL_SET_WAIT_MASK:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_WAIT_MASK\n"));
			break;
		case IOCTL_SERIAL_SET_XOFF:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_XOFF\n"));
			break;
		case IOCTL_SERIAL_SET_XON:
			KdPrint(("IOCTL code: IOCTL_SERIAL_SET_XON\n"));
			break;
		case IOCTL_SERIAL_WAIT_ON_MASK:
			KdPrint(("IOCTL code: IOCTL_SERIAL_WAIT_ON_MASK\n"));
			break;
		case IOCTL_SERIAL_XOFF_COUNTER:
			KdPrint(("IOCTL code: IOCTL_SERIAL_XOFF_COUNTER\n"));
			break;

		default:
			KdPrint(("IOCTL code: unkown\n"));
			
		}
}
VOID DriverCancelWaitIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	KdPrint(("DriverCancelWaitIrp\n"));
	PDEVICE_EXTENSION pExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    KIRQL                   OldIrql;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    KeAcquireSpinLock(&pExtension->IoctlSpinLock, &OldIrql);

    pExtension->pWaitIrp = NULL;        

    KeReleaseSpinLock(&pExtension->IoctlSpinLock, OldIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

VOID DriverCheckEvent(IN PDEVICE_EXTENSION pExtension, IN ULONG events)
{
	KdPrint(("DriverCheckEvent\n"));
    PIRP            pOldWaitIrp = NULL;
    PDRIVER_CANCEL  pOldCancelRoutine;
    KIRQL           OldIrql;

    KeAcquireSpinLock(&pExtension->IoctlSpinLock, &OldIrql);

    pExtension->HistoryEvents |= events;

    events &= pExtension->EventMask;

	//�൱�����ô����¼�
    if ((pExtension->pWaitIrp != NULL) && (events != 0))
    {
        pOldWaitIrp = pExtension->pWaitIrp;

        pOldCancelRoutine = IoSetCancelRoutine(pOldWaitIrp, NULL);

        //�Ƿ��Ѿ���cancel��?
        if (pOldCancelRoutine != NULL)
        {
            // Nein, also Request beenden
            pOldWaitIrp->IoStatus.Information = sizeof(ULONG);
            *(PULONG)pOldWaitIrp->AssociatedIrp.SystemBuffer = events;

            pOldWaitIrp->IoStatus.Status = STATUS_SUCCESS;

            pExtension->pWaitIrp      = NULL;
            pExtension->HistoryEvents = 0;
        }
        else
        {
			//���cancel�����Ͳ���IoCompleteRequest��
            pOldWaitIrp = NULL;
        }
    }

    KeReleaseSpinLock(&pExtension->IoctlSpinLock, OldIrql);

    if (pOldWaitIrp != NULL)
	{
		KdPrint(("complete the wait irp\n"));
        IoCompleteRequest(pOldWaitIrp, IO_NO_INCREMENT);
	}
}
#pragma PAGEDCODE
NTSTATUS HelloWDMDispatchControlp(PDEVICE_OBJECT fdo, 
										PIRP Irp)
{
	PAGED_CODE();

    NTSTATUS            ntStatus = STATUS_SUCCESS;// Assume success

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    KIRQL                       OldIrql;

	Irp->IoStatus.Information = 0;
	ntStatus = STATUS_SUCCESS;

	PrintIoControlCode(irpSp->Parameters.DeviceIoControl.IoControlCode);

    switch ( irpSp->Parameters.DeviceIoControl.IoControlCode )

     {
        case IOCTL_SERIAL_SET_BAUD_RATE: 
		{
			
			pdx->BaudRate = ((PSERIAL_BAUD_RATE)(Irp->AssociatedIrp.SystemBuffer))->BaudRate;

			Irp->IoStatus.Information = 0;
			break;
		}

        case IOCTL_SERIAL_GET_BAUD_RATE: 
		{
            PSERIAL_BAUD_RATE Br = (PSERIAL_BAUD_RATE)Irp->AssociatedIrp.SystemBuffer;
			Br->BaudRate = pdx->BaudRate;

            Irp->IoStatus.Information = sizeof(SERIAL_BAUD_RATE);
			break;
		}

        case IOCTL_SERIAL_SET_RTS:
		{
			pdx->RTSstate = 1;
			break;
		}

        case IOCTL_SERIAL_CLR_RTS: 
		{
			pdx->RTSstate = 0;
			break;
		}

        case IOCTL_SERIAL_SET_DTR:
		{
			pdx->DTRstate = 1;
			break;
		}

        case IOCTL_SERIAL_CLR_DTR: 
		{
			pdx->DTRstate = 0;
			break;
		}

        case IOCTL_SERIAL_GET_DTRRTS: 
		{
            ULONG ModemControl;
            ModemControl = pdx->DTRstate + (pdx->RTSstate<<1);
            
			*(PULONG)Irp->AssociatedIrp.SystemBuffer = ModemControl;
            Irp->IoStatus.Information = sizeof(ULONG);
			
			break;
		}

        case IOCTL_SERIAL_GET_MODEMSTATUS:
		{
			ULONG Cts, Dsr, Dcd;

			Cts = 1; //�Է���������				//pdx->pOther->RTSstate;
			Dsr = 1; //�Է������ն��Ƿ�׼����	//pdx->pOther->DTRstate;

			Dcd = 1; //�Է��豸�Ƿ��	//pdx->pOther->IsOpen;

			*(PULONG)Irp->AssociatedIrp.SystemBuffer = 
				(Cts ? SERIAL_CTS_STATE : 0) | (Dsr ? SERIAL_DSR_STATE : 0) | 
				(Dcd ? SERIAL_DCD_STATE : 0);
            Irp->IoStatus.Information = sizeof(ULONG);

			break;
		}

        case IOCTL_SERIAL_SET_TIMEOUTS: 
		{
			pdx->Timeouts = *((PSERIAL_TIMEOUTS)(Irp->AssociatedIrp.SystemBuffer));
			break;
		}

        case IOCTL_SERIAL_GET_TIMEOUTS: 
		{
            *((PSERIAL_TIMEOUTS)Irp->AssociatedIrp.SystemBuffer) = pdx->Timeouts;
            Irp->IoStatus.Information = sizeof(SERIAL_TIMEOUTS);
			break;
		}

        case IOCTL_SERIAL_RESET_DEVICE: 
		{
			break;
		}

        case IOCTL_SERIAL_PURGE:
        {
			break;
		}

        case IOCTL_SERIAL_SET_LINE_CONTROL: 
		{
			pdx->Lc = *((PSERIAL_LINE_CONTROL)(Irp->AssociatedIrp.SystemBuffer));
			break;
		}

        case IOCTL_SERIAL_GET_LINE_CONTROL: 
		{
			*((PSERIAL_LINE_CONTROL)(Irp->AssociatedIrp.SystemBuffer)) = pdx->Lc;

            Irp->IoStatus.Information = sizeof(SERIAL_LINE_CONTROL);
			break;
		}

        case IOCTL_SERIAL_SET_WAIT_MASK:
        {
            PIRP            pOldWaitIrp;
            PDRIVER_CANCEL  pOldCancelRoutine;

			pdx->EventMask = *(PULONG)Irp->AssociatedIrp.SystemBuffer;

            KeAcquireSpinLock(&pdx->IoctlSpinLock, &OldIrql);

            pOldWaitIrp = pdx->pWaitIrp;
            if (pOldWaitIrp != NULL)
            {
                pOldCancelRoutine = IoSetCancelRoutine(pOldWaitIrp, NULL);

				//����ǰû�н���������̵ĵȴ�irp���������
                if (pOldCancelRoutine != NULL)
                {
                    pOldWaitIrp->IoStatus.Information = sizeof(ULONG);
                    *(PULONG)pOldWaitIrp->AssociatedIrp.SystemBuffer = 0;

                    pOldWaitIrp->IoStatus.Status = STATUS_SUCCESS;

                    pdx->pWaitIrp = NULL;
                }
                else
                {
                    pOldWaitIrp = NULL;
                }
            }

            KeReleaseSpinLock(&pdx->IoctlSpinLock, OldIrql);

            if (pOldWaitIrp != NULL)
			{
                IoCompleteRequest(pOldWaitIrp, IO_NO_INCREMENT);
			}

			break;
		}

        case IOCTL_SERIAL_GET_WAIT_MASK:
		{
            *(PULONG)Irp->AssociatedIrp.SystemBuffer = pdx->EventMask;

            Irp->IoStatus.Information = sizeof(ULONG);
			break;
		}
		
		case IOCTL_SERIAL_WAIT_ON_MASK:
		{
			PDRIVER_CANCEL  pOldCancelRoutine;

			KeAcquireSpinLock(&pdx->IoctlSpinLock, &OldIrql);

			//�ȴ�irpһ�����������eventMaskһ����Ϊ0
			if ((pdx->pWaitIrp != NULL) || (pdx->EventMask == 0))
				ntStatus = STATUS_INVALID_PARAMETER;
			else if ((pdx->EventMask & pdx->HistoryEvents) != 0)
			{
                // Some events happened
                Irp->IoStatus.Information = sizeof(ULONG);
                *(PULONG)Irp->AssociatedIrp.SystemBuffer = pdx->EventMask & pdx->HistoryEvents;
                pdx->HistoryEvents = 0;
                ntStatus = STATUS_SUCCESS;
			}else
			{
				pdx->pWaitIrp = Irp;

				ntStatus = STATUS_PENDING;

				IoSetCancelRoutine(Irp, DriverCancelWaitIrp);

				if (Irp->Cancel)
				{
					pOldCancelRoutine = IoSetCancelRoutine(Irp, NULL);

                    if (pOldCancelRoutine != NULL)
                    {
                        ntStatus = STATUS_CANCELLED;

                        pdx->pWaitIrp = NULL;
                    }
                    else
                    {
						IoMarkIrpPending(Irp);
                    }
                }
                else
				{
                    IoMarkIrpPending(Irp);
				}
            }

            KeReleaseSpinLock(&pdx->IoctlSpinLock, OldIrql);
			break;

 		}

        case IOCTL_SERIAL_GET_COMMSTATUS:
		{
            PSERIAL_STATUS  pStatus = (PSERIAL_STATUS)Irp->AssociatedIrp.SystemBuffer;

			ULONG InputLen;

            KeAcquireSpinLock(&pdx->WriteSpinLock, &OldIrql);

			InputLen = pdx->uReadWrite;

            KeReleaseSpinLock(&pdx->WriteSpinLock, OldIrql);

            RtlZeroMemory(Irp->AssociatedIrp.SystemBuffer, sizeof(SERIAL_STATUS));
            pStatus->AmountInInQueue  = InputLen;
            pStatus->AmountInOutQueue = 0;

            Irp->IoStatus.Information = sizeof(SERIAL_STATUS);
			break;
		}

        case IOCTL_SERIAL_GET_CHARS:
		{
            RtlZeroMemory(Irp->AssociatedIrp.SystemBuffer, sizeof(SERIAL_CHARS));
            Irp->IoStatus.Information = sizeof(SERIAL_CHARS);
			break;
		}

        case IOCTL_SERIAL_GET_HANDFLOW:
		{
			break;
		}

		default:
		{
			break;
		}
	 
	}

    Irp->IoStatus.Status = ntStatus;

	if (ntStatus!=STATUS_PENDING)
		IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return ntStatus;
}

#pragma PAGEDCODE
NTSTATUS HelloWDMClose(IN PDEVICE_OBJECT fdo,
                        IN PIRP Irp)
{
	PAGED_CODE();

	KdPrint(("HelloWDMClose()\n"));

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;

	pdx->pWaitIrp = NULL;
	pdx->pReadIrp = NULL;

	pdx->IsOpen = FALSE;

	pdx->uReadWrite = 0;

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;

}
VOID DriverCancelCurrentReadIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp)
{
	PDEVICE_EXTENSION pExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    KIRQL                   OldIrql;

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    KeAcquireSpinLock(&pExtension->WriteSpinLock, &OldIrql);

    pExtension->pReadIrp = NULL;        

    KeReleaseSpinLock(&pExtension->WriteSpinLock, OldIrql);

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}
NTSTATUS HelloWDMRead(IN PDEVICE_OBJECT fdo,
                        IN PIRP Irp)
{
	KdPrint(("HelloWDMRead\n"));

    NTSTATUS ntStatus = STATUS_SUCCESS;// Assume success

	PDEVICE_EXTENSION pExtension = (PDEVICE_EXTENSION)fdo->DeviceExtension;

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

	ULONG BufLen = irpSp->Parameters.Read.Length;
	PCHAR pBuf = (PCHAR)Irp->AssociatedIrp.SystemBuffer;

    KIRQL OldIrql;

    PDRIVER_CANCEL pOldCancelRoutine;

	Irp->IoStatus.Information = 0;
	
	DbgPrint("DeviceObject:%08X Read\n",fdo);

	if (BufLen == 0)
    {
		ntStatus = STATUS_SUCCESS;
    }
	else
	{
        KeAcquireSpinLock(&pExtension->WriteSpinLock, &OldIrql);

		RtlCopyMemory(pBuf,pExtension->Buffer,BufLen);

		Irp->IoStatus.Information = BufLen;

		if (BufLen==0 && pExtension->pReadIrp==NULL) // nothing, store
		{
			pExtension->pReadIrp = Irp;
			Irp->IoStatus.Status = ntStatus = STATUS_PENDING;

            IoSetCancelRoutine(Irp, DriverCancelCurrentReadIrp);

            if (Irp->Cancel)
            {
                pOldCancelRoutine = IoSetCancelRoutine(Irp, NULL);

                if (pOldCancelRoutine != NULL)
                {
                    // Nein, also IRP hier abbrechen
                    Irp->IoStatus.Status = ntStatus = STATUS_CANCELLED;

                    pExtension->pReadIrp = NULL;
                }
                else
                {
                    // Ja, Cancel-Routine wird Request beenden
                    IoMarkIrpPending(Irp);
                }
			}
			else
			{
                    IoMarkIrpPending(Irp);
			}
		}

        KeReleaseSpinLock(&pExtension->WriteSpinLock, OldIrql);
	
	}

    Irp->IoStatus.Status = ntStatus;
	if (ntStatus != STATUS_PENDING)
		IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return ntStatus;
}

NTSTATUS HelloWDMWrite(IN PDEVICE_OBJECT fdo,
                        IN PIRP Irp)
{
	KdPrint(("HelloWDMWrite\n"));
	
    NTSTATUS ntStatus = STATUS_SUCCESS;// Assume success

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

	ULONG DataLen = irpSp->Parameters.Write.Length;
	
	PUCHAR pData = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;

    KIRQL OldIrql;

    PIRP            pOldReadIrp = NULL;
    PDRIVER_CANCEL  pOldCancelRoutine;

	Irp->IoStatus.Information = 0;
	ntStatus = STATUS_SUCCESS;

	if (DataLen == 0)
    {
		ntStatus = STATUS_SUCCESS;
    }else if (DataLen>COMBUFLEN)
    {
		ntStatus = STATUS_INVALID_PARAMETER;
    }
	else
	{
		KdPrint(("Write\n"));
        
		KeAcquireSpinLock(&pdx->WriteSpinLock, &OldIrql);

		RtlCopyMemory(pdx->Buffer,pData,DataLen);

		pdx->uReadWrite = DataLen;

		if (pdx->pReadIrp != NULL) // drop it out
		{

			pOldReadIrp = pdx->pReadIrp;

			pOldCancelRoutine = IoSetCancelRoutine(pOldReadIrp, NULL);

			if (pOldCancelRoutine != NULL)
			{
				pOldReadIrp->IoStatus.Information = 0;

				pOldReadIrp->IoStatus.Status = STATUS_SUCCESS;

				pdx->pReadIrp = NULL;
			}
			else
			{
				pOldReadIrp = NULL;
			}

		}

		DriverCheckEvent(pdx, SERIAL_EV_RXCHAR | SERIAL_EV_RX80FULL);

//		DriverCheckEvent(pdx, SERIAL_EV_TXEMPTY);

        KeReleaseSpinLock(&pdx->WriteSpinLock, OldIrql);

	    if (pOldReadIrp != NULL)
		    IoCompleteRequest(pOldReadIrp, IO_NO_INCREMENT);
	}

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = DataLen;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return ntStatus;

}