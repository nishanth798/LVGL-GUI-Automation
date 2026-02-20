# Firmware: VST One Touch Display



## Download and Install

Download binaries from [here](https://github.com/obhcare/vstone/releases)

- **Simulator Binary:** `vstone.ui.sim.linux-amd64.v1.0.0-alpha`
- **Firmware Files:** `vstone.ui.v1.0.0-alpha.zip`

#### NOTE: Simulator binary is linux-amd64 only as of now.

### Firmware
Install arduino-cli `v0.34` from [here](https://arduino.github.io/arduino-cli/0.34/). This tool will be used to upload the firmware.

Run the following commands in terminal

Install esp32 core

`arduino-cli core update-index && arduino-cli core install esp32:esp32@2.0.11`

Upload firmware

`arduino-cli upload --discovery-timeout 3s -b esp32:esp32:esp32s3:PSRAM=opi,PartitionScheme=huge_app,FlashSize=16M,CDCOnBoot=cdc --verify --input-dir <vstone.ui.v1.0.0-alpha> --port </dev/ttyACM0>`


- Extract the firmware files and replace `<vstone.ui.v1.0.0-alpha>` with the firmware directory.  
- Replace `</dev/ttyACM0>` with the correct VSTOne Display port number on your machine.


### Simulator

Make executable

`chmod u+x <vstone.ui.sim.linux-amd64.v1.0.0-alpha>`

Run

`./vstone.ui.sim.linux-amd64.v1.0.0-alpha`

## Usage

## API
Read [API doc](api.md)

## Test


## Development Setup
### Prerequisites

#### Linux/Ubuntu
`sudo apt install build-essential libsdl2-dev`

#### Mac

### Library Docs:
- Arduino board platform [arduino-esp32](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/index.html)
- GUI Library [LVGL v8.3](https://docs.lvgl.io/8.3/index.html)



### Sublime Text
https://www.sublimetext.com/


## Known Issues
- While trying to erase flash using `esptool.py --port /dev/cu.usbmodem111101 --chip esp32 erase_flash` on macos the serial port dissappered (display flash was not erased), but `ioreg -p IOUSB` continued to list the display as one of the connected USB devices. Unplugging and replugging the display USB connector did not fix the problem, but restarting macos resolved the problem. `esptool.py` may not have closed the port cleanly before crashing.
// TODO: See how we can restart the service that manages USB serial ports on macos and linux. This could be the same problem experience by the US team while utilizing watchdog.

- Custom USB VID:PID change reverted as serial comm failed.



