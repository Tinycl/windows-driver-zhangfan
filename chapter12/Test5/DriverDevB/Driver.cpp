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

	pDevObj->Flags |= DO_DIRECT_IO;
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

const int MAX_PACKAGE_SIZE=1024;

#pragma PAGEDCODE
NTSTATUS
HelloDDKReadCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
	KdPrint(("DriverB:Enter B HelloDDKReadCompletion\n"));

	PMYDRIVER_RW_CONTEXT rwContext = (PMYDRIVER_RW_CONTEXT) Context;
    NTSTATUS ntStatus = Irp->IoStatus.Status;

	ULONG stageLength;

	if(rwContext && NT_SUCCESS(ntStatus)) 
	{
		//�Ѿ������˶����ֽ�
		rwContext->Numxfer += Irp->IoStatus.Information;

       if(rwContext->Length) 
	   {
		   //�趨��һ�׶ζ�ȡ�ֽ���
            if(rwContext->Length > MAX_PACKAGE_SIZE) 
			{
				stageLength = MAX_PACKAGE_SIZE;
            }
            else 
			{
                stageLength = rwContext->Length;
            }
			//��������MDL
            MmPrepareMdlForReuse(rwContext->NewMdl);

            IoBuildPartialMdl(Irp->MdlAddress,
                              rwContext->NewMdl,
                              (PVOID) rwContext->VirtualAddress,
                              stageLength);
        
            rwContext->VirtualAddress += stageLength;
            rwContext->Length -= stageLength;

			IoCopyCurrentIrpStackLocationToNext(Irp);
            PIO_STACK_LOCATION nextStack = IoGetNextIrpStackLocation(Irp);

			nextStack->Parameters.Read.Length = stageLength;

            IoSetCompletionRoutine(Irp,
                                   HelloDDKReadCompletion,
                                   rwContext,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            IoCallDriver(rwContext->DeviceExtension->TargetDevice, 
                         Irp);

            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        else 
		{
			//���һ�δ���
            Irp->IoStatus.Information = rwContext->Numxfer;
        }
	}

	KdPrint(("DriverB:Leave B HelloDDKReadCompletion\n"));
	return STATUS_MORE_PROCESSING_REQUIRED;  
}

#pragma PAGEDCODE
NTSTATUS HelloDDKRead(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("DriverB:Enter B HelloDDKRead\n"));
	NTSTATUS status = STATUS_SUCCESS;

	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
		pDevObj->DeviceExtension;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	
	ULONG totalLength;
	ULONG stageLength;
	PMDL mdl;
	PVOID virtualAddress;
	PMYDRIVER_RW_CONTEXT rwContext = NULL;
	PIO_STACK_LOCATION nextStack;

	if (!pIrp->MdlAddress)
	{
		status = STATUS_UNSUCCESSFUL;
		totalLength = 0;
		goto HelloDDKRead_EXIT;
	}

	//��ȡMDL�������ַ
	virtualAddress = MmGetMdlVirtualAddress(pIrp->MdlAddress);
	//��ȡMDL�ĳ���
	totalLength = MmGetMdlByteCount(pIrp->MdlAddress);

	KdPrint(("DriverB:(pIrp->MdlAddress)MmGetMdlVirtualAddress:%08X\n",MmGetMdlVirtualAddress(pIrp->MdlAddress)));
	KdPrint(("DriverB:(pIrp->MdlAddress)MmGetMdlByteCount:%d\n",MmGetMdlByteCount(pIrp->MdlAddress)));

	//���ܵĴ��䣬�ֳɼ����׶Σ������趨ÿ�ν׶εĳ���
    if(totalLength > MAX_PACKAGE_SIZE) 
	{
        stageLength = MAX_PACKAGE_SIZE;
    }else 
	{
        stageLength = totalLength;
    }

	//�����µ�MDL
    mdl = IoAllocateMdl((PVOID) virtualAddress,
                        totalLength,
                        FALSE,
                        FALSE,
                        NULL);

	KdPrint(("DriverB:(new mdl)MmGetMdlVirtualAddress:%08X\n",MmGetMdlVirtualAddress(mdl)));
	KdPrint(("DriverB:(new mdl)MmGetMdlByteCount:%d\n",MmGetMdlByteCount(mdl)));

    if(mdl == NULL) 
	{
		KdPrint(("DriverB:Failed to alloc mem for mdl\n"));
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto HelloDDKRead_EXIT;
    }

	//��IRP��MDL������ӳ��
    IoBuildPartialMdl(pIrp->MdlAddress,
                      mdl,
                      (PVOID) virtualAddress,
                      stageLength);
	KdPrint(("DriverB:(new mdl)MmGetMdlVirtualAddress:%08X\n",MmGetMdlVirtualAddress(mdl)));
	KdPrint(("DriverB:(new mdl)MmGetMdlByteCount:%d\n",MmGetMdlByteCount(mdl)));

	rwContext = (PMYDRIVER_RW_CONTEXT)
                ExAllocatePool(NonPagedPool,sizeof(MYDRIVER_RW_CONTEXT));

    rwContext->NewMdl			= mdl;
	rwContext->PreviousMdl		= pIrp->MdlAddress;
    rwContext->Length			= totalLength - stageLength;//��ʣ�¶���û��ȡ
    rwContext->Numxfer			= 0;						//���˶����ֽ�
    rwContext->VirtualAddress	= ((ULONG_PTR)virtualAddress + stageLength);//��һ�׶ο�ʼ��ȡ�ĵ�ַ
	rwContext->DeviceExtension	= pDevExt;

	//�������ײ��ջ
	IoCopyCurrentIrpStackLocationToNext(pIrp);

	nextStack = IoGetNextIrpStackLocation(pIrp);
	//���ݵײ�������ʵ�֣��ײ������п��ܶ�ȡ�����ֵ��Ҳ�п��ܶ�ȡmdl��length��
	nextStack->Parameters.Read.Length = stageLength;

	pIrp->MdlAddress = mdl;
	
	//�趨�������
    IoSetCompletionRoutine(pIrp, 
                           (PIO_COMPLETION_ROUTINE)HelloDDKReadCompletion,
                           rwContext,
                           TRUE,
                           TRUE,
                           TRUE);

    IoCallDriver(pDevExt->TargetDevice,pIrp);
	
	pIrp->MdlAddress = rwContext->PreviousMdl;
	IoFreeMdl(rwContext->NewMdl);

HelloDDKRead_EXIT:
	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = totalLength;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("DriverB:Leave B HelloDDKRead\n"));
	return status;
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