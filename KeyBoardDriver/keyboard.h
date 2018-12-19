#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#include <ntddk.h>

#include <ntddkbd.h>

// 先绑定所有的设备
NTSTATUS AttachAllDevice(PDRIVER_OBJECT DriverObject);

// 通用的派遣函数
NTSTATUS CommonDisPatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// 读按键信息
NTSTATUS ReadDisPatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// 读完成函数
NTSTATUS ReadCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);

#ifdef __cplusplus
}
#endif

