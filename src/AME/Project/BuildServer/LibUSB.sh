#!/bin/bash
# ------------------------------------------------------------
# LibUSB.sh
# methods query the usb port of used devices
# ONLY valid if started in directory src/BuildServer
# author: jja
# ------------------------------------------------------------
# on use as module in other shell scripts
# to switch default echo commands off add the export before lib load
# export LIB_USB_USE_ECHO="no" ;
# ------------------------------------------------------------
# methods in alphabetical order
# ------------------------------------------------------------

#######################################
# get usb port with connected terminal
# Globals:
#   PORT
# Arguments:
#   1 "${1}" = 1 = return message, if not found
#######################################
lib_usb::get_module_terminal_port() {
  PORT="$(lib_usb::get_port "FT232" "${1}")"
  echo "${PORT}"
}

#######################################
# get usb port with connected multibox
# Globals:
#   PORT
# Arguments:
#   1 "${1}" = 1 = return message, if not found
#######################################
lib_usb::get_multi_box_port() {
  PORT="$(lib_usb::get_port "Prolific" "${1}")"
  echo "${PORT}"
}

#######################################
# get connected port of given usb device name
# Globals:
#   DEVICES
#   DEV_NAME
#   ID_SERIAL
#   PORT
#   SYS_DEV_PATH
#   SYS_PATH
# Arguments:
#   1 "${1}" = usb device search string from lsusb
#   2 "${2}" = 1 = return message, if not found
#######################################
lib_usb::get_port() {
  DEVICES=$( (
    # shellcheck disable=SC2044
    for SYS_DEV_PATH in $(find /sys/bus/usb/devices/usb*/ -name dev); do
      # ( to launch a subshell here
      (
        SYS_PATH="${SYS_DEV_PATH%/dev}"
        DEV_NAME="$(udevadm info -q name -p "${SYS_PATH}")"
        [[ "${DEV_NAME}" == "bus/"* ]] && exit
        eval "$(udevadm info -q property --export -p "${SYS_PATH}")"
        [[ -z "${ID_SERIAL}" ]] && exit
        echo "/dev/${DEV_NAME} - ${ID_SERIAL}"
      ) & # & here is causing all of these queries to run simultaneously
    done
    # wait then gives a chance for all the iterations to complete
    wait
    # output order is random due to multiprocessing so sort results
  ) | sort)
  # extract first element in first matching result row
  PORT=$(echo "${DEVICES}" | grep "${1}" | awk '{print $1}')
  if [ "${2}" == "1" ] && [ "${PORT}" == "" ]; then
    echo "not found! (${1})"
  else
    echo "${PORT}"
  fi
}

#######################################
# reset hub of e2lite device
# see https://github.com/netinvent/usb_resetter
# run only with root privilege, change /etc/sudoers
# plcdev ALL = (root) NOPASSWD: /usr/local/bin/usb_resetter
# Globals:
#   DEVICE_ID
#   DEVICE_DESC
# Arguments:
#  None
#######################################
lib_usb::reset_e2lite_port_hub() {
  FILE_OF_USB_RESETTER="/usr/local/bin/usb_resetter"
  if [ -f "${FILE_OF_USB_RESETTER}" ]; then
    DEVICE_ID="045b:82a0"
    DEVICE_DESC="Renesas E2 Lite"
    echo "##teamcity[message text='USB reset hub of device ${DEVICE_ID} (${DEVICE_DESC})']"
    sudo usb_resetter -d "${DEVICE_ID}" --reset-hub
  else
    echo "##teamcity[message text='USB reset hub: file ${FILE_OF_USB_RESETTER} not found!']"
  fi
}

if [ "${LIB_USB_USE_ECHO}" == "" ]; then
  lsusb
  echo "Port mapping"
  RETURN_MESSAGE_IF_NOT_FOUND=1
  echo "Module terminal port (FT232) is $(lib_usb::get_module_terminal_port ${RETURN_MESSAGE_IF_NOT_FOUND})"
  echo "MultiBox port (Prolific) is $(lib_usb::get_multi_box_port ${RETURN_MESSAGE_IF_NOT_FOUND})"
fi
