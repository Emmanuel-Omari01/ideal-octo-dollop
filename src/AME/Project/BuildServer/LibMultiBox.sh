#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# LibMultiBox.sh
# methods for switchable multi socket outlet
# author: jja
# ------------------------------------------------------------
# serial communication with: 9600,8,n,1
# timeout 5 sec for every command
# MultiBox commands
# {+}h = hardware id of MultiBox
# {+}s = software version of MultiBox
# {+}echo off = MultiBox does not send any echo off commands
# {+}echo on = MultiBox does send an echo off commands
# {n}get = get status of socket n=1 2 3 4 5 or + for all
# {n}off = switch socket off n=1 2 3 4 5 or + for all
# {n}on = switch socket on n=1 2 3 4 5 or + for all
# ------------------------------------------------------------
# prerequisites
# environment variables with local setup on build client
#   PATH_TO_TIO
# uses
#   LibUSB.sh library
# ------------------------------------------------------------
# load LibUSB.sh
export LIB_USB_USE_ECHO="no"
CURRENT_DIR="$(dirname "$0")"

if [ "${BUILD_LIB_DIR}" == "" ]; then
  . "${CURRENT_DIR}"/LibUSB.sh
else
  . "${BUILD_LIB_DIR}"/LibUSB.sh
fi

if [ "${PATH_TO_TIO}" == "" ]; then
  PATH_TO_TIO="tio"
fi

# local config
# MultiBox commands are ignored by default
MULTI_BOX_USE_COMMANDS=""
# tio default values are: tio -b 115200 -d 8 -f none -s 1 -p none --response-timeout 100
MULTI_BOX_TIO_OPTIONS="--baudrate 9600 --no-autoconnect --response-wait --response-timeout 5000"
# MultiBox usb port to use
MULTI_BOX_PORT="$(lib_usb::get_multi_box_port)"
# ------------------------------------------------------------
# methods in alphabetical order
# ------------------------------------------------------------

#######################################
# switch echo off for multibox commands to allow clear return codes from multibox
# Globals:
#   DESCRIPTION
#   MULTI_BOX_PORT
#   MULTI_BOX_TIO_OPTIONS
#   MULTI_BOX_USE_COMMANDS
#   PATH_TO_TIO
#   RC_MULTI_BOX
# Arguments:
#  None
#######################################
lib_multi_box::echo_off() {
  if [ "${MULTI_BOX_USE_COMMANDS}" != "" ]; then
    DESCRIPTION="MultiBox switch echo off"
    echo "##teamcity[blockOpened name='MultiBoxEchoOff' description='${DESCRIPTION}.']"
    # ------------------------------------------------------------
    # Send command to serial device and wait for line response or return on failed device:
    # shellcheck disable=SC2086
    RC_MULTI_BOX=$(printf "{+}echo off\r\n" | "${PATH_TO_TIO}" ${MULTI_BOX_TIO_OPTIONS} "${MULTI_BOX_PORT}")
    # ------------------------------------------------------------
    # no error check, because MultiBox return no clear response if echo is on
    echo "##teamcity[blockClosed name='MultiBoxEchoOff']"
  fi
}

#######################################
# switch echo on for multibox commands to reset default state
# Globals:
#   DESCRIPTION
#   MULTI_BOX_PORT
#   MULTI_BOX_TIO_OPTIONS
#   MULTI_BOX_USE_COMMANDS
#   PATH_TO_TIO
#   RC_MULTI_BOX
# Arguments:
#  None
#######################################
lib_multi_box::echo_on() {
  if [ "${MULTI_BOX_USE_COMMANDS}" != "" ]; then
    DESCRIPTION="MultiBox switch echo on"
    echo "##teamcity[blockOpened name='MultiBoxEchoOn' description='${DESCRIPTION}.']"
    # ------------------------------------------------------------
    # Send command to serial device and wait for line response or return on failed device:
    # shellcheck disable=SC2086
    RC_MULTI_BOX=$(printf "{+}echo on\r\n" | "${PATH_TO_TIO}" ${MULTI_BOX_TIO_OPTIONS} "${MULTI_BOX_PORT}")
    # ------------------------------------------------------------
    # convert error to teamcity result
    if ! [[ "${RC_MULTI_BOX:0:2}" == "OK" ]]; then
      # failed MultiBox
      # stop build with error, status can take the following values: NORMAL (default), WARNING, FAILURE, ERROR
      echo "##teamcity[message text='${DESCRIPTION} failed !' errorDetails='Is MultiBox cable connected?' status='ERROR']"
      echo "##teamcity[blockClosed name='MultiBoxEchoOn']"
      exit 1
    fi
    echo "##teamcity[blockClosed name='MultiBoxEchoOn']"
  fi
}

#######################################
# init of multibox
#   - switch all sockets off
#   - set multibox echo off
# Globals:
#   DESCRIPTION
#   MULTI_BOX_USE_COMMANDS
# Arguments:
#  None
#######################################
lib_multi_box::init() {
  DESCRIPTION="MultiBox init"
  echo "##teamcity[blockOpened name='MultiBoxInit' description='${DESCRIPTION}.']"
  if [ "${MULTI_BOX_PORT}" != "" ]; then
    MULTI_BOX_USE_COMMANDS="yes"
    # Switch multi box echo off to allow result code parsing
    lib_multi_box::echo_off
    lib_multi_box::socket_off +
  else
    ERROR_DETAILS="errorDetails='Port ${MULTI_BOX_PORT} not found! Is MultiBox cable connected?'"
    echo "##teamcity[message text='${DESCRIPTION}' ${ERROR_DETAILS} status='ERROR']"
    echo "##teamcity[blockClosed name='MultiBoxInit']"
    exit 1
  fi
  echo "##teamcity[blockClosed name='MultiBoxInit']"
}

