#include <ntifs.h>
#include <wdmsec.h>

#define DEVICE_NAME L"\\Device\\MyFirstDevice"
#define SYM_NAME L"\\??\\MyFirstDevice"

#define IOCTL_MUL CTL_CODE(FILE_DEVICE_UNKNOWN, 0X9888, METHOD_BUFFERED, FILE_ANY_ACCESS)
char g_DeviceExtension[0x200] = { 0 };
void DrvUnload(PDRIVER_OBJECT pdriver)
{
	DbgPrint("Unload..\n");

	if (pdriver->DeviceObject)
	{
		IoDeleteDevice(pdriver->DeviceObject);
	}

	UNICODE_STRING sym_name = { 0 };
	RtlInitUnicodeString(&sym_name, SYM_NAME);
	IoDeleteSymbolicLink(&sym_name);

}

NTSTATUS MyCreate(IN PDEVICE_OBJECT dev, IN PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(77,0,"My device has be created!\n");

	pirp->IoStatus.Information = 0;
	pirp->IoStatus.Status = status;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MyClose(IN PDEVICE_OBJECT dev, IN PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(77, 0, "My device has be closed!\n");

	pirp->IoStatus.Information = 0;
	pirp->IoStatus.Status = status;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MyCleanup(IN PDEVICE_OBJECT dev, IN PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(77, 0, "My device has be cleanup!\n");

	pirp->IoStatus.Information = 0;
	pirp->IoStatus.Status = status;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MyRead(IN PDEVICE_OBJECT dev, IN PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(77, 0, "My device has be Readed!\n");

	PIO_STACK_LOCATION pstack = IoGetCurrentIrpStackLocation(pirp);
	ULONG readlen = pstack->Parameters.Read.Length;
	PCHAR read_buff = pirp->AssociatedIrp.SystemBuffer;

	RtlCopyMemory(read_buff, "This is a kernel message.", strlen("This is a kernel message."));

	pirp->IoStatus.Information = strlen("This is a kernel message.");
	pirp->IoStatus.Status = status;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	DbgPrintEx(77, 0, "Readed succeed!\n");

	return STATUS_SUCCESS;
}

NTSTATUS MyWrite(IN PDEVICE_OBJECT dev, IN PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(77, 0, "My device has be Writed!\n");

	__debugbreak();

	PIO_STACK_LOCATION pstack = IoGetCurrentIrpStackLocation(pirp);
	ULONG writelen = pstack->Parameters.Write.Length;
	PCHAR write_buff = pirp->AssociatedIrp.SystemBuffer;
	dev->DeviceExtension = &g_DeviceExtension;
	RtlZeroMemory(dev->DeviceExtension, 200);      
	RtlCopyMemory(dev->DeviceExtension, write_buff, writelen);
	DbgPrintEx(77, 0, "%s\n", (PCHAR)dev->DeviceExtension);

	pirp->IoStatus.Information = 13;
	pirp->IoStatus.Status = status;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	DbgPrintEx(77, 0, "Write succeed!\n");

	return STATUS_SUCCESS;
}

NTSTATUS MyControl (IN PDEVICE_OBJECT dev, IN PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(77, 0, "My device has be control!\n");

	PIO_STACK_LOCATION pstack = IoGetCurrentIrpStackLocation(pirp);

	ULONG iocode = pstack->Parameters.DeviceIoControl.IoControlCode;	
	ULONG inlen = pstack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG outlen = pstack->Parameters.DeviceIoControl.OutputBufferLength;

	ULONG ioinfo = 0;

	switch(iocode){

	case IOCTL_MUL:
	{
		DWORD32 indata = *(PDWORD32)pirp->AssociatedIrp.SystemBuffer;

		DbgPrintEx(77, 0, "indata: %d\n", indata);

		indata = indata * 5;
		*(PDWORD64)pirp->AssociatedIrp.SystemBuffer = indata;

		ioinfo = 4;
		break;
	}
		
	default:
		status = STATUS_UNSUCCESSFUL;
		ioinfo = 0;
		break;
	}

	pirp->IoStatus.Information = ioinfo;
	pirp->IoStatus.Status = status;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	DbgPrintEx(77, 0, "IoCtl succeed!\n");

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(
	__in PDRIVER_OBJECT driver,
	__in PUNICODE_STRING reg_path
)
{
	NTSTATUS status;

	UNICODE_STRING device_name = { 0 };
	PDEVICE_OBJECT pdevice = NULL;


	driver->DriverUnload = DrvUnload;

	// 创建设备
	RtlInitUnicodeString(&device_name, DEVICE_NAME);
	status = IoCreateDevice(driver, 0, &device_name, FILE_DEVICE_UNKNOWN, 0, TRUE, &pdevice);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Create device failed: %x\n", status);

		return status;
	}

	pdevice->Flags |= DO_BUFFERED_IO;

	// 创建设备成功，创建符号连接
	UNICODE_STRING sym_name = { 0 };
	RtlInitUnicodeString(&sym_name, SYM_NAME);
	status = IoCreateSymbolicLink(&sym_name, &device_name);
	if (!NT_SUCCESS(status))
	{

		DbgPrint("Create symbol failed: %x\n", status);

		IoDeleteDevice(pdevice);

		return status;
	}

	driver->MajorFunction[IRP_MJ_CREATE] = MyCreate;

	driver->MajorFunction[IRP_MJ_CLOSE] = MyClose;

	driver->MajorFunction[IRP_MJ_CLEANUP] = MyCleanup;

	driver->MajorFunction[IRP_MJ_READ] = MyRead;

	driver->MajorFunction[IRP_MJ_WRITE] = MyWrite;

	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MyControl;

	return STATUS_SUCCESS;
}