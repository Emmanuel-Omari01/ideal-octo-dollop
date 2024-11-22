#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# TransferToAndTestOnHardware.sh
# transfer the (given) firmware on the device and
# run the unit test related to hardware checks
# author: jja
# ------------------------------------------------------------
# prerequisites
# environment variables with local setup on build client
#   FLASH_SERIAL_NUMBERS = E2Lite serial numbers to use (blank padded)
# shell script parameters
#   $1 = teamcity_agent_work_dir (not used)
#   $2 = TEAMCITY_BUILD_WORKING_DIR (teamcity_build_workingDir)
# uses
#   LibConfig.sh library
#   LibFlashTool.sh library
#   LibMultiBox.sh library
#   TransferOnClient.sh
#   TestOnClient.sh
# ------------------------------------------------------------
# reset FLASH_SERIAL_NUMBERS
export FLASH_SERIAL_NUMBERS=""
# mark for return of failed test
export IS_HARDWARE_TEST_RETURN_FAILS="1"
# load LibConfig.sh
CURRENT_DIR="$(dirname "$0")"
. "${CURRENT_DIR}"/LibConfig.sh
# check script parameter
if [ "${2}" == "" ]; then
  TEAMCITY_BUILD_WORKING_DIR="$(lib_config::get_teamcity_build_working_dir)"
else
  TEAMCITY_BUILD_WORKING_DIR="${2}"
fi
# load LibMultiBox.sh
. "${CURRENT_DIR}"/LibMultiBox.sh
# load LibFlashTool.sh
. "${CURRENT_DIR}"/LibFlashTool.sh
# ------------------------------------------------------------
# main
RC_TRANSFER_AND_TEST=0
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "!!! Transfer and test on module !!!"
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo ""
echo "! At the moment only firmware with debug information will be used !"
echo ""
echo "Check prerequisites:"
echo "- Are the source files from the right branch (master/development) ?"
echo "- Is PLCF*.mot successfully generated in e2studio ?"
echo "- Is PLCF*.mot in the right directory (HardwareDebug) ?"
echo "- Is ONLY the e2lite connected to the new module in the TestBox connected to usb hub ?"
echo ""
echo "Are prerequisites fulfilled ? (y/n)"
while true; do
  read -r -n 1 key
  if [[ $key == "y" ]]; then
    echo ""
    echo "Ok, start of transfer and test."
    break
  else
    RC_TRANSFER_AND_TEST=1
    exit 1
  fi
done
bash TransferOnClient.sh "notUsed" "${TEAMCITY_BUILD_WORKING_DIR}"
RC_TRANSFER_ON_CLIENT=$?
if [ "${RC_TRANSFER_ON_CLIENT}" == "1" ]; then
  RC_TRANSFER_AND_TEST=1
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  echo "!!! Transfer on module failed !!!"
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  exit $RC_TRANSFER_AND_TEST
else
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  echo "!!! Transfer on module passed !!!"
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
fi
HW_TEST_SUITE_DIRS="TestSuite008 TestSuite029 TestSuite031"
echo ""
echo "!!!!!!!!!!!!!!!!!!"
echo "Use hardware test: ${HW_TEST_SUITE_DIRS}"
echo "!!!!!!!!!!!!!!!!!!"
echo ""
bash TestOnClient.sh "notUsed" "${TEAMCITY_BUILD_WORKING_DIR}" "$HW_TEST_SUITE_DIRS"
RC_TEST_ON_CLIENT=$?
if [ "${RC_TEST_ON_CLIENT}" == "1" ]; then
  RC_TRANSFER_AND_TEST=1
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  echo "!!! Test on module failed !!!"
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  exit $RC_TRANSFER_AND_TEST
else
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  echo "!!! Test on module passed !!!"
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
fi

exit $RC_TRANSFER_AND_TEST
