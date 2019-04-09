/************************************************************************
* �ļ�����:HelloWDM.h                                                 
* ��    ��:�ŷ�
* �������:2007-11-1
*************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif 

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT NextStackDevice;
	UNICODE_STRING interfaceName;   //�豸�ӿ�
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define PAGEDCODE code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("INIT")

#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

NTSTATUS HelloWDMAddDevice(IN PDRIVER_OBJECT DriverObject,
                           IN PDEVICE_OBJECT PhysicalDeviceObject);
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo,
                        IN PIRP Irp);
NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT fdo,
								 IN PIRP Irp);
void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject);

NTSTATUS HelloWDMDeviceIOControl(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp);

extern "C"
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
                     IN PUNICODE_STRING RegistryPath);