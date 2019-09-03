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

typedef NTSTATUS (* DispatchRoutine)(PDEVICE_OBJECT DeviceObject, PIRP Irp);
DispatchRoutine OrignalWriteDispatch;

#define DBGSTRING		"KeyBoardDriver: " 

typedef struct _DEVICE_EXTENSION_FOR_KBD_
{
	PDEVICE_OBJECT LowerDeviceObject;
}DEVICE_EXTENSION_FOR_KBD, *PDEVICE_EXTENSION_FOR_KBD;

unsigned char asciiTbl[] = {
	0x00, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08, 0x09,	//normal
		0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6F, 0x70, 0x5B, 0x5D, 0x0D, 0x00, 0x61, 0x73,
		0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C, 0x3B, 0x27, 0x60, 0x00, 0x5C, 0x7A, 0x78, 0x63, 0x76,
		0x62, 0x6E, 0x6D, 0x2C, 0x2E, 0x2F, 0x00, 0x2A, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x38, 0x39, 0x2D, 0x34, 0x35, 0x36, 0x2B, 0x31,
		0x32, 0x33, 0x30, 0x2E,
		0x00, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08, 0x09,	//caps
		0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4F, 0x50, 0x5B, 0x5D, 0x0D, 0x00, 0x41, 0x53,
		0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0x3B, 0x27, 0x60, 0x00, 0x5C, 0x5A, 0x58, 0x43, 0x56,
		0x42, 0x4E, 0x4D, 0x2C, 0x2E, 0x2F, 0x00, 0x2A, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x38, 0x39, 0x2D, 0x34, 0x35, 0x36, 0x2B, 0x31,
		0x32, 0x33, 0x30, 0x2E,
		0x00, 0x1B, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5E, 0x26, 0x2A, 0x28, 0x29, 0x5F, 0x2B, 0x08, 0x09,	//shift
		0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4F, 0x50, 0x7B, 0x7D, 0x0D, 0x00, 0x41, 0x53,
		0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0x3A, 0x22, 0x7E, 0x00, 0x7C, 0x5A, 0x58, 0x43, 0x56,
		0x42, 0x4E, 0x4D, 0x3C, 0x3E, 0x3F, 0x00, 0x2A, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x38, 0x39, 0x2D, 0x34, 0x35, 0x36, 0x2B, 0x31,
		0x32, 0x33, 0x30, 0x2E,
		0x00, 0x1B, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5E, 0x26, 0x2A, 0x28, 0x29, 0x5F, 0x2B, 0x08, 0x09,	//caps + shift
		0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6F, 0x70, 0x7B, 0x7D, 0x0D, 0x00, 0x61, 0x73,
		0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C, 0x3A, 0x22, 0x7E, 0x00, 0x7C, 0x7A, 0x78, 0x63, 0x76,
		0x62, 0x6E, 0x6D, 0x3C, 0x3E, 0x3F, 0x00, 0x2A, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x38, 0x39, 0x2D, 0x34, 0x35, 0x36, 0x2B, 0x31,
		0x32, 0x33, 0x30, 0x2E
};

// flags for keyboard status
#define	S_SHIFT				1
#define	S_CAPS				2
#define	S_NUM				4
static int kb_status = S_NUM;
void __stdcall print_keystroke(UCHAR sch)
{
	UCHAR	ch = 0;
	int		off = 0;

	if ((sch & 0x80) == 0)	//make
	{
		if ((sch < 0x47) ||
			((sch >= 0x47 && sch < 0x54) && (kb_status & S_NUM))) // Num Lock
		{
			ch = asciiTbl[off + sch];
		}

		switch (sch)
		{
		case 0x3A:
			kb_status ^= S_CAPS;
			break;

		case 0x2A:
		case 0x36:
			kb_status |= S_SHIFT;
			break;

		case 0x45:
			kb_status ^= S_NUM;
		}
	}
	else		//break
	{
		if (sch == 0xAA || sch == 0xB6)
			kb_status &= ~S_SHIFT;
	}

	if (ch >= 0x20 && ch < 0x7F)
	{
		DbgPrint("%C \n", ch);
	}

}

	VOID Unload(PDRIVER_OBJECT DriverObject)
	{
		KdPrint(("KeyBoardDriver: 驱动卸载\n"));

		// 从设备栈中移除创建的设备并删除

		PDEVICE_OBJECT FltDeviceObject = DriverObject->DeviceObject;

		while(FltDeviceObject != NULL)
		{
			PDEVICE_EXTENSION_FOR_KBD DeviceExtension = (PDEVICE_EXTENSION_FOR_KBD)FltDeviceObject->DeviceExtension;
			IoDetachDevice(DeviceExtension->LowerDeviceObject);

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

		DriverObject->MajorFunction[IRP_MJ_READ] = ReadDisPatch;

		// status = HookInstall();

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

	NTSTATUS ReadDisPatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
	{
		NTSTATUS status = STATUS_SUCCESS;

		PDEVICE_EXTENSION_FOR_KBD DeviceExtension = (PDEVICE_EXTENSION_FOR_KBD)DeviceObject->DeviceExtension;

		IoCopyCurrentIrpStackLocationToNext(Irp);
		IoSetCompletionRoutine(Irp, ReadCompletionRoutine, NULL, TRUE, TRUE, TRUE);

		status = IoCallDriver(DeviceExtension->LowerDeviceObject, Irp);

		return status;

	}

	NTSTATUS ReadCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
	{
		if(NT_SUCCESS(Irp->IoStatus.Status))
		{
			PVOID buf = Irp->AssociatedIrp.SystemBuffer;
			PKEYBOARD_INPUT_DATA data = (PKEYBOARD_INPUT_DATA)buf;
			ULONG dataLen = Irp->IoStatus.Information;

			ULONG keyWordNums = dataLen / sizeof(PKEYBOARD_INPUT_DATA);

			for(int i=0; i<keyWordNums; i++)
			{
				KdPrint((DBGSTRING"ScanCode: %x", data->MakeCode));
				KdPrint((DBGSTRING" %s\n", data->Flags ? "UP" : "DOWN"));
				print_keystroke((UCHAR)data->MakeCode);
			}
		}

		if(Irp->PendingReturned)
		{
			IoMarkIrpPending(Irp);
		}

		return Irp->IoStatus.Status;
	}

	NTSTATUS HookWriteDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
	{
		// 发现按键的时候并不走这里
		KdPrint((DBGSTRING"enter HookWriteDispatch!\n"));
		return OrignalWriteDispatch(DeviceObject, Irp);
	}

	NTSTATUS HookInstall()
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

		OrignalWriteDispatch = KbdDriverObject->MajorFunction[IRP_MJ_WRITE];

		InterlockedExchangePointer((PVOID *)&KbdDriverObject->MajorFunction[IRP_MJ_WRITE], HookWriteDispatch);
		
		return status;
	}

#ifdef __cplusplus
	}
#endif
