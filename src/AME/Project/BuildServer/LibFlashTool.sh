#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# LibFlashTool.sh
# methods for flasher tool of device
# e2 programmer must run with (root) rights to access port on
# /dev/ttyS0...S3 or use 99-renesas-emu.rules from flasher
# directory
# author: jja
# ------------------------------------------------------------
# rfp-cli parameters
# -d = Select device type
# -t = Select tool
# -t:<serial> = Select tool with serial # use -list-tools parameter to read serial
# -if fine = Select the communication interface FINE (see e2studio config)
# -vo 3.3 = Enable power supply NO USE with power from plug
# -range-exclude 00100000-00101FFF = exclude from programming, here data memory
# commands
# -reset = Stop firmware on device
# -run = Start firmware on device
# -a = Same as -e -p -v (erase, program, verify)
# mot file name
# ------------------------------------------------------------
# prerequisites
# environment variables with local tool path on build client
#   PATH_TO_RENESAS_FLASHER
#   PATH_TO_TIO
# uses
#   LibConfig.sh library, must be load in parent
#   LibUSB.sh library
# ------------------------------------------------------------
# load LibUSB.sh
export LIB_USB_USE_ECHO="no"
CURRENT_DIR="$(dirname "$0")"

if [ "${BUILD_LIB_DIR}" == "" ]; then
  . "${CURRENT_DIR}"/LibConfig.sh
  . "${CURRENT_DIR}"/LibUSB.sh
else
  . "${BUILD_LIB_DIR}"/LibConfig.sh
  . "${BUILD_LIB_DIR}"/LibUSB.sh
fi

if [ "${PATH_TO_RENESAS_FLASHER}" == "" ]; then
  PATH_TO_RENESAS_FLASHER="/home/plcdev/dev/toolchains/renesas_flasher/"
fi

# local config
if [ "$(lib_config::use_tio_on_module_terminal_in_background)" -eq 1 ]; then
  if [ "${PATH_TO_TIO}" == "" ]; then
    PATH_TO_TIO="tio"
  fi
  # tio default values are: tio -b 115200 -d 8 -f none -s 1 -p none --response-timeout 100
  FLASH_TOOL_TIO_OPTIONS=""
  # Module terminal usb port to use
  MODULE_TERMINAL_PORT="$(lib_usb::get_module_terminal_port)"
fi
# ------------------------------------------------------------
# used vars for rfp-cli parameter setup
# ------------------------------------------------------------
FLASH_DEVICE_TYPE="RX100"
FLASH_TOOL="e2l"
FLASH_INTERFACE="fine"
# use only one variable value of the following definitions
# to switch power supply from tool or not; defaults to not
# FLASH_POWER="-vo 3.3" ;
FLASH_POWER=""
# to exclude data memory from change or not; defaults to change
# FLASH_EXCLUDE_DATA_MEMORY="-range-exclude 00100000,00101FFF" ;
FLASH_EXCLUDE_DATA_MEMORY=""
# ------------------------------------------------------------
# methods in alphabetical order
# ------------------------------------------------------------

