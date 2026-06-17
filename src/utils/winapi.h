#pragma once
#include "win.h"

typedef NTSTATUS(NTAPI* NtResumeThread)(_In_ HANDLE ThreadHandle,
                                        _Out_opt_ PULONG PreviousSuspendCount);

typedef NTSTATUS(NTAPI* NtSetContextThread)(_In_ HANDLE ThreadHandle,
                                            _In_ PCONTEXT ThreadContext);

typedef NTSTATUS(NTAPI* NtGetContextThread)(_In_ HANDLE ThreadHandle,
                                            _Inout_ PCONTEXT ThreadContext);

typedef NTSTATUS(NTAPI* NtSuspendThread)(_In_ HANDLE ThreadHandle,
                                         _Out_opt_ PULONG PreviousSuspendCount);

typedef NTSTATUS(NTAPI* NtOpenThread)(_Out_ PHANDLE ThreadHandle,
                                      _In_ ACCESS_MASK DesiredAccess,
                                      _In_ PCOBJECT_ATTRIBUTES ObjectAttributes,
                                      _In_opt_ PCLIENT_ID ClientId);

typedef NTSTATUS(NTAPI* NtCreateThreadEx)(
    _Out_ PHANDLE ThreadHandle, _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ PCOBJECT_ATTRIBUTES ObjectAttributes, _In_ HANDLE ProcessHandle,
    _In_ PUSER_THREAD_START_ROUTINE StartRoutine, _In_opt_ PVOID Argument,
    _In_ ULONG CreateFlags,  // THREAD_CREATE_FLAGS_*
    _In_ SIZE_T ZeroBits, _In_ SIZE_T StackSize, _In_ SIZE_T MaximumStackSize,
    _In_opt_ PPS_ATTRIBUTE_LIST AttributeList);

typedef NTSTATUS(NTAPI* NtCreateFile)(
    _Out_ PHANDLE FileHandle, _In_ ACCESS_MASK DesiredAccess,
    _In_ PCOBJECT_ATTRIBUTES ObjectAttributes,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_opt_ PLARGE_INTEGER AllocationSize, _In_ ULONG FileAttributes,
    _In_ ULONG ShareAccess, _In_ ULONG CreateDisposition,
    _In_ ULONG CreateOptions, _In_reads_bytes_opt_(EaLength) PVOID EaBuffer,
    _In_ ULONG EaLength);

typedef NTSTATUS(NTAPI* NtQueryInformationFile)(
    _In_ HANDLE FileHandle, _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _Out_writes_bytes_(Length) PVOID FileInformation, _In_ ULONG Length,
    _In_ FILE_INFORMATION_CLASS FileInformationClass);

typedef NTSTATUS(NTAPI* NtReadFile)(
    _In_ HANDLE FileHandle, _In_opt_ HANDLE Event,
    _In_opt_ PIO_APC_ROUTINE ApcRoutine, _In_opt_ PVOID ApcContext,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _Out_writes_bytes_(Length) PVOID Buffer, _In_ ULONG Length,
    _In_opt_ PLARGE_INTEGER ByteOffset, _In_opt_ PULONG Key);

typedef NTSTATUS(NTAPI* NtDuplicateObject)(_In_ HANDLE SourceProcessHandle,
                                           _In_ HANDLE SourceHandle,
                                           _In_opt_ HANDLE TargetProcessHandle,
                                           _Out_opt_ PHANDLE TargetHandle,
                                           _In_ ACCESS_MASK DesiredAccess,
                                           _In_ ULONG HandleAttributes,
                                           _In_ ULONG Options);

typedef NTSTATUS(NTAPI* NtQuerySystemInformation)(
    _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _Out_writes_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
    _In_ ULONG SystemInformationLength, _Out_opt_ PULONG ReturnLength);

typedef NTSTATUS(NTAPI* NtQueryInformationProcess)(
    _In_ HANDLE ProcessHandle, _In_ PROCESSINFOCLASS ProcessInformationClass,
    _Out_writes_bytes_(ProcessInformationLength) PVOID ProcessInformation,
    _In_ ULONG ProcessInformationLength, _Out_opt_ PULONG ReturnLength);

typedef NTSTATUS(NTAPI* NtAllocateVirtualMemory)(
    _In_ HANDLE ProcessHandle,
    _Inout_ _At_(*BaseAddress,
                 _Readable_bytes_(*RegionSize) _Writable_bytes_(*RegionSize)
                     _Post_readable_byte_size_(*RegionSize)) PVOID* BaseAddress,
    _In_ ULONG_PTR ZeroBits, _Inout_ PSIZE_T RegionSize,
    _In_ ULONG AllocationType, _In_ ULONG PageProtection);

