#include "keyboard.h"

#ifdef __cplusplus
extern  "C"
{
#endif

	NTSYSAPI
		NTSTATUS NTAPI ObReferenceObjectByName(

			IN PUNICODE_STRING ObjectPath,

			IN ULONG Attributes,

			IN PACCESS_STATE PassedAccessState OPTIONAL,

			IN ACCESS_MASK DesiredAccess OPTIONAL,

			IN POBJECT_TYPE ObjectType,

			IN KPROCESSOR_MODE AccessMode,

			IN OUT PVOID ParseContext OPTIONAL,

			OUT PVOID *ObjectPtr

		);

	extern POBJECT_TYPE * IoDriverObjectType;

#define DBGSTRING		"KeyBoardDriver: " 

typedef struct _DEVICE_EXTENSION_FOR_KBD_
{
	PDEVICE_OBJECT LowerDeviceObject;
}DEVICE_EXTENSION_FOR_KBD, *PDEVICE_EXTENSION_FOR_KBD;


	VOID Unload(PDRIVER_OBJECT DriverObject)
	{
		KdPrint(("KeyBoardDriver: 驱动卸载\n"));

		// 删除创建的设备

		PDEVICE_OBJECT FltDeviceObject = DriverObject->DeviceObject;

		while(FltDeviceObject != NULL)
		{
			IoDeleteDevice(FltDeviceObject);

			FltDeviceObject = FltDeviceObject->NextDevice;
		}
		
	}



	NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
	{
		NTSTATUS status = STATUS_SUCCESS;

		ULONG i = 0;

		DriverObject->DriverUnload = Unload;
		
		KdPrint(("KeyBoardDriver: 驱动加载\n"));

		for(i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		{
			DriverObject->MajorFunction[i] = CommonDisPatch;
		}

		status = AttachAllDevice(DriverObject);

		return status;
	}


	NTSTATUS AttachAllDevice(PDRIVER_OBJECT DriverObject)
	{
		NTSTATUS status = STATUS_SUCCESS;

		// 先获取kbdclass驱动对象

		PDRIVER_OBJECT KbdDriverObject = NULL;
		UNICODE_STRING KbdClassString = RTL_CONSTANT_STRING(L"\\Driver\\KbdClass");

		status = ObReferenceObjectByName(
			&KbdClassString,
			OBJ_CASE_INSENSITIVE,
			NULL,
			0,
			*IoDriverObjectType,
			KernelMode,
			NULL,
			(PVOID *)&KbdDriverObject
		);

		if(!NT_SUCCESS(status))
		{
			KdPrint((DBGSTRING"获取驱动对象失败\n"));
			return status;
		}
		else
		{
			ObDereferenceObject(KbdDriverObject);
		}

		// 获取驱动的设备链的首个设备对象

		PDEVICE_OBJECT KbdDeviceObject = KbdDriverObject->DeviceObject;


		PDEVICE_OBJECT FltDeviceObject = NULL;

		// 开始绑定所有设备
		while(KbdDeviceObject)
		{
			status = IoCreateDevice(
				DriverObject,
				sizeof(DEVICE_EXTENSION_FOR_KBD),					//为了记录设备栈的下一个设备，故申请设备扩展
				NULL,
				KbdDeviceObject->DeviceType,
				KbdDeviceObject->Characteristics,
				FALSE,
				&FltDeviceObject);
			if(!NT_SUCCESS(status))
			{
				KdPrint((DBGSTRING"创建设备失败\n"));

				return status;
			}

			PDEVICE_OBJECT LowerDeviceObject = IoAttachDeviceToDeviceStack(FltDeviceObject, KbdDeviceObject);

			if(!LowerDeviceObject)
			{
				KdPrint((DBGSTRING"绑定设备失败\n"));
				status = STATUS_UNSUCCESSFUL;
				IoDeleteDevice(FltDeviceObject);
				FltDeviceObject = NULL;
				return status;
			}

			// 设置设备的一些属性
			FltDeviceObject->DeviceType = LowerDeviceObject->DeviceType;
			FltDeviceObject->Characteristics = LowerDeviceObject->Characteristics;
			FltDeviceObject->StackSize = LowerDeviceObject->StackSize + 1;
			FltDeviceObject->Flags |= LowerDeviceObject->Flags & (DO_DIRECT_IO | DO_BUFFERED_IO | DO_POWER_PAGABLE);			//设置读写的方式

			// 记录设备栈的下一个设备
			PDEVICE_EXTENSION_FOR_KBD DeviceExtension = (PDEVICE_EXTENSION_FOR_KBD)FltDeviceObject->DeviceExtension;
			DeviceExtension->LowerDeviceObject = LowerDeviceObject;

			KbdDeviceObject = KbdDeviceObject->NextDevice;
		}

		return status;
	}

	NTSTATUS CommonDisPatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
	{
		NTSTATUS status = STATUS_SUCCESS;
		PDEVICE_EXTENSION_FOR_KBD DeviceExtension = (PDEVICE_EXTENSION_FOR_KBD)DeviceObject->DeviceExtension;
		IoSkipCurrentIrpStackLocation(Irp);
		status = IoCallDriver(DeviceExtension->LowerDeviceObject, Irp);
		return status;
	}

#ifdef __cplusplus
	}
#endif
