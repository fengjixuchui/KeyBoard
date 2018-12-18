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
		KdPrint(("KeyBoardDriver: ����ж��\n"));
		
	}



	NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
	{
		NTSTATUS status = STATUS_SUCCESS;

		ULONG i = 0;

		DriverObject->DriverUnload = Unload;
		
		KdPrint(("KeyBoardDriver: ��������\n"));

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

		// �Ȼ�ȡkbdclass��������

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
			KdPrint((DBGSTRING"��ȡ��������ʧ��\n"));
			return status;
		}
		else
		{
			ObDereferenceObject(KbdDriverObject);
		}

		// ��ȡ�������豸�����׸��豸����

		PDEVICE_OBJECT KbdDeviceObject = KbdDriverObject->DeviceObject;


		PDEVICE_OBJECT FltDeviceObject = NULL;

		// ��ʼ�������豸
		while(KbdDeviceObject)
		{
			status = IoCreateDevice(
				DriverObject,
				sizeof(DEVICE_EXTENSION_FOR_KBD),					//Ϊ�˼�¼�豸ջ����һ���豸���������豸��չ
				NULL,
				KbdDeviceObject->DeviceType,
				KbdDeviceObject->Characteristics,
				FALSE,
				&FltDeviceObject);
			if(!NT_SUCCESS(status))
			{
				KdPrint((DBGSTRING"�����豸ʧ��\n"));

				return status;
			}

			PDEVICE_OBJECT LowerDeviceObject = IoAttachDeviceToDeviceStack(FltDeviceObject, KbdDeviceObject);

			if(!LowerDeviceObject)
			{
				KdPrint((DBGSTRING"���豸ʧ��\n"));
				status = STATUS_UNSUCCESSFUL;
				IoDeleteDevice(FltDeviceObject);
				FltDeviceObject = NULL;
				return status;
			}

			// �����豸��һЩ����
			FltDeviceObject->DeviceType = LowerDeviceObject->DeviceType;
			FltDeviceObject->Characteristics = LowerDeviceObject->Characteristics;
			FltDeviceObject->StackSize = LowerDeviceObject->StackSize + 1;
			FltDeviceObject->Flags |= LowerDeviceObject->Flags & (DO_DIRECT_IO | DO_BUFFERED_IO | DO_POWER_PAGABLE);			//���ö�д�ķ�ʽ

			// ��¼�豸ջ����һ���豸
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
		IoCallDriver(DeviceExtension->LowerDeviceObject, Irp);
		return status;
	}

#ifdef __cplusplus
	}
#endif
