#!/bin/bash
# ------------------------------------------------------------
# LibConfig.sh
# methods for setup of vars and environment
# ONLY valid if started in directory src/AME/Project/BuildServer
# author: jja
# ------------------------------------------------------------
# methods in alphabetical order
# ------------------------------------------------------------

#######################################
# get name of the default eclipse build configuration
# Globals:
#   BASH_SOURCE
# Arguments:
#  None
#######################################
lib_config::get_eclipse_build_config_default() {
  echo HardwareDebug
}

#######################################
# get firmware startup wait time to allow boot of module to finish
# in seconds
# Globals:
#   BASH_SOURCE
# Arguments:
#  None
#######################################
lib_config::get_firmware_startup_wait_time() {
  echo 15
}

#######################################
# extract the filename of the firmware mot file
# Globals:
#   FIRMWARE_FILENAME
#   MOT_FILES
#   TEAMCITY_BUILD_WORKING_DIR
# Arguments:
#   1 "${1}" = TEAMCITY_BUILD_WORKING_DIR
#######################################
lib_config::get_firmware_filename_with_path() {
  if [ "${1}" = "" ]; then
    TEAMCITY_BUILD_WORKING_DIR="$(lib_config::get_teamcity_build_working_dir)"
  else
    TEAMCITY_BUILD_WORKING_DIR="${1}"
  fi
  # collect mot files
  shopt -s dotglob
  shopt -s nullglob
  MOT_FILES=("${TEAMCITY_BUILD_WORKING_DIR}"/HardwareDebug/*.mot)
  FIRMWARE_FILENAME="${MOT_FILES[0]}"
  echo "${FIRMWARE_FILENAME}"
}

#######################################
# define CONFIG_LIB_PATH
# Globals:
#   CONFIG_LIB_PATH
#   TEAMCITY_BUILD_WORKING_DIR
# Arguments:
#   1 "${1}" = TEAMCITY_BUILD_WORKING_DIR
#######################################
lib_config::get_lib_path() {
  if [ "${1}" = "" ]; then
    TEAMCITY_BUILD_WORKING_DIR="$(lib_config::get_teamcity_build_working_dir)"
  else
    TEAMCITY_BUILD_WORKING_DIR="${1}"
  fi
  CONFIG_LIB_PATH="${TEAMCITY_BUILD_WORKING_DIR}"/src/AME/Project/BuildServer
  echo "${CONFIG_LIB_PATH}"
}

#######################################
# define TEAMCITY_BUILD_WORKING_DIR
# Globals:
#   BASH_SOURCE
#   TEAMCITY_BUILD_WORKING_DIR
# Arguments:
#  None
#######################################
lib_config::get_teamcity_build_working_dir() {
  TEAMCITY_BUILD_WORKING_DIR=$(cd -P -- "$(dirname -- "${BASH_SOURCE[0]}")/../../../.." &>/dev/null && pwd)
  echo "${TEAMCITY_BUILD_WORKING_DIR}"
}

#######################################
# check module compatibility of hardware and software version
# use 0=no, 1=yes
# Arguments:
#  None
#######################################
lib_config::is_version_of_hw_module_and_software_config_identical() {
  DESCRIPTION="Check module hardware version match to configured software version"
  echo "##teamcity[blockOpened name='VersionCheck' description='${DESCRIPTION}.']"
  RC_IS_IDENTICAL=1
  # no matching version found
  FILE_WITH_VERSION_TEXT="$(lib_config::get_teamcity_build_working_dir)/src/AME/Project/SystemBase/ProgramConfig.h"
  echo "##teamcity[message text='Use file ${FILE_WITH_VERSION_TEXT}']"
  if [ -f "${FILE_WITH_VERSION_TEXT}" ]; then
    # defaults to 1.1
    BUILD_CLIENT_HW_MODULE_VERSION="${WITH_PROGRAM_CONFIG_H_HW_VERSION:-1}"
    HW_MODULE_VERSION_SEARCH_STRING="#define HW_VERSION ${BUILD_CLIENT_HW_MODULE_VERSION}  //"
    echo "##teamcity[message text='Search text from build client =${HW_MODULE_VERSION_SEARCH_STRING}= (used without string frame character =)']"
    if grep -q "${HW_MODULE_VERSION_SEARCH_STRING}" "${FILE_WITH_VERSION_TEXT}"; then
      # matching version found
      RC_IS_IDENTICAL=0
      echo "##teamcity[message text='Software version matches hardware version.']"
    else
      echo "##teamcity[message text='Software version does NOT match hardware version.' status='WARNING']"
    fi
  else
    echo "##teamcity[message text='File not found!']"
  fi
  echo "##teamcity[blockClosed name='VersionCheck']"
  return ${RC_IS_IDENTICAL}
}

#######################################
# flag for send of MultiBox commands
# use 0=no, 1=yes
# Arguments:
#  None
#######################################
lib_config::use_multibox() {
  echo 1
}

#######################################
# flag for send of MultiBox commands in test suites
# use 0=no, 1=yes
# Arguments:
#  None
#######################################
lib_config::use_multibox_in_test_suites() {
  echo 1
}

#######################################
# flag for start of tio in background
# use 0=no start, 1=start
# Arguments:
#  None
#######################################
lib_config::use_tio_on_module_terminal_in_background() {
  echo 1
}

#######################################
# flag for reset of usb hub(s) with e2lite
# use 0=no, 1=yes
# Arguments:
#  None
#######################################
lib_config::use_usb_reset_for_e2lite() {
  echo 1
}

#######################################
# get usb reset wait time
# in seconds
# Globals:
#   BASH_SOURCE
# Arguments:
#  None
#######################################
lib_config::get_usb_reset_wait_time() {
  echo 5
}
