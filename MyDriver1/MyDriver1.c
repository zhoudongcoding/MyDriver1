#include <ntifs.h>
#include <wdmsec.h>

#define DEVICE_NAME L"\\Device\\MyFirstDevice"
#define SYM_NAME L"\\??\\MyFirstDevice"

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

NTSTATUS MyCreate(IN PDEVICE_OBJECT dev, IN PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(77,0,"My device has be created!\n");

	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = status;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MyClose(IN PDEVICE_OBJECT dev, IN PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(77, 0, "My device has be closed!\n");

	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = status;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MyCleanup(IN PDEVICE_OBJECT dev, IN PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(77, 0, "My device has be cleanup!\n");

	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = status;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

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

	__debugbreak();

	driver->DriverUnload = DrvUnload;

	// 创建设备
	RtlInitUnicodeString(&device_name, DEVICE_NAME);
	status = IoCreateDevice(driver, 0, &device_name, FILE_DEVICE_UNKNOWN, 0, TRUE, &pdevice);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Create device failed: %x\n", status);

		return status;
	}

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

	return STATUS_SUCCESS;
}