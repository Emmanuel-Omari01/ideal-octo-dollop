#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# LibTest.sh
# methods for TestSuites
# ONLY valid if started in directory test/AME/Project
# author: jja
# ------------------------------------------------------------
# prerequisites
# environment variables with local setup on build client
#   PATH_TO_TIO
#   FLASH_SERIAL_NUMBERS = E2Lite serial numbers to use (blank padded)
# uses
#   LibConfig.sh library
#   LibFlashTool.sh library
#   LibMultiBox.sh library
# ------------------------------------------------------------
# methods in alphabetical order
# ------------------------------------------------------------

#######################################
# check parameter of testSuite
# Globals:
#   BASH_SOURCE
# Arguments:
#   1 "${1}" = TEST_SUITE_NAME
#   2 "${2}" = BUILD_LIB_DIR
#   3 "${3}" = optional $0 from calling script
#######################################
lib_test::check_script_parameter() {
  if [ "${1}" == "" ]; then
    echo "miss script parameter 1 = TEST_SUITE_NAME"
	if [ "${3}" != "" ]; then
      TEST_SUITE_FILE_NAME="$(basename "${3}")"
      TEST_SUITE_NAME="${TEST_SUITE_FILE_NAME%%.*}"
      echo "use ${TEST_SUITE_NAME}"
    else
      exit 1
    fi
  else
    TEST_SUITE_NAME="${1}"
  fi
  if [ "${2}" == "" ]; then
    echo "miss script parameter 2 = BUILD_LIB_DIR"
    TEAMCITY_BUILD_WORKING_DIR=$(cd -P -- "$(dirname -- "${3}")/../../../.." &>/dev/null && pwd)
    BUILD_LIB_DIR="${TEAMCITY_BUILD_WORKING_DIR}"/src/AME/Project/BuildServer
    echo "use ${BUILD_LIB_DIR}"
  else
    BUILD_LIB_DIR="${2}"
  fi
}

#######################################
# load the libraries and config setup
# Globals:
#   BASH_SOURCE
#   TEST_SUITE_NAME
#   TEST_LIB_DIR
#######################################
lib_test::load_libraries_config_setup() {
  # load LibConfig.sh
  . "${BUILD_LIB_DIR}"/LibConfig.sh
  # load LibFlashTool.sh
  . "${BUILD_LIB_DIR}"/LibFlashTool.sh
  # load LibMultiBox.sh
  . "${BUILD_LIB_DIR}"/LibMultiBox.sh
  # local config
  if [ "${PATH_TO_TIO}" == "" ]; then
    PATH_TO_TIO="tio"
  fi
  # file name for captured data from firmware terminal
  TEST_RESULT_FILE_NAME="resultOfFirmwareTest"
  # tio default values are: tio -b 115200 -d 8 -f none -s 1 -p none --response-timeout 100
  MODULE_TIO_OPTIONS="--local-echo --no-autoconnect --response-wait --response-timeout 2000"
  # Module terminal usb port to use
  MODULE_TERMINAL_PORT="$(lib_usb::get_module_terminal_port)"
}

#######################################
# reset and run firmware on used modules
# with given parameter
#  1 "${1}" = count of used peers
#  only first module = coordinator
# and given count of peers will be used
# Globals:
#   BASH_SOURCE
#   TEST_SUITE_NAME
#   TEST_LIB_DIR
# Arguments:
#   1 "${1}" = count of used peers
#######################################
lib_test::reset_and_run_firmware_on_coordinator_and_count_of_peers() {
  if [ "${1}" == "" ]; then
    COUNT_OF_PEERS=0
  else
    COUNT_OF_PEERS="${1}"
  fi
  echo "##teamcity[message text='Environment 1 coordinator, ${COUNT_OF_PEERS} peers.']"
  if [ "$(lib_config::use_multibox_in_test_suites)" -gt 0 ]; then
    lib_multi_box::init
  fi
  if [ "${FLASH_SERIAL_NUMBERS}" == "" ]; then
    lib_multi_box::socket_on 1
    lib_flash_tool::reset_device
    lib_flash_tool::start_firmware_on_device
  else
    SOCKET_COUNT=1
    FLASH_SERIALS=("${FLASH_SERIAL_NUMBERS}")
    # shellcheck disable=SC2128
    for FLASH_SERIAL in $FLASH_SERIALS; do
      lib_multi_box::socket_on $SOCKET_COUNT
      lib_flash_tool::reset_device "${FLASH_SERIAL}"
      lib_flash_tool::start_firmware_on_device "${FLASH_SERIAL}"
      if [[ 0 == "${COUNT_OF_PEERS}" ]]; then
        break
      else
        ((COUNT_OF_PEERS-=1))
        ((SOCKET_COUNT+=1))
      fi
    done
  fi
}

#######################################
# convert results to teamcity result
# Globals:
#   BASH_SOURCE
#   TEST_SUITE_NAME
#   TEST_LIB_DIR
#######################################
lib_test::convert_results_to_teamcity_result() {
  RC_TEST_SUITE=0
  if [ "${RC_TIO}" -eq 1 ]; then
    # failed connect
    cat "${TEST_RESULT_FILE_NAME}"
    # stop build with error, status can take the following values: NORMAL (default), WARNING, FAILURE, ERROR
    ERROR_DETAILS="errorDetails='tio response timeout! Is hardware USB cable connected to VM ${MODULE_TERMINAL_PORT} ?'"
    echo "##teamcity[message text='${TEST_SUITE_NAME} failed !' ${ERROR_DETAILS} status='ERROR']"
    RC_TEST_SUITE=1
  else
    if grep -q "${TEST_PASSED_TEXT}" "${TEST_RESULT_FILE_NAME}"; then
      # successful test
      echo "##teamcity[testStdOut name='${TEST_DESCRIPTION}' out='${TEST_DESCRIPTION} is successful, see details in build log...']"
      cat "${TEST_RESULT_FILE_NAME}"
      echo "##teamcity[testFinished name='${TEST_DESCRIPTION}']"
      echo "##teamcity[testSuiteFinished name='${TEST_SUITE_NAME}']"
    else
      # failed test
      echo "##teamcity[testStdErr name='${TEST_DESCRIPTION}' out='${TEST_DESCRIPTION} is failed, see details in build log...']"
      cat "${TEST_RESULT_FILE_NAME}"
      echo "##teamcity[testFailed name='${TEST_DESCRIPTION}' message='${TEST_DESCRIPTION} is failed !' details='see details in build log...']"
      echo "##teamcity[testFinished name='${TEST_DESCRIPTION}']"
      echo "##teamcity[testSuiteFinished name='${TEST_SUITE_NAME}']"
      if [ "${IS_HARDWARE_TEST_RETURN_FAILS}" = "1" ]; then
        RC_TEST_SUITE=1
      fi
    fi
  fi
  # remove the result file
  if [ -f "${TEST_RESULT_FILE_NAME}" ]; then
    rm "${TEST_RESULT_FILE_NAME}"
  fi
}
