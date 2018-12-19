#pragma once
#ifdef __cplusplus

extern "C"
{
#endif

#include <ntddk.h>

#include <ntddkbd.h>

// �Ȱ����е��豸
NTSTATUS AttachAllDevice(PDRIVER_OBJECT DriverObject);

// ͨ�õ���ǲ����
NTSTATUS CommonDisPatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// ��������Ϣ
NTSTATUS ReadDisPatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// ����ɺ���
NTSTATUS ReadCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);

// hookд��ǲ����
NTSTATUS HookWriteDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS HookInstall();
#ifdef __cplusplus
}
#endif