#######################################
# reset the device connected to the flash tool
# Globals:
#   DESCRIPTION
#   FLASH_DEVICE_TYPE
#   FLASH_INTERFACE
#   FLASH_POWER
#   FLASH_TOOL
#   FLASH_TOOL_WITH_SERIAL
#   PATH_TO_RENESAS_FLASHER
#   RC_RFP_CLI
# Arguments:
#   1 "${1}" = optional e2lite serial number
#######################################
lib_flash_tool::reset_device() {
  if [ "${1}" = "" ]; then
    FLASH_TOOL_WITH_SERIAL="${FLASH_TOOL}"
  else
    FLASH_TOOL_WITH_SERIAL="${FLASH_TOOL}:${1}"
  fi
  DESCRIPTION="FlashTool reset of device"
  echo "##teamcity[blockOpened name='FlashToolResetDevice' description='${DESCRIPTION} ${FLASH_TOOL_WITH_SERIAL} (tool:<serial>).']"
  # ------------------------------------------------------------
  # reset the device, to build a defined state of the device
  lib_flash_tool::start_tio_on_module_terminal_in_background
  RESET_REPEAT=0
  while [[ 5 -gt $RESET_REPEAT ]]; do
    # shellcheck disable=SC2086
    "${PATH_TO_RENESAS_FLASHER}"rfp-cli -d "${FLASH_DEVICE_TYPE}" -t "${FLASH_TOOL_WITH_SERIAL}" -if "${FLASH_INTERFACE}" ${FLASH_POWER} -reset
    # save rfp-cli return code for error check
    RC_RFP_CLI=$?
    if [[ $RC_RFP_CLI -eq 0 ]]; then
        break # reset is ok
    fi
    if [ "$(lib_config::use_usb_reset_for_e2lite)" -gt 0 ]; then
      echo "##teamcity[message text='${DESCRIPTION} ${FLASH_TOOL_WITH_SERIAL} reset usb hub(s) with e2lite!']"
      lib_usb::reset_e2lite_port_hub
    fi
    RESET_REPEAT=$((RESET_REPEAT + 1))
    SLEEP_TIME_OUT_IN_SEC=$(lib_config::get_usb_reset_wait_time)
    echo "##teamcity[message text='${DESCRIPTION} ${FLASH_TOOL_WITH_SERIAL} sleep ${SLEEP_TIME_OUT_IN_SEC} sec. and repeat ${RESET_REPEAT} times!']"
    sleep "${SLEEP_TIME_OUT_IN_SEC}"
  done
  # ------------------------------------------------------------
  # convert error to teamcity result
  # shellcheck disable=SC2086
  if [ $RC_RFP_CLI -eq 1 ]; then
    # failed flasher tool
    # stop build with error, status can take the following values: NORMAL (default), WARNING, FAILURE, ERROR
    ERROR_DETAILS="errorDetails='Is flash programmer cable connected?'"
    echo "##teamcity[message text='${DESCRIPTION} ${FLASH_TOOL_WITH_SERIAL} failed !' ${ERROR_DETAILS} status='ERROR']"
    echo "##teamcity[blockClosed name='FlashToolResetDevice']"
    exit 1
  fi
  echo "##teamcity[blockClosed name='FlashToolResetDevice']"
}

#######################################
# start firmware on device and wait x sec for boot up
# Globals:
#   DESCRIPTION
#   FLASH_DEVICE_TYPE
#   FLASH_INTERFACE
#   FLASH_POWER
#   FLASH_TOOL
#   FLASH_TOOL_WITH_SERIAL
#   PATH_TO_RENESAS_FLASHER
#   RC_RFP_CLI
# Arguments:
#   1 "${1}" = optional e2lite serial number
#######################################
lib_flash_tool::start_firmware_on_device() {
  if [ "${1}" = "" ]; then
    FLASH_TOOL_WITH_SERIAL="${FLASH_TOOL}"
  else
    FLASH_TOOL_WITH_SERIAL="${FLASH_TOOL}:${1}"
  fi
  DESCRIPTION="FlashTool run / start firmware on device"
  echo "##teamcity[blockOpened name='FlashToolRunFirmwareOnDevice' description='${DESCRIPTION} ${FLASH_TOOL_WITH_SERIAL} (tool:<serial>).']"
  # ------------------------------------------------------------
  lib_flash_tool::start_tio_on_module_terminal_in_background
  # shellcheck disable=SC2086
  "${PATH_TO_RENESAS_FLASHER}"rfp-cli -d "${FLASH_DEVICE_TYPE}" -t "${FLASH_TOOL_WITH_SERIAL}" -if "${FLASH_INTERFACE}" ${FLASH_POWER} -run
  # save rfp-cli return code for error check
  RC_RFP_CLI=$?
  # ------------------------------------------------------------
  # convert error to teamcity result
  if [ $RC_RFP_CLI -eq 1 ]; then
    # failed flasher tool
    # stop build with error, status can take the following values: NORMAL (default), WARNING, FAILURE, ERROR
    ERROR_DETAILS="errorDetails='Is flash programmer cable connected?'"
    echo "##teamcity[message text='${DESCRIPTION} ${FLASH_TOOL_WITH_SERIAL} failed !' ${ERROR_DETAILS} status='ERROR']"
    echo "##teamcity[blockClosed name='FlashToolRunFirmwareOnDevice']"
    exit 1
  else
    SLEEP_TIME_OUT_IN_SEC=$(lib_config::get_firmware_startup_wait_time)
    echo "##teamcity[message text='Sleep ${SLEEP_TIME_OUT_IN_SEC} sec. and wait for boot of firmware on device ${FLASH_TOOL_WITH_SERIAL}.' status='NORMAL']"
    sleep "${SLEEP_TIME_OUT_IN_SEC}"
  fi
  echo "##teamcity[blockClosed name='FlashToolRunFirmwareOnDevice']"
}