typedef NTSTATUS(NTAPI* NtFreeVirtualMemory)(_In_ HANDLE ProcessHandle,
                                             _Inout_ __drv_freesMem(Mem)
                                                 PVOID* BaseAddress,
                                             _Inout_ PSIZE_T RegionSize,
                                             _In_ ULONG FreeType);

typedef NTSTATUS(NTAPI* NtOpenProcess)(
    _Out_ PHANDLE ProcessHandle, _In_ ACCESS_MASK DesiredAccess,
    _In_ PCOBJECT_ATTRIBUTES ObjectAttributes, _In_opt_ PCLIENT_ID ClientId);

typedef NTSTATUS(NTAPI* NtClose)(_In_ _Post_ptr_invalid_ HANDLE Handle);

typedef NTSTATUS(NTAPI* NtReadVirtualMemory)(
    _In_ HANDLE ProcessHandle, _In_opt_ PVOID BaseAddress,
    _Out_writes_bytes_to_(NumberOfBytesToRead, *NumberOfBytesRead) PVOID Buffer,
    _In_ SIZE_T NumberOfBytesToRead, _Out_opt_ PSIZE_T NumberOfBytesRead);

typedef NTSTATUS(NTAPI* NtWriteVirtualMemory)(
    _In_ HANDLE ProcessHandle, _In_opt_ PVOID BaseAddress,
    _In_reads_bytes_(NumberOfBytesToWrite) PVOID Buffer,
    _In_ SIZE_T NumberOfBytesToWrite, _Out_opt_ PSIZE_T NumberOfBytesWritten);

typedef NTSTATUS(NTAPI* NtQueryVirtualMemory)(
    _In_ HANDLE ProcessHandle, _In_opt_ PVOID BaseAddress,
    _In_ MEMORY_INFORMATION_CLASS MemoryInformationClass,
    _Out_writes_bytes_(MemoryInformationLength) PVOID MemoryInformation,
    _In_ SIZE_T MemoryInformationLength, _Out_opt_ PSIZE_T ReturnLength);

typedef NTSTATUS(NTAPI* NtProtectVirtualMemory)(_In_ HANDLE ProcessHandle,
                                                _Inout_ PVOID* BaseAddress,
                                                _Inout_ PSIZE_T RegionSize,
                                                _In_ ULONG NewProtection,
                                                _Out_ PULONG OldProtection);

typedef NTSTATUS(NTAPI* TpAllocWork)(
    _Out_ PTP_WORK* WorkReturn, _In_ PTP_WORK_CALLBACK Callback,
    _Inout_opt_ PVOID Context, _In_opt_ PTP_CALLBACK_ENVIRON CallbackEnviron);

typedef VOID(NTAPI* TpPostWork)(_Inout_ PTP_WORK Work);

typedef VOID(NTAPI* TpWaitForWork)(_Inout_ PTP_WORK Work,
                                   _In_ LOGICAL CancelPendingCallbacks);

typedef VOID(NTAPI* TpReleaseWork)(_Inout_ PTP_WORK Work);

typedef NTSTATUS(NTAPI* LdrLoadDll)(_In_opt_ PCWSTR DllPath,
                                    _In_opt_ PULONG DllCharacteristics,
                                    _In_ PCUNICODE_STRING DllName,
                                    _Out_ PVOID* DllHandle);

typedef NTSTATUS(NTAPI* LdrUnloadDll)(_In_ PVOID DllHandle);

typedef PVOID(NTAPI* RtlAddVectoredExceptionHandler)(
    _In_ ULONG First, _In_ PVECTORED_EXCEPTION_HANDLER Handler);

typedef ULONG(NTAPI* RtlRemoveVectoredExceptionHandler)(_In_ PVOID Handle);

typedef NTSTATUS(NTAPI* RtlDosPathNameToNtPathName_U_WithStatus)(
    _In_ PCWSTR DosFileName, _Out_ PUNICODE_STRING NtFileName,
    _Out_opt_ PWSTR* FilePart, _Out_opt_ PRTL_RELATIVE_NAME_U RelativeName);

typedef BOOLEAN(NTAPI* RtlDosPathNameToRelativeNtPathName_U)(
    _In_ PCWSTR DosFileName, _Out_ PUNICODE_STRING NtFileName,
    _Out_opt_ PWSTR* FilePart, _Out_opt_ PRTL_RELATIVE_NAME_U RelativeName);