#######################################
# shutdown of multibox
#   - switch all sockets off
#   - set multibox echo on
# description
# Globals:
#   DESCRIPTION
#   MULTI_BOX_USE_COMMANDS
# Arguments:
#  None
#######################################
lib_multi_box::shutdown() {
  DESCRIPTION="MultiBox shutdown"
  echo "##teamcity[blockOpened name='MultiBoxShutdown' description='${DESCRIPTION}.']"
  MULTI_BOX_USE_COMMANDS="yes"
  lib_multi_box::socket_off +
  lib_multi_box::echo_on
  MULTI_BOX_USE_COMMANDS=""
  echo "##teamcity[blockClosed name='MultiBoxShutdown']"
}

#######################################
# switch one multibox socket off
# optional with timer
# Globals:
#   MULTI_BOX_USE_COMMANDS
# Arguments:
#   1 "${1}" = socket off n=1 2 3 4 5 or + for all
#   2 "${2}" = socket off after m seconds, defaults to 0
#######################################
lib_multi_box::socket_off() {
  if [ "${MULTI_BOX_USE_COMMANDS}" != "" ]; then
    lib_multi_box::socket_switch_to "${1}" off "${2}"
  fi
}

#######################################
# switch one multibox socket on
# optional with timer
# Globals:
#   MULTI_BOX_USE_COMMANDS
# Arguments:
#   1 "${1}" = socket off n=1 2 3 4 5 or + for all
#   2 "${2}" = socket off after m seconds, defaults to 0
#######################################
lib_multi_box::socket_on() {
  if [ "${MULTI_BOX_USE_COMMANDS}" != "" ]; then
    lib_multi_box::socket_switch_to "${1}" on "${2}"
  fi
}

#######################################
# switch one multibox socket on or off
# optional with timer
# Globals:
#   DESCRIPTION
#   MULTI_BOX_PORT
#   MULTI_BOX_TIO_OPTIONS
#   PATH_TO_TIO
#   RC_MULTI_BOX
#   SOCKET
#   SOCKET_MODE
#   SOCKET_MODE_TEXT
#   SOCKET_TEXT
#   SOCKET_TIMER
#   SOCKET_TIMER_TEXT
# Arguments:
#   1 "${1}" = socket: n=1 2 3 4 5 or + for all
#   2 "${2}" = socket mode: on or off
#   3 "${3}" = socket off after m seconds, defaults to 0
#######################################
lib_multi_box::socket_switch_to() {
  DESCRIPTION="MultiBox switch socket"
  if [ "${1}" == "" ] || [ "${1}" == "+" ]; then
    SOCKET="+"
    SOCKET_TEXT="all"
  else
    SOCKET="${1}"
    SOCKET_TEXT="${1}"
  fi
  if [ "${2}" == "on" ]; then
    SOCKET_MODE="On"
    SOCKET_MODE_TEXT="on"
  else
    SOCKET_MODE="Off"
    SOCKET_MODE_TEXT="off"
  fi
  if [ "${3}" == "" ]; then
    SOCKET_TIMER=""
    SOCKET_TIMER_TEXT=""
  else
    SOCKET_TIMER="=${3}"
    SOCKET_TIMER_TEXT=" after ${3} seconds"
  fi
  DESCRIPTION_TEXT="description='${DESCRIPTION} ${SOCKET_TEXT} ${SOCKET_MODE_TEXT}${SOCKET_TIMER_TEXT}.'"
  echo "##teamcity[blockOpened name='MultiBoxSocket${SOCKET_MODE}' ${DESCRIPTION_TEXT}]"
  # ------------------------------------------------------------
  # Send command to serial device and wait for line response or return on failed device:
  # save multi box return code for error check
  # shellcheck disable=SC2086
  RC_MULTI_BOX=$(printf "{%s}${SOCKET_MODE_TEXT}${SOCKET_TIMER}\r\n" "${SOCKET}" | "${PATH_TO_TIO}" ${MULTI_BOX_TIO_OPTIONS} "${MULTI_BOX_PORT}")
  # ------------------------------------------------------------
  # convert error to teamcity result
  if ! [[ "${RC_MULTI_BOX:0:2}" == "OK" ]]; then
    # failed MultiBox
    # stop build with error, status can take the following values: NORMAL (default), WARNING, FAILURE, ERROR
    ERROR_DETAILS="errorDetails='Is MultiBox cable connected?'"
    echo "##teamcity[message text='${DESCRIPTION} ${SOCKET_TEXT} ${SOCKET_MODE_TEXT} failed !' ${ERROR_DETAILS} status='ERROR']"
    echo "##teamcity[blockClosed name='MultiBoxSocket${SOCKET_MODE}']"
    exit 1
  fi
  echo "##teamcity[blockClosed name='MultiBoxSocket${SOCKET_MODE}']"
}

# example
#lib_multi_box::init
#lib_multi_box::socket_on +
#lib_multi_box::socket_off 1
#lib_multi_box::socket_on 1
#lib_multi_box::socket_off 2 10
#lib_multi_box::socket_on 2 10
#lib_multi_box::shutdown