#######################################
# transfer firmware to device
# Globals:
#   FLASH_DEVICE_TYPE
#   FLASH_EXCLUDE_DATA_MEMORY
#   FLASH_INTERFACE
#   FLASH_POWER
#   FLASH_TOOL
#   FLASH_TOOL_WITH_SERIAL
#   PATH_TO_RENESAS_FLASHER
#   RC_RFP_CLI
# Arguments:
#   1 "${1}" = firmware file name with full path (mot file)
#   2 "${2}" = optional e2lite serial number
#######################################
lib_flash_tool::transfer_firmware_to_device() {
  if [ "${2}" = "" ]; then
    FLASH_TOOL_WITH_SERIAL="${FLASH_TOOL}"
  else
    FLASH_TOOL_WITH_SERIAL="${FLASH_TOOL}:${2}"
  fi
  DESCRIPTION_TEXT="description='Erase, program and verify device memory of ${FLASH_TOOL_WITH_SERIAL} (tool:<serial>).'"
  echo "##teamcity[blockOpened name='FlashToolTransferFirmwareToDevice' ${DESCRIPTION_TEXT}]"
  echo "##teamcity[message text='Transfer file ${1} to device ${FLASH_TOOL_WITH_SERIAL} (tool:<serial>).' status='NORMAL']"
  # ------------------------------------------------------------
  lib_flash_tool::start_tio_on_module_terminal_in_background
  # remove of progress bar from log
  FLASH_RESULT_FILE_NAME="resultOfFlash"
  # shellcheck disable=SC2086
  "${PATH_TO_RENESAS_FLASHER}"rfp-cli -d "${FLASH_DEVICE_TYPE}" -t "${FLASH_TOOL_WITH_SERIAL}" -if "${FLASH_INTERFACE}" \
    ${FLASH_POWER} ${FLASH_EXCLUDE_DATA_MEMORY} -a "${1}"  2>"${FLASH_RESULT_FILE_NAME}"
  # save rfp-cli return code for error check
  RC_RFP_CLI=$?
  # ------------------------------------------------------------
  # convert error to teamcity result
  if [ $RC_RFP_CLI -eq 1 ]; then
    # failed flasher tool
    cat "${FLASH_RESULT_FILE_NAME}"
    # stop build with error, status can take the following values: NORMAL (default), WARNING, FAILURE, ERROR
    ERROR_DETAILS="errorDetails='Is flash programmer cable connected?'"
    echo "##teamcity[message text='Transfer to device ${FLASH_TOOL_WITH_SERIAL} failed !' ${ERROR_DETAILS} status='ERROR']"
    echo "##teamcity[blockClosed name='FlashToolTransferFirmwareToDevice']"
    exit 1
  fi
  # remove the result file
  if [ -f "${FLASH_RESULT_FILE_NAME}" ]; then
    rm "${FLASH_RESULT_FILE_NAME}"
  fi
  echo "##teamcity[blockClosed name='FlashToolTransferFirmwareToDevice']"
}

#######################################
# work around for rfp-cli error on no device found
# Globals:
#   FLASH_TOOL_TIO_OPTIONS
#   MODULE_TERMINAL_PORT
#   PATH_TO_TIO
# Arguments:
#   None
#######################################
lib_flash_tool::start_tio_on_module_terminal_in_background() {
  if [ "$(lib_config::use_tio_on_module_terminal_in_background)" -eq 1 ]; then
    echo "start tio on module terminal port (${MODULE_TERMINAL_PORT}) in background"
    # shellcheck disable=SC2086
    "${PATH_TO_TIO}" ${FLASH_TOOL_TIO_OPTIONS} "${MODULE_TERMINAL_PORT}" 2>/dev/null 1>&2 &
  fi
}

# test serial numbers
# lib_flash_tool::reset_device "${1}"
