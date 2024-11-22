### Description

Use of shell scripts to build the project from source code and
run tests on device

1. get fresh source code from version control system
   (on build client this step will be done from the build server)
2. build project from source code = BuildOnClient.sh
3. transfer the firmware on the device = TransferOnClient.sh
4. test the firmware on the device = TestOnClient.sh


### Prerequisites

- environment variables with local setup on build client
  - PATH_TO_E2STUDIO
  - PATH_TO_RENESAS_FLASHER
  - PATH_TO_TIO
  - FLASH_SERIAL_NUMBERS = E2Lite serial numbers to use (blank padded) or empty for first found

- shell script parameters
  - $1 = teamcity_agent_work_dir
  - $2 = teamcity_build_workingDir

- shell script libraries
  - LibConfig.sh = config of environment with directory path vars
  - LibFlashTool.sh = flash tool commands and setup of tool parameter
  - LibMultiBox.sh = MultiBox commands and setup of parameter
  - LibUSB.sh = get MultiBox and Module terminal USB device ports
  - LibTest.sh = general functions for use in TestSuit*.sh files

### Shell scripts that uses the BuildServer scripts

- TransferToAndTestOnHardware.sh

  Used for burn in check after production of new hardware.
  This script flashes the firmware onto the hardware and
  run hardware related test to check the basic function.