#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#include <ntddk.h>

// �Ȱ����е��豸
NTSTATUS AttachAllDevice(PDRIVER_OBJECT DriverObject);

// ͨ�õ���ǲ����
NTSTATUS CommonDisPatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

#ifdef __cplusplus
}
#endif

