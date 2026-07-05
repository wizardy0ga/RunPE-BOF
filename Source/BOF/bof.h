# include <windows.h>
# include "beacon.h"

//# define DEBUG

# ifdef DEBUG
# define DBG( msg, ... ) BeaconPrintf( CALLBACK_OUTPUT, msg, ##__VA_ARGS__ );
# else 
# define DBG( msg, ... ) do {} while(0);
# endif

# define MSG( msg, ... ) BeaconPrintf( CALLBACK_OUTPUT, msg, ##__VA_ARGS__ );

DECLSPEC_IMPORT BOOL   Kernel32$GetLastError();

# define ERR( api )             BeaconPrintf( CALLBACK_ERROR,"%s failed with error: %d", api, Kernel32$GetLastError() );
# define NTERR( api, status )   BeaconPrintf( CALLBACK_ERROR, "%s failed with error: 0x%0.8X", api, status );

/* Kernel32 Functions */

DECLSPEC_IMPORT HANDLE Kernel32$GetProcessHeap();

DECLSPEC_IMPORT VOID Kernel32$DeleteProcThreadAttributeList( LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList );

DECLSPEC_IMPORT HANDLE Kernel32$OpenProcess(
    DWORD dwDesiredAccess,
    BOOL  bInheritHandle,
    DWORD dwProcessId
);

DECLSPEC_IMPORT BOOL Kernel32$CreateProcessA(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);

DECLSPEC_IMPORT BOOL Kernel32$InitializeProcThreadAttributeList(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD                        dwAttributeCount,
    DWORD                        dwFlags,
    PSIZE_T                      lpSize
);

DECLSPEC_IMPORT BOOL Kernel32$UpdateProcThreadAttribute(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD                        dwFlags,
    DWORD_PTR                    Attribute,
    PVOID                        lpValue,
    SIZE_T                       cbSize,
    PVOID                        lpPreviousValue,
    PSIZE_T                      lpReturnSize
);

DECLSPEC_IMPORT LPVOID Kernel32$HeapAlloc(
    HANDLE hHeap,
    DWORD  dwFlags,
    SIZE_T dwBytes
);

DECLSPEC_IMPORT BOOL Kernel32$HeapFree(
    HANDLE hHeap,
    DWORD  dwFlags,
    LPVOID lpMem
);

/* Ntdll Functions */
DECLSPEC_IMPORT NTSTATUS Ntdll$NtClose( HANDLE Handle );

DECLSPEC_IMPORT NTSTATUS Ntdll$NtAllocateVirtualMemory(
    HANDLE ProcessHandle,
    PVOID *BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG PageProtection
);

DECLSPEC_IMPORT NTSTATUS Ntdll$NtWriteVirtualMemory(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T NumberOfBytesToWrite,
    PSIZE_T NumberOfBytesWritten
);

DECLSPEC_IMPORT NTSTATUS Ntdll$NtFreeVirtualMemory(
    HANDLE  ProcessHandle,
    PVOID   *BaseAddress,
    PSIZE_T RegionSize,
    ULONG   FreeType
);

DECLSPEC_IMPORT NTSTATUS Ntdll$NtProtectVirtualMemory(
    HANDLE ProcessHandle,
    PVOID *BaseAddress,
    PSIZE_T RegionSize,
    ULONG NewProtection,
    PULONG OldProtection
);

DECLSPEC_IMPORT NTSTATUS Ntdll$NtGetContextThread(
    HANDLE ThreadHandle,
    PCONTEXT ThreadContext
);

DECLSPEC_IMPORT NTSTATUS Ntdll$NtSetContextThread(
    HANDLE ThreadHandle,
    PCONTEXT ThreadContext
);

DECLSPEC_IMPORT NTSTATUS Ntdll$NtResumeThread(
    HANDLE ThreadHandle,
    PULONG PreviousSuspendCount
);