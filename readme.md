# Process Hollowing BOF
This BOF uses the [process hollowing](https://attack.mitre.org/techniques/T1055/012/) technique to run a portable executable file in the address space of another process. By default, a sacrificial process is created to inject into. Optionally, operators can spoof the parent of the sacrificial process by supplying a target parent process id. 

I created this BOF to learn further about them.

###### Using the BOF in Havoc Framework
![Demo](/Img/img.png)

###### Using the BOF in Adaptix C2
![Adaptix Decmo](/Img/adc2.png)

## Usage

### Adaptix C2
`runpe /path/to/hollow path/to/payload.exe -cwd CWD -ppid 1234`

### Havoc Framework
`runpe /path/to/hollow path/to/payload.exe cwd ppid`
`runpe /path/to/hollow path/to/payload.exe ppid`

## Installation

1. Clone this repo to your device
`git clone https://github.com/wizardy0ga/RunPE-BOF`

2. Navigate to the [Install](/Install/) dir, locate the installation script for the corresponding C2 and execute it. Follow any instructions provided by the script.

### Supported Frameworks
- [Havoc](https://github.com/havocframework/havoc)
- [Adaptix C2](https://github.com/Adaptix-Framework/AdaptixC2)

## Known Issues
- The Parent Console associated with a spoofed process can still be attributed back to the creating process. (View spoofed process in SystemInformer).
- Various calls within this source code are dynamically resolved by beacon rather than using beacons built in calls for the same functionality. This is due to this BOF being developed with the Havoc Framework which doesn't support some of the functionality that the original BOF api supports. See [ObjectApi.c](https://github.com/HavocFramework/Havoc/blob/main/payloads/Demon/src/core/ObjectApi.c) in Havoc. I don't have access to cobalt strike :').