# Process Hollowing BOF
This BOF uses the [process hollowing](https://attack.mitre.org/techniques/T1055/012/) technique to run a portable executable file in the address space of another process. By default, a sacrificial process is created to inject into. Optionally, operators can spoof the parent of the sacrificial process by supplying a target parent process id. 

I created this BOF to learn further about them.

![Demo](/Img/img.png)
###### Using the BOF in Havoc Framework

## Usage
`runpe </path/to/hollow> </path/to/payload.exe> <ppid?>`

## Installation

1. Clone this repo to your device
`git clone https://github.com/wizardy0ga/RunPE-BOF`

2. Navigate to the [Install](/Install/) dir, locate the installation script for the corresponding C2 and execute it. Follow any instructions provided by the script.

## Known Issues
- The Parent Console associated with a spoofed process can still be attributed back to the creating process. (View spoofed process in SystemInformer).
- The DOS header artitfact remains in memory.
- Hardcoded call to "C:\\Windows\\System32" as CWD in CreateProcess calls should be dynamic.