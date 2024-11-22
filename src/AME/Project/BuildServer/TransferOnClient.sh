#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# TransferOnClient.sh
# transfer the firmware on the device
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
# ------------------------------------------------------------
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
# set return code of script
RC_TRANSFER_ON_CLIENT=0
lib_config::is_version_of_hw_module_and_software_config_identical
RC_IS_IDENTICAL=$?
if [ $RC_IS_IDENTICAL -ne 0 ]; then
  echo "##teamcity[message text='Transfer on client stopped ! Version of hardware module and software ProgramConfig.h mismatch !' status='ERROR']"
  RC_TRANSFER_ON_CLIENT=1
else
  if [ "$(lib_config::use_multibox)" -gt 0 ]; then
    lib_multi_box::init
  fi
  FIRMWARE_FILENAME_WITH_PATH="$(lib_config::get_firmware_filename_with_path "${TEAMCITY_BUILD_WORKING_DIR}")"
  if [ "${FLASH_SERIAL_NUMBERS}" == "" ]; then
    lib_multi_box::socket_on 1
    lib_flash_tool::reset_device
    lib_flash_tool::transfer_firmware_to_device "${FIRMWARE_FILENAME_WITH_PATH}"
  else
    lib_multi_box::socket_on +
    FLASH_SERIALS=("${FLASH_SERIAL_NUMBERS}")
    # shellcheck disable=SC2128
    for FLASH_SERIAL in $FLASH_SERIALS; do
      lib_flash_tool::reset_device "${FLASH_SERIAL}"
      lib_flash_tool::transfer_firmware_to_device "${FIRMWARE_FILENAME_WITH_PATH}" "${FLASH_SERIAL}"
    done
  fi
  if [ "$(lib_config::use_multibox_in_test_suites)" -gt 0 ]; then
    lib_multi_box::shutdown
  fi
fi
exit $RC_TRANSFER_ON_CLIENT
