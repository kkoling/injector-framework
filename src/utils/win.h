#pragma once
#include <Windows.h>
#ifndef _NTDEF_
typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
typedef NTSTATUS* PNTSTATUS;
#endif

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001)
#define STATUS_INVALID_PAGE_PROTECTION ((NTSTATUS)0xC0000045)
#define STATUS_NOT_FOUND ((NTSTATUS)0xC0000225)
#define NtCurrentProcess() ((HANDLE)(ULONG64) - 1)
#define NtCurrentThread() ((HANDLE)(LONG_PTR) - 2)

#define InitializeObjectAttributes(p, n, a, r, s) \
  {                                               \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES);      \
    (p)->RootDirectory = r;                       \
    (p)->Attributes = a;                          \
    (p)->ObjectName = n;                          \
    (p)->SecurityDescriptor = s;                  \
    (p)->SecurityQualityOfService = NULL;         \
  }

#define OBJ_CASE_INSENSITIVE 0x00000040L

#define FILE_OPEN 0x00000001
#define FILE_OVERWRITE_IF 0x00000005

#define FILE_SYNCHRONOUS_IO_NONALERT 0x00000020
#define FILE_NON_DIRECTORY_FILE 0x00000040

#define RELOC_FLAG(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

typedef struct _PEB_LDR_DATA {
  ULONG Length;                                // 0x00
  BOOLEAN Initialized;                         // 0x04
  PVOID SsHandle;                              // 0x08
  LIST_ENTRY InLoadOrderModuleList;            // 0x0C (x86) / 0x10 (x64)
  LIST_ENTRY InMemoryOrderModuleList;          // 0x14 (x86) / 0x20 (x64)
  LIST_ENTRY InInitializationOrderModuleList;  // 0x1C (x86) / 0x30 (x64)
  VOID* EntryInProgress;                       // 0x40
  BOOLEAN ShutdownInProgress;                  // 0x48
  VOID* ShutdownThreadId;                      // 0x50
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _UNICODE_STRING {
  USHORT Length;                                                   // 0x0
  USHORT MaximumLength;                                            // 0x2
  _Field_size_bytes_part_opt_(MaximumLength, Length) PWCH Buffer;  // 0x8
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _LEAP_SECOND_DATA* PLEAP_SECOND_DATA;

typedef struct _CURDIR {
  UNICODE_STRING DosPath;
  HANDLE Handle;
} CURDIR, *PCURDIR;

typedef struct _STRING {
  USHORT Length;
  USHORT MaximumLength;
  _Field_size_bytes_part_opt_(MaximumLength, Length) PCHAR Buffer;
} STRING, *PSTRING, ANSI_STRING, *PANSI_STRING, OEM_STRING, *POEM_STRING;

typedef struct _RTL_DRIVE_LETTER_CURDIR {
  USHORT Flags;
  USHORT Length;
  ULONG TimeStamp;
  STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

#define RTL_MAX_DRIVE_LETTERS 32

typedef struct _RTL_USER_PROCESS_PARAMETERS {
  ULONG MaximumLength;
  ULONG Length;

  ULONG Flags;
  ULONG DebugFlags;

  HANDLE ConsoleHandle;
  ULONG ConsoleFlags;
  HANDLE StandardInput;
  HANDLE StandardOutput;
  HANDLE StandardError;

  CURDIR CurrentDirectory;
  UNICODE_STRING DllPath;
  UNICODE_STRING ImagePathName;
  UNICODE_STRING CommandLine;
  PVOID Environment;

  ULONG StartingX;
  ULONG StartingY;
  ULONG CountX;
  ULONG CountY;
  ULONG CountCharsX;
  ULONG CountCharsY;
  ULONG FillAttribute;

  ULONG WindowFlags;
  ULONG ShowWindowFlags;
  UNICODE_STRING WindowTitle;
  UNICODE_STRING DesktopInfo;
  UNICODE_STRING ShellInfo;
  UNICODE_STRING RuntimeData;
  RTL_DRIVE_LETTER_CURDIR CurrentDirectories[RTL_MAX_DRIVE_LETTERS];

  ULONG_PTR EnvironmentSize;
  ULONG_PTR EnvironmentVersion;

  PVOID PackageDependencyData;
  ULONG ProcessGroupId;
  ULONG LoaderThreads;
  UNICODE_STRING RedirectionDllName;  // REDSTONE4
  UNICODE_STRING HeapPartitionName;   // 19H1
  PULONGLONG DefaultThreadpoolCpuSetMasks;
  ULONG DefaultThreadpoolCpuSetMaskCount;
  ULONG DefaultThreadpoolThreadMaximum;
  ULONG HeapMemoryTypeMask;  // WIN11
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB {
  BOOLEAN InheritedAddressSpace;     // 0x00 (x86) / 0x00 (x64)
  BOOLEAN ReadImageFileExecOptions;  // 0x01 (x86) / 0x01 (x64)
  BOOLEAN BeingDebugged;             // 0x02 (x86) / 0x02 (x64)
  union {
    BOOLEAN BitField;  // 0x03 (x86) / 0x03 (x64)
    struct {
      BOOLEAN ImageUsesLargePages : 1;
      BOOLEAN IsProtectedProcess : 1;
      BOOLEAN IsImageDynamicallyRelocated : 1;
      BOOLEAN SkipPatchingUser32Forwarders : 1;
      BOOLEAN IsPackagedProcess : 1;
      BOOLEAN IsAppContainer : 1;
      BOOLEAN IsProtectedProcessLight : 1;
      BOOLEAN IsLongPathAwareProcess : 1;
    };
  };
  HANDLE Mutant;                                   // 0x04 (x86) / 0x04 (x64)
  PVOID ImageBaseAddress;                          // 0x08 (x86) / 0x08 (x64)
  PPEB_LDR_DATA Ldr;                               // 0x0C (x86) / 0x18 (x64)
  PRTL_USER_PROCESS_PARAMETERS ProcessParameters;  // 0x10 (x86) / 0x20 (x64)
  PVOID SubSystemData;                             // 0x14 (x86) / 0x28 (x64)
  PVOID ProcessHeap;                               // 0x18 (x86) / 0x30 (x64)
  PRTL_CRITICAL_SECTION FastPebLock;               // 0x1C (x86) / 0x38 (x64)
  PVOID AtlThunkSListPtr;                          // 0x20 (x86) / 0x40 (x64)
  PVOID IFEOKey;                                   // 0x24 (x86) / 0x48 (x64)
} PEB, *PPEB;

typedef struct _CLIENT_ID {
  HANDLE UniqueProcess;
  HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _ACTIVATION_CONTEXT {
  LONG RefCount;
  ULONG Flags;
} ACTIVATION_CONTEXT, *PACTIVATION_CONTEXT;

typedef struct _GDI_TEB_BATCH {
  ULONG Offset : 31;              // 0x0
  ULONG HasRenderingCommand : 1;  // 0x0
  ULONGLONG HDC;                  // 0x8
  ULONG Buffer[310];              // 0x10
} GDI_TEB_BATCH, *PGDI_TEB_BATCH;

typedef struct _TEB {
  NT_TIB NtTib;                         // Thread Information Block (TIB)
  PVOID EnvironmentPointer;             // Pointer to environment block
  CLIENT_ID ClientId;                   // Thread/Process IDs
  PVOID ActiveRpcHandle;                // Active RPC handle
  PVOID ThreadLocalStoragePointer;      // TLS array
  PPEB ProcessEnvironmentBlock;         // PEB pointer
  ULONG LastErrorValue;                 // Last error value
  ULONG CountOfOwnedCriticalSections;   // Count of critical sections owned
  PVOID CsrClientThread;                // CSR client thread
  PVOID Win32ThreadInfo;                // Win32 thread information (KTHREAD)
  ULONG User32Reserved[26];             // Reserved for User32
  ULONG UserReserved[5];                // Reserved for user
  PVOID WOW32Reserved;                  // WOW32 state
  LCID CurrentLocale;                   // Current locale
  ULONG FpSoftwareStatusRegister;       // Floating-point status
  PVOID SystemReserved1[54];            // Reserved for system
  LONG ExceptionCode;                   // Exception code
  ULONG ActivationContextStackPointer;  // Activation context stack
  ULONG_PTR InstrumentationCallbackSp;  // 0x2d0
  ULONG_PTR InstrumentationCallbackPreviousPc;  // 0x2d8
  ULONG_PTR InstrumentationCallbackPreviousSp;  // 0x2e0
  ULONG TxFsContext;                            // 0x2e8
  BOOLEAN InstrumentationCallbackDisabled;      // 0x2ec
  BOOLEAN UnalignedLoadStoreExceptions;         // 0x2ed
} TEB, *PTEB;

typedef _Function_class_(LDR_INIT_ROUTINE) BOOLEAN NTAPI
    LDR_INIT_ROUTINE(_In_ PVOID DllHandle, _In_ ULONG Reason,
                     _In_opt_ PVOID Context);

typedef LDR_INIT_ROUTINE* PLDR_INIT_ROUTINE;

typedef struct _LDR_DATA_TABLE_ENTRY {
  LIST_ENTRY InLoadOrderLinks;            // 0x0
  LIST_ENTRY InMemoryOrderLinks;          // 0x10
  LIST_ENTRY InInitializationOrderLinks;  // 0x20
  PVOID DllBase;                          // 0x30
  PLDR_INIT_ROUTINE EntryPoint;           // 0x38
  ULONG SizeOfImage;                      // 0x40
  UNICODE_STRING FullDllName;             // 0x48
  UNICODE_STRING BaseDllName;             // 0x58
  union {
    UCHAR FlagGroup[4];  // 0x68
    ULONG Flags;         // 0x68
    struct {
      ULONG PackagedBinary : 1;           // 0x68
      ULONG MarkedForRemoval : 1;         // 0x68
      ULONG ImageDll : 1;                 // 0x68
      ULONG LoadNotificationsSent : 1;    // 0x68
      ULONG TelemetryEntryProcessed : 1;  // 0x68
      ULONG ProcessStaticImport : 1;      // 0x68
      ULONG InLegacyLists : 1;            // 0x68
      ULONG InIndexes : 1;                // 0x68
      ULONG ShimDll : 1;                  // 0x68
      ULONG InExceptionTable : 1;         // 0x68
      ULONG ReservedFlags1 : 2;           // 0x68
      ULONG LoadInProgress : 1;           // 0x68
      ULONG LoadConfigProcessed : 1;      // 0x68
      ULONG EntryProcessed : 1;           // 0x68
      ULONG ProtectDelayLoad : 1;         // 0x68
      ULONG ReservedFlags3 : 2;           // 0x68
      ULONG DontCallForThreads : 1;       // 0x68
      ULONG ProcessAttachCalled : 1;      // 0x68
      ULONG ProcessAttachFailed : 1;      // 0x68
      ULONG CorDeferredValidate : 1;      // 0x68
      ULONG CorImage : 1;                 // 0x68
      ULONG DontRelocate : 1;             // 0x68
      ULONG CorILOnly : 1;                // 0x68
      ULONG ChpeImage : 1;                // 0x68
      ULONG ReservedFlags5 : 2;           // 0x68
      ULONG Redirected : 1;               // 0x68
      ULONG ReservedFlags6 : 2;           // 0x68
      ULONG CompatDatabaseProcessed : 1;  // 0x68
    };
  };
  USHORT ObsoleteLoadCount;                         // 0x6c
  USHORT TlsIndex;                                  // 0x6e
  LIST_ENTRY HashLinks;                             // 0x70
  ULONG TimeDateStamp;                              // 0x80
  PACTIVATION_CONTEXT EntryPointActivationContext;  // 0x88
  PVOID Lock;                                       // 0x90
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef enum _SYSTEM_INFORMATION_CLASS {
  SystemBasicInformation,                 // q: SYSTEM_BASIC_INFORMATION
  SystemProcessorInformation,             // q: SYSTEM_PROCESSOR_INFORMATION
  SystemPerformanceInformation,           // q: SYSTEM_PERFORMANCE_INFORMATION
  SystemTimeOfDayInformation,             // q: SYSTEM_TIMEOFDAY_INFORMATION
  SystemPathInformation,                  // not implemented
  SystemProcessInformation,               // q: SYSTEM_PROCESS_INFORMATION
  SystemCallCountInformation,             // q: SYSTEM_CALL_COUNT_INFORMATION
  SystemDeviceInformation,                // q: SYSTEM_DEVICE_INFORMATION
  SystemProcessorPerformanceInformation,  // q:
                                          // SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
                                          // (EX in: USHORT ProcessorGroup)
  SystemFlagsInformation,     // q: SYSTEM_FLAGS_INFORMATION
  SystemCallTimeInformation,  // not implemented // SYSTEM_CALL_TIME_INFORMATION
                              // // 10
  SystemModuleInformation,    // q: RTL_PROCESS_MODULES
  SystemLocksInformation,     // q: RTL_PROCESS_LOCKS
  SystemStackTraceInformation,    // q: RTL_PROCESS_BACKTRACES
  SystemPagedPoolInformation,     // not implemented
  SystemNonPagedPoolInformation,  // not implemented
  SystemHandleInformation,        // q: SYSTEM_HANDLE_INFORMATION
  SystemObjectInformation,        // q: SYSTEM_OBJECTTYPE_INFORMATION mixed with
                                  // SYSTEM_OBJECT_INFORMATION
  SystemPageFileInformation,      // q: SYSTEM_PAGEFILE_INFORMATION
  SystemVdmInstemulInformation,   // q: SYSTEM_VDM_INSTEMUL_INFO
  SystemVdmBopInformation,        // not implemented // 20
  SystemFileCacheInformation,  // q: SYSTEM_FILECACHE_INFORMATION; s (requires
                               // SeIncreaseQuotaPrivilege) (info for
                               // WorkingSetTypeSystemCache)
  SystemPoolTagInformation,    // q: SYSTEM_POOLTAG_INFORMATION
  SystemInterruptInformation,  // q: SYSTEM_INTERRUPT_INFORMATION (EX in: USHORT
                               // ProcessorGroup)
  SystemDpcBehaviorInformation,    // q: SYSTEM_DPC_BEHAVIOR_INFORMATION; s:
                                   // SYSTEM_DPC_BEHAVIOR_INFORMATION (requires
                                   // SeLoadDriverPrivilege)
  SystemFullMemoryInformation,     // not implemented //
                                   // SYSTEM_MEMORY_USAGE_INFORMATION
  SystemLoadGdiDriverInformation,  // s (kernel-mode only)
  SystemUnloadGdiDriverInformation,  // s (kernel-mode only)
  SystemTimeAdjustmentInformation,   // q: SYSTEM_QUERY_TIME_ADJUST_INFORMATION;
                                     // s: SYSTEM_SET_TIME_ADJUST_INFORMATION
                                     // (requires SeSystemtimePrivilege)
  SystemSummaryMemoryInformation,    // not implemented //
                                     // SYSTEM_MEMORY_USAGE_INFORMATION
  SystemMirrorMemoryInformation,     // s (requires license value
                                     // "Kernel-MemoryMirroringSupported")
                                     // (requires SeShutdownPrivilege) // 30
  SystemPerformanceTraceInformation,  // q; s: (type depends on
                                      // EVENT_TRACE_INFORMATION_CLASS)
  SystemObsolete0,                    // not implemented
  SystemExceptionInformation,         // q: SYSTEM_EXCEPTION_INFORMATION
  SystemCrashDumpStateInformation,    // s: SYSTEM_CRASH_DUMP_STATE_INFORMATION
                                      // (requires SeDebugPrivilege)
  SystemKernelDebuggerInformation,    // q: SYSTEM_KERNEL_DEBUGGER_INFORMATION
  SystemContextSwitchInformation,     // q: SYSTEM_CONTEXT_SWITCH_INFORMATION
  SystemRegistryQuotaInformation,     // q: SYSTEM_REGISTRY_QUOTA_INFORMATION; s
                                      // (requires SeIncreaseQuotaPrivilege)
  SystemExtendServiceTableInformation,  // s (requires SeLoadDriverPrivilege) //
                                        // loads win32k only
  SystemPrioritySeparation,             // s (requires SeTcbPrivilege)
  SystemVerifierAddDriverInformation,   // s: UNICODE_STRING (requires
                                        // SeDebugPrivilege) // 40
  SystemVerifierRemoveDriverInformation,  // s: UNICODE_STRING (requires
                                          // SeDebugPrivilege)
  SystemProcessorIdleInformation,    // q: SYSTEM_PROCESSOR_IDLE_INFORMATION (EX
                                     // in: USHORT ProcessorGroup)
  SystemLegacyDriverInformation,     // q: SYSTEM_LEGACY_DRIVER_INFORMATION
  SystemCurrentTimeZoneInformation,  // q; s: RTL_TIME_ZONE_INFORMATION
  SystemLookasideInformation,        // q: SYSTEM_LOOKASIDE_INFORMATION
  SystemTimeSlipNotification,        // s: HANDLE (NtCreateEvent) (requires
                                     // SeSystemtimePrivilege)
  SystemSessionCreate,               // not implemented
  SystemSessionDetach,               // not implemented
  SystemSessionInformation,     // not implemented (SYSTEM_SESSION_INFORMATION)
  SystemRangeStartInformation,  // q: SYSTEM_RANGE_START_INFORMATION // 50
  SystemVerifierInformation,    // q: SYSTEM_VERIFIER_INFORMATION; s (requires
                                // SeDebugPrivilege)
  SystemVerifierThunkExtend,    // s (kernel-mode only)
  SystemSessionProcessInformation,   // q: SYSTEM_SESSION_PROCESS_INFORMATION
  SystemLoadGdiDriverInSystemSpace,  // s: SYSTEM_GDI_DRIVER_INFORMATION
                                     // (kernel-mode only) (same as
                                     // SystemLoadGdiDriverInformation)
  SystemNumaProcessorMap,            // q: SYSTEM_NUMA_INFORMATION
  SystemPrefetcherInformation,       // q; s: PREFETCHER_INFORMATION //
                                     // PfSnQueryPrefetcherInformation
  SystemExtendedProcessInformation,  // q: SYSTEM_EXTENDED_PROCESS_INFORMATION
  SystemRecommendedSharedDataAlignment,  // q: ULONG //
                                         // KeGetRecommendedSharedDataAlignment
  SystemComPlusPackage,                  // q; s: ULONG
  SystemNumaAvailableMemory,             // q: SYSTEM_NUMA_INFORMATION // 60
  SystemProcessorPowerInformation,  // q: SYSTEM_PROCESSOR_POWER_INFORMATION (EX
                                    // in: USHORT ProcessorGroup)
  SystemEmulationBasicInformation,  // q: SYSTEM_BASIC_INFORMATION
  SystemEmulationProcessorInformation,  // q: SYSTEM_PROCESSOR_INFORMATION
  SystemExtendedHandleInformation,      // q: SYSTEM_HANDLE_INFORMATION_EX
  SystemLostDelayedWriteInformation,    // q: ULONG
} SYSTEM_INFORMATION_CLASS;

typedef const UNICODE_STRING* PCUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  HANDLE RootDirectory;
  PCUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;        // PSECURITY_DESCRIPTOR;
  PVOID SecurityQualityOfService;  // PSECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef const OBJECT_ATTRIBUTES* PCOBJECT_ATTRIBUTES;

typedef enum _PROCESSINFOCLASS {
  ProcessBasicInformation,  // q: PROCESS_BASIC_INFORMATION,
                            // PROCESS_EXTENDED_BASIC_INFORMATION
  ProcessQuotaLimits,       // qs: QUOTA_LIMITS, QUOTA_LIMITS_EX
  ProcessIoCounters,        // q: IO_COUNTERS
  ProcessVmCounters,        // q: VM_COUNTERS, VM_COUNTERS_EX, VM_COUNTERS_EX2
  ProcessTimes,             // q: KERNEL_USER_TIMES
  ProcessBasePriority,      // s: KPRIORITY
  ProcessRaisePriority,     // s: ULONG
  ProcessDebugPort,         // q: HANDLE
  ProcessExceptionPort,   // s: PROCESS_EXCEPTION_PORT (requires SeTcbPrivilege)
  ProcessAccessToken,     // s: PROCESS_ACCESS_TOKEN
  ProcessLdtInformation,  // qs: PROCESS_LDT_INFORMATION // 10
  ProcessLdtSize,         // s: PROCESS_LDT_SIZE
  ProcessDefaultHardErrorMode,  // qs: ULONG
  ProcessIoPortHandlers,        // s: PROCESS_IO_PORT_HANDLER_INFORMATION //
                                // (kernel-mode only)
  ProcessPooledUsageAndLimits,  // q: POOLED_USAGE_AND_LIMITS
  ProcessWorkingSetWatch,       // q: PROCESS_WS_WATCH_INFORMATION[]; s: void
  ProcessUserModeIOPL,          // qs: ULONG (requires SeTcbPrivilege)
  ProcessEnableAlignmentFaultFixup,  // s: BOOLEAN
  ProcessPriorityClass,              // qs: PROCESS_PRIORITY_CLASS
  ProcessWx86Information,  // qs: ULONG (requires SeTcbPrivilege) (VdmAllowed)
  ProcessHandleCount,      // q: ULONG, PROCESS_HANDLE_INFORMATION // 20
  ProcessAffinityMask,     // (q >WIN7)s: KAFFINITY, qs: GROUP_AFFINITY
  ProcessPriorityBoost,    // qs: ULONG
  ProcessDeviceMap,        // qs: PROCESS_DEVICEMAP_INFORMATION,
                           // PROCESS_DEVICEMAP_INFORMATION_EX
  ProcessSessionInformation,     // q: PROCESS_SESSION_INFORMATION
  ProcessForegroundInformation,  // s: PROCESS_FOREGROUND_BACKGROUND
  ProcessWow64Information,       // q: ULONG_PTR
  ProcessImageFileName,          // q: UNICODE_STRING
  ProcessLUIDDeviceMapsEnabled,  // q: ULONG
  ProcessBreakOnTermination,     // qs: ULONG
  ProcessDebugObjectHandle,      // q: HANDLE // 30
  ProcessDebugFlags,             // qs: ULONG
  ProcessHandleTracing,          // q: PROCESS_HANDLE_TRACING_QUERY; s:
                         // PROCESS_HANDLE_TRACING_ENABLE[_EX] or void to
                         // disable
  ProcessIoPriority,        // qs: IO_PRIORITY_HINT
  ProcessExecuteFlags,      // qs: ULONG (MEM_EXECUTE_OPTION_*)
  ProcessTlsInformation,    // PROCESS_TLS_INFORMATION //
                            // ProcessResourceManagement
  ProcessCookie,            // q: ULONG
  ProcessImageInformation,  // q: SECTION_IMAGE_INFORMATION
  ProcessCycleTime,         // q: PROCESS_CYCLE_TIME_INFORMATION // since VISTA
  ProcessPagePriority,      // qs: PAGE_PRIORITY_INFORMATION
  ProcessInstrumentationCallback,  // s: PVOID or
                                   // PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION
                                   // // 40
  ProcessThreadStackAllocation,  // s: PROCESS_STACK_ALLOCATION_INFORMATION,
                                 // PROCESS_STACK_ALLOCATION_INFORMATION_EX
  ProcessWorkingSetWatchEx,     // q: PROCESS_WS_WATCH_INFORMATION_EX[]; s: void
  ProcessImageFileNameWin32,    // q: UNICODE_STRING
  ProcessImageFileMapping,      // q: HANDLE (input)
  ProcessAffinityUpdateMode,    // qs: PROCESS_AFFINITY_UPDATE_MODE
  ProcessMemoryAllocationMode,  // qs: PROCESS_MEMORY_ALLOCATION_MODE
  ProcessGroupInformation,      // q: USHORT[]
  ProcessTokenVirtualizationEnabled,  // s: ULONG
  ProcessConsoleHostProcess,  // qs: ULONG_PTR // ProcessOwnerInformation
  ProcessWindowInformation,   // q: PROCESS_WINDOW_INFORMATION // 50
  ProcessHandleInformation,   // q: PROCESS_HANDLE_SNAPSHOT_INFORMATION // since
                              // WIN8
} PROCESSINFOCLASS;

typedef LONG KPRIORITY, *PKPRIORITY;

typedef enum _KTHREAD_STATE {
  Initialized,
  Ready,
  Running,
  Standby,
  Terminated,
  Waiting,
  Transition,
  DeferredReady,
  GateWaitObsolete,
  WaitingForProcessInSwap,
  MaximumThreadState
} KTHREAD_STATE,
    *PKTHREAD_STATE;

typedef enum _KWAIT_REASON {
  Executive,       // Waiting for an executive event.
  FreePage,        // Waiting for a free page.
  PageIn,          // Waiting for a page to be read in.
  PoolAllocation,  // Waiting for a pool allocation.
  DelayExecution,  // Waiting due to a delay execution.           //
                   // NtDelayExecution
  Suspended,  // Waiting because the thread is suspended.    // NtSuspendThread
  UserRequest,        // Waiting due to a user request.              //
                      // NtWaitForSingleObject
  WrExecutive,        // Waiting for an executive event.
  WrFreePage,         // Waiting for a free page.
  WrPageIn,           // Waiting for a page to be read in.
  WrPoolAllocation,   // Waiting for a pool allocation.
  WrDelayExecution,   // Waiting due to a delay execution.
  WrSuspended,        // Waiting because the thread is suspended.
  WrUserRequest,      // Waiting due to a user request.
  WrEventPair,        // Waiting for an event pair.                  //
                      // NtCreateEventPair
  WrQueue,            // Waiting for a queue.                        //
                      // NtRemoveIoCompletion
  WrLpcReceive,       // Waiting for an LPC receive.
  WrLpcReply,         // Waiting for an LPC reply.
  WrVirtualMemory,    // Waiting for virtual memory.
  WrPageOut,          // Waiting for a page to be written out.
  WrRendezvous,       // Waiting for a rendezvous.
  WrKeyedEvent,       // Waiting for a keyed event.                  //
                      // NtCreateKeyedEvent
  WrTerminated,       // Waiting for thread termination.
  WrProcessInSwap,    // Waiting for a process to be swapped in.
  WrCpuRateControl,   // Waiting for CPU rate control.
  WrCalloutStack,     // Waiting for a callout stack.
  WrKernel,           // Waiting for a kernel event.
  WrResource,         // Waiting for a resource.
  WrPushLock,         // Waiting for a push lock.
  WrMutex,            // Waiting for a mutex.
  WrQuantumEnd,       // Waiting for the end of a quantum.
  WrDispatchInt,      // Waiting for a dispatch interrupt.
  WrPreempted,        // Waiting because the thread was preempted.
  WrYieldExecution,   // Waiting to yield execution.
  WrFastMutex,        // Waiting for a fast mutex.
  WrGuardedMutex,     // Waiting for a guarded mutex.
  WrRundown,          // Waiting for a rundown.
  WrAlertByThreadId,  // Waiting for an alert by thread ID.
  WrDeferredPreempt,  // Waiting for a deferred preemption.
  WrPhysicalFault,    // Waiting for a physical fault.
  WrIoRing,           // Waiting for an I/O ring.
  WrMdlCache,         // Waiting for an MDL cache.
  WrRcu,              // Waiting for read-copy-update (RCU) synchronization.
  MaximumWaitReason
} KWAIT_REASON,
    *PKWAIT_REASON;

typedef struct _SYSTEM_THREAD_INFORMATION {
  LARGE_INTEGER KernelTime;  // Number of 100-nanosecond intervals spent
                             // executing kernel code.
  LARGE_INTEGER UserTime;  // Number of 100-nanosecond intervals spent executing
                           // user code.
  LARGE_INTEGER CreateTime;  // System time when the thread was created.
  ULONG WaitTime;      // Time spent in ready queue or waiting (depending on the
                       // thread state).
  PVOID StartAddress;  // Start address of the thread.
  CLIENT_ID ClientId;  // ID of the thread and the process owning the thread.
  KPRIORITY Priority;  // Dynamic thread priority.
  KPRIORITY BasePriority;     // Base thread priority.
  ULONG ContextSwitches;      // Total context switches.
  KTHREAD_STATE ThreadState;  // Current thread state.
  KWAIT_REASON WaitReason;    // The reason the thread is waiting.
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
  ULONG NextEntryOffset;  // The address of the previous item plus the value in
                          // the NextEntryOffset member. For the last item in
                          // the array, NextEntryOffset is 0.
  ULONG NumberOfThreads;  // The NumberOfThreads member contains the number of
                          // threads in the process.
  ULONGLONG WorkingSetPrivateSize;     // since VISTA
  ULONG HardFaultCount;                // since WIN7
  ULONG NumberOfThreadsHighWatermark;  // The peak number of threads that were
                                       // running at any given point in time,
                                       // indicative of potential performance
                                       // bottlenecks related to thread
                                       // management.
  ULONGLONG
  CycleTime;  // The sum of the cycle time of all threads in the process.
  LARGE_INTEGER CreateTime;  // Number of 100-nanosecond intervals since the
                             // creation time of the process. Not updated during
                             // system timezone changes.
  LARGE_INTEGER UserTime;
  LARGE_INTEGER KernelTime;
  UNICODE_STRING ImageName;  // The file name of the executable image.
  KPRIORITY BasePriority;
  HANDLE UniqueProcessId;
  HANDLE InheritedFromUniqueProcessId;
  ULONG HandleCount;
  ULONG SessionId;
  ULONG_PTR UniqueProcessKey;  // since VISTA (requires
                               // SystemExtendedProcessInformation)
  SIZE_T PeakVirtualSize;      // The peak size, in bytes, of the virtual memory
                               // used by the process.
  SIZE_T VirtualSize;  // The current size, in bytes, of virtual memory used by
                       // the process.
  ULONG PageFaultCount;       // The member of page faults for data that is not
                              // currently in memory.
  SIZE_T PeakWorkingSetSize;  // The peak size, in kilobytes, of the working set
                              // of the process.
  SIZE_T WorkingSetSize;      // The number of pages visible to the process in
                              // physical memory. These pages are resident and
  // available for use without triggering a page fault.
  SIZE_T QuotaPeakPagedPoolUsage;  // The peak quota charged to the process for
                                   // pool usage, in bytes.
  SIZE_T QuotaPagedPoolUsage;      // The quota charged to the process for paged
                                   // pool usage, in bytes.
  SIZE_T QuotaPeakNonPagedPoolUsage;  // The peak quota charged to the process
                                      // for nonpaged pool usage, in bytes.
  SIZE_T QuotaNonPagedPoolUsage;  // The current quota charged to the process
                                  // for nonpaged pool usage.
  SIZE_T PagefileUsage;  // The PagefileUsage member contains the number of
                         // bytes of page file storage in use by the process.
  SIZE_T PeakPagefileUsage;  // The maximum number of bytes of page-file storage
                             // used by the process.
  SIZE_T PrivatePageCount;   // The number of memory pages allocated for the use
                             // by the process.
  LARGE_INTEGER
  ReadOperationCount;  // The total number of read operations performed.
  LARGE_INTEGER
  WriteOperationCount;  // The total number of write operations performed.
  LARGE_INTEGER
  OtherOperationCount;  // The total number of I/O operations performed
                        // other than read and write operations.
  LARGE_INTEGER ReadTransferCount;   // The total number of bytes read during a
                                     // read operation.
  LARGE_INTEGER WriteTransferCount;  // The total number of bytes written during
                                     // a write operation.
  LARGE_INTEGER
  OtherTransferCount;  // The total number of bytes transferred during
                       // operations other than read and write operations.
  SYSTEM_THREAD_INFORMATION
  Threads[1];  // This type is not defined in the structure but was added
               // for convenience.
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _IO_STATUS_BLOCK {
  union {
    NTSTATUS Status;
    PVOID Pointer;
  };
  ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef _Function_class_(IO_APC_ROUTINE) VOID NTAPI
    IO_APC_ROUTINE(_In_ PVOID ApcContext, _In_ PIO_STATUS_BLOCK IoStatusBlock,
                   _In_ ULONG Reserved);

typedef IO_APC_ROUTINE* PIO_APC_ROUTINE;

typedef enum _SECTION_INHERIT { ViewShare = 1, ViewUnmap = 2 } SECTION_INHERIT;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO {
  USHORT UniqueProcessId;
  USHORT CreatorBackTraceIndex;
  UCHAR ObjectTypeIndex;
  UCHAR HandleAttributes;
  USHORT HandleValue;
  PVOID Object;
  ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION {
  ULONG NumberOfHandles;
  _Field_size_(NumberOfHandles) SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX {
  PVOID Object;
  HANDLE UniqueProcessId;
  HANDLE HandleValue;
  ACCESS_MASK GrantedAccess;
  USHORT CreatorBackTraceIndex;
  USHORT ObjectTypeIndex;
  ULONG HandleAttributes;
  ULONG Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX {
  ULONG_PTR NumberOfHandles;
  ULONG_PTR Reserved;
  _Field_size_(NumberOfHandles) SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

typedef struct _OBJECT_BASIC_INFORMATION {
  ULONG Attributes;
  ACCESS_MASK GrantedAccess;
  ULONG HandleCount;
  ULONG PointerCount;
  ULONG PagedPoolCharge;
  ULONG NonPagedPoolCharge;
  ULONG Reserved[3];
  ULONG NameInfoSize;
  ULONG TypeInfoSize;
  ULONG SecurityDescriptorSize;
  LARGE_INTEGER CreationTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_NAME_INFORMATION {
  UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION {
  UNICODE_STRING TypeName;
  ULONG TotalNumberOfObjects;
  ULONG TotalNumberOfHandles;
  ULONG TotalPagedPoolUsage;
  ULONG TotalNonPagedPoolUsage;
  ULONG TotalNamePoolUsage;
  ULONG TotalHandleTableUsage;
  ULONG HighWaterNumberOfObjects;
  ULONG HighWaterNumberOfHandles;
  ULONG HighWaterPagedPoolUsage;
  ULONG HighWaterNonPagedPoolUsage;
  ULONG HighWaterNamePoolUsage;
  ULONG HighWaterHandleTableUsage;
  ULONG InvalidAttributes;
  GENERIC_MAPPING GenericMapping;
  ULONG ValidAccessMask;
  BOOLEAN SecurityRequired;
  BOOLEAN MaintainHandleCount;
  UCHAR TypeIndex;  // since WINBLUE
  CHAR ReservedByte;
  ULONG PoolType;
  ULONG DefaultPagedPoolCharge;
  ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_TYPES_INFORMATION {
  ULONG NumberOfTypes;
} OBJECT_TYPES_INFORMATION, *POBJECT_TYPES_INFORMATION;

typedef struct _OBJECT_HANDLE_FLAG_INFORMATION {
  BOOLEAN Inherit;
  BOOLEAN ProtectFromClose;
} OBJECT_HANDLE_FLAG_INFORMATION, *POBJECT_HANDLE_FLAG_INFORMATION;

typedef enum _OBJECT_INFORMATION_CLASS {
  ObjectBasicInformation,       // q: OBJECT_BASIC_INFORMATION
  ObjectNameInformation,        // q: OBJECT_NAME_INFORMATION
  ObjectTypeInformation,        // q: OBJECT_TYPE_INFORMATION
  ObjectTypesInformation,       // q: OBJECT_TYPES_INFORMATION
  ObjectHandleFlagInformation,  // qs: OBJECT_HANDLE_FLAG_INFORMATION
  ObjectSessionInformation,     // s: void // change object session // (requires
                                // SeTcbPrivilege)
  ObjectSessionObjectInformation,  // s: void // change object session //
                                   // (requires SeTcbPrivilege)
  MaxObjectInfoClass
} OBJECT_INFORMATION_CLASS;

typedef struct _PROCESS_BASIC_INFORMATION {
  NTSTATUS ExitStatus;  // The exit status of the process. (GetExitCodeProcess)
  PPEB PebBaseAddress;  // A pointer to the process environment block (PEB) of
                        // the process.
  KAFFINITY AffinityMask;  // The affinity mask of the process.
                           // (GetProcessAffinityMask) (deprecated)
  KPRIORITY
  BasePriority;  // The base priority of the process. (GetPriorityClass)
  HANDLE
  UniqueProcessId;  // The unique identifier of the process. (GetProcessId)
  HANDLE InheritedFromUniqueProcessId;  // The unique identifier of the parent
                                        // process.
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

typedef struct _RTLP_CURDIR_REF {
  LONG ReferenceCount;
  HANDLE DirectoryHandle;
} RTLP_CURDIR_REF, *PRTLP_CURDIR_REF;

typedef struct _RTL_RELATIVE_NAME_U {
  UNICODE_STRING RelativeName;
  HANDLE ContainingDirectory;
  PRTLP_CURDIR_REF CurDirRef;
} RTL_RELATIVE_NAME_U, *PRTL_RELATIVE_NAME_U;

typedef _Function_class_(USER_THREAD_START_ROUTINE) NTSTATUS NTAPI
    USER_THREAD_START_ROUTINE(_In_ PVOID ThreadParameter);

typedef USER_THREAD_START_ROUTINE* PUSER_THREAD_START_ROUTINE;

typedef struct _PS_ATTRIBUTE {
  ULONG_PTR Attribute;
  SIZE_T Size;
  union {
    ULONG_PTR Value;
    PVOID ValuePtr;
  };
  PSIZE_T ReturnLength;
} PS_ATTRIBUTE, *PPS_ATTRIBUTE;

typedef struct _PS_ATTRIBUTE_LIST {
  SIZE_T TotalLength;
  PS_ATTRIBUTE Attributes[1];
} PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;

typedef _Function_class_(PS_APC_ROUTINE) VOID NTAPI
    PS_APC_ROUTINE(_In_opt_ PVOID ApcArgument1, _In_opt_ PVOID ApcArgument2,
                   _In_opt_ PVOID ApcArgument3);

typedef PS_APC_ROUTINE* PPS_APC_ROUTINE;

typedef enum _THREADINFOCLASS {
  ThreadBasicInformation,  // q: THREAD_BASIC_INFORMATION
  ThreadTimes,             // q: KERNEL_USER_TIMES
  ThreadPriority,  // s: KPRIORITY (requires SeIncreaseBasePriorityPrivilege)
  ThreadBasePriority,               // s: KPRIORITY
  ThreadAffinityMask,               // s: KAFFINITY
  ThreadImpersonationToken,         // s: HANDLE
  ThreadDescriptorTableEntry,       // q: DESCRIPTOR_TABLE_ENTRY (or
                                    // WOW64_DESCRIPTOR_TABLE_ENTRY)
  ThreadEnableAlignmentFaultFixup,  // s: BOOLEAN
  ThreadEventPair,                  // Obsolete
  ThreadQuerySetWin32StartAddress,  // qs: PVOID (requires
                                    // THREAD_Set_LIMITED_INFORMATION)
  ThreadZeroTlsCell,                // s: ULONG // TlsIndex // 10
  ThreadPerformanceCount,           // q: LARGE_INTEGER
  ThreadAmILastThread,              // q: ULONG
  ThreadIdealProcessor,             // s: ULONG
  ThreadPriorityBoost,              // qs: ULONG
  ThreadSetTlsArrayAddress,         // s: ULONG_PTR
  ThreadIsIoPending,                // q: ULONG
  ThreadHideFromDebugger,           // q: BOOLEAN; s: void
  ThreadBreakOnTermination,         // qs: ULONG
  ThreadSwitchLegacyState,          // s: void // NtCurrentThread // NPX/FPU
  ThreadIsTerminated,               // q: ULONG // 20
  ThreadLastSystemCall,             // q: THREAD_LAST_SYSCALL_INFORMATION
  ThreadIoPriority,                 // qs: IO_PRIORITY_HINT (requires
                                    // SeIncreaseBasePriorityPrivilege)
  ThreadCycleTime,           // q: THREAD_CYCLE_TIME_INFORMATION (requires
                             // THREAD_QUERY_LIMITED_INFORMATION)
  ThreadPagePriority,        // qs: PAGE_PRIORITY_INFORMATION
  ThreadActualBasePriority,  // s: LONG (requires
                             // SeIncreaseBasePriorityPrivilege)
  ThreadTebInformation,      // q: THREAD_TEB_INFORMATION (requires
                             // THREAD_GET_CONTEXT + THREAD_SET_CONTEXT)
  ThreadCSwitchMon,          // Obsolete
  ThreadCSwitchPmu,          // Obsolete
  ThreadWow64Context,        // qs: WOW64_CONTEXT, ARM_NT_CONTEXT since 20H1
  ThreadGroupInformation,    // qs: GROUP_AFFINITY // 30
  ThreadUmsInformation,      // q: THREAD_UMS_INFORMATION // Obsolete
  ThreadCounterProfiling,    // q: BOOLEAN; s: THREAD_PROFILING_INFORMATION?
  ThreadIdealProcessorEx,  // qs: PROCESSOR_NUMBER; s: previous PROCESSOR_NUMBER
                           // on return
  ThreadCpuAccountingInformation,  // q: BOOLEAN; s: HANDLE (NtOpenSession) //
                                   // NtCurrentThread // since WIN8
  ThreadSuspendCount,              // q: ULONG // since WINBLUE
  ThreadHeterogeneousCpuPolicy,    // q: KHETERO_CPU_POLICY // since THRESHOLD
  ThreadContainerId,               // q: GUID
  ThreadNameInformation,           // qs: THREAD_NAME_INFORMATION (requires
                                   // THREAD_SET_LIMITED_INFORMATION)
  ThreadSelectedCpuSets,           // q: ULONG[]
  ThreadSystemThreadInformation,   // q: SYSTEM_THREAD_INFORMATION // 40
  ThreadActualGroupAffinity,       // q: GROUP_AFFINITY // since THRESHOLD2
  ThreadDynamicCodePolicyInfo,     // q: ULONG; s: ULONG (NtCurrentThread)
  ThreadExplicitCaseSensitivity,  // qs: ULONG; s: 0 disables, otherwise enables
                                  // // (requires SeDebugPrivilege and
                                  // PsProtectedSignerAntimalware)
  ThreadWorkOnBehalfTicket,       // ALPC_WORK_ON_BEHALF_TICKET //
                             // RTL_WORK_ON_BEHALF_TICKET_EX // NtCurrentThread
  ThreadSubsystemInformation,  // q: SUBSYSTEM_INFORMATION_TYPE // since
                               // REDSTONE2
  ThreadDbgkWerReportActive,   // s: ULONG; s: 0 disables, otherwise enables
  ThreadAttachContainer,       // s: HANDLE (job object) // NtCurrentThread
  ThreadManageWritesToExecutableMemory,  // MANAGE_WRITES_TO_EXECUTABLE_MEMORY
                                         // // since REDSTONE3
  ThreadPowerThrottlingState,  // qs: POWER_THROTTLING_THREAD_STATE // since
                               // REDSTONE3 (set), WIN11 22H2 (query)
  ThreadWorkloadClass,         // THREAD_WORKLOAD_CLASS // since REDSTONE5 // 50
  ThreadCreateStateChange,     // since WIN11
  ThreadApplyStateChange,
  ThreadStrongerBadHandleChecks,  // s: ULONG // NtCurrentThread // since 22H1
  ThreadEffectiveIoPriority,      // q: IO_PRIORITY_HINT
  ThreadEffectivePagePriority,    // q: ULONG
  ThreadUpdateLockOwnership,      // THREAD_LOCK_OWNERSHIP // since 24H2
  ThreadSchedulerSharedDataSlot,  // SCHEDULER_SHARED_DATA_SLOT_INFORMATION
  ThreadTebInformationAtomic,     // q: THREAD_TEB_INFORMATION (requires
                               // THREAD_GET_CONTEXT + THREAD_QUERY_INFORMATION)
  ThreadIndexInformation,  // THREAD_INDEX_INFORMATION
  MaxThreadInfoClass
} THREADINFOCLASS;

typedef struct _PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION {
  ULONG Version;   // The version of the instrumentation callback information.
  ULONG Reserved;  // Reserved for future use.
  PVOID Callback;  // Pointer to the callback function.
} PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION,
    *PPROCESS_INSTRUMENTATION_CALLBACK_INFORMATION;

typedef enum _MEMORY_INFORMATION_CLASS {
  MemoryBasicInformation,            // q: MEMORY_BASIC_INFORMATION
  MemoryWorkingSetInformation,       // q: MEMORY_WORKING_SET_INFORMATION
  MemoryMappedFilenameInformation,   // q: UNICODE_STRING
  MemoryRegionInformation,           // q: MEMORY_REGION_INFORMATION
  MemoryWorkingSetExInformation,     // q: MEMORY_WORKING_SET_EX_INFORMATION //
                                     // since VISTA
  MemorySharedCommitInformation,     // q: MEMORY_SHARED_COMMIT_INFORMATION //
                                     // since WIN8
  MemoryImageInformation,            // q: MEMORY_IMAGE_INFORMATION
  MemoryRegionInformationEx,         // MEMORY_REGION_INFORMATION
  MemoryPrivilegedBasicInformation,  // MEMORY_BASIC_INFORMATION
  MemoryEnclaveImageInformation,  // MEMORY_ENCLAVE_IMAGE_INFORMATION // since
                                  // REDSTONE3
  MemoryBasicInformationCapped,   // 10
  MemoryPhysicalContiguityInformation,  // MEMORY_PHYSICAL_CONTIGUITY_INFORMATION
                                        // // since 20H1
  MemoryBadInformation,              // since WIN11
  MemoryBadInformationAllProcesses,  // since 22H1
  MemoryImageExtensionInformation,   // MEMORY_IMAGE_EXTENSION_INFORMATION //
                                     // since 24H2
  MaxMemoryInfoClass
} MEMORY_INFORMATION_CLASS;

typedef enum _FILE_INFORMATION_CLASS {
  FileDirectoryInformation =
      1,  // q: FILE_DIRECTORY_INFORMATION (requires FILE_LIST_DIRECTORY)
          // (NtQueryDirectoryFile[Ex])
  FileFullDirectoryInformation,  // q: FILE_FULL_DIR_INFORMATION (requires
                                 // FILE_LIST_DIRECTORY)
                                 // (NtQueryDirectoryFile[Ex])
  FileBothDirectoryInformation,  // q: FILE_BOTH_DIR_INFORMATION (requires
                                 // FILE_LIST_DIRECTORY)
                                 // (NtQueryDirectoryFile[Ex])
  FileBasicInformation,          // qs: FILE_BASIC_INFORMATION (q: requires
                                 // FILE_READ_ATTRIBUTES; s: requires
                                 // FILE_WRITE_ATTRIBUTES)
  FileStandardInformation,       // q: FILE_STANDARD_INFORMATION,
                                 // FILE_STANDARD_INFORMATION_EX
  FileInternalInformation,       // q: FILE_INTERNAL_INFORMATION
} FILE_INFORMATION_CLASS,
    *PFILE_INFORMATION_CLASS;

typedef ULONG LOGICAL;

typedef struct _FILE_STANDARD_INFORMATION {
  LARGE_INTEGER
  AllocationSize;  // The file allocation size in bytes. Usually, this value
                   // is a multiple of the sector or cluster size of the
                   // underlying physical device.
  LARGE_INTEGER EndOfFile;  // The end of file location as a byte offset.
  ULONG NumberOfLinks;      // The number of hard links to the file.
  BOOLEAN DeletePending;    // The delete pending status. TRUE indicates that a
                            // file deletion has been requested.
  BOOLEAN Directory;  // The file directory status. TRUE indicates the file
                      // object represents a directory.
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;
