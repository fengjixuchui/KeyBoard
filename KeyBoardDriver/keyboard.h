#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#include <ntddk.h>

// 先绑定所有的设备
NTSTATUS AttachAllDevice(PDRIVER_OBJECT DriverObject);

// 通用的派遣函数
NTSTATUS CommonDisPatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

#ifdef __cplusplus
}
#endif

