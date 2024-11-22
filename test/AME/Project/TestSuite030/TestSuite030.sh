#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# TestSuite030
# 1. reset and run firmware on used modules
# 2. run test suite in firmware on device and get results
# 3. convert results to teamcity result
# author: jja
# ------------------------------------------------------------
# prerequisites
# environment variables with local setup on build client
#   PATH_TO_TIO
#   FLASH_SERIAL_NUMBERS = E2Lite serial numbers to use (blank padded)
# shell script parameters
#   $1 = TEST_SUITE_NAME
#   $2 = BUILD_LIB_DIR
# uses
#   LibConfig.sh library
#   LibFlashTool.sh library
#   LibMultiBox.sh library
#   LibTest.sh library
# ------------------------------------------------------------
# load LibTest.sh
. ../LibTest.sh

lib_test::check_script_parameter "${1}" "${2}" "${0}"
lib_test::load_libraries_config_setup

# ------------------------------------------------------------
# config test suite with values from firmware test suite
# ------------------------------------------------------------
# command(s) to execute before the test suite is started
TEST_COMMAND_PREFIX=""
# command(s) to execute the test suite
TEST_COMMAND="start integration test 1 120 3"
# description text to show in test / build log
TEST_DESCRIPTION="030 Alive round trips for 120 seconds with 3 peers"
# text from firmware to mark a successful test run
TEST_PASSED_TEXT="Testcase passed"

# ------------------------------------------------------------
# main
# ------------------------------------------------------------
echo "##teamcity[testSuiteStarted name='${TEST_SUITE_NAME}']"
# ------------------------------------------------------------
# 1. reset and run firmware on used modules
# ------------------------------------------------------------
# TODO(jja) remove this, skip of test
#lib_test::reset_and_run_firmware_on_coordinator_and_count_of_peers 3

echo "##teamcity[testStarted name='${TEST_DESCRIPTION}']"
## ------------------------------------------------------------
## 2. run test suite in firmware on device and get results
## ------------------------------------------------------------
## Send command to serial device and wait for line response or return on failed device:
## set tio timeout > 1000 seconds
#MODULE_TIO_OPTIONS_LOCAL="--local-echo --no-autoconnect --response-wait --response-timeout 1100000"
## shellcheck disable=SC2086
#printf "%s\r\n%s\r\n" "${TEST_COMMAND_PREFIX}" "${TEST_COMMAND}" \
#  | "${PATH_TO_TIO}" ${MODULE_TIO_OPTIONS_LOCAL} "${MODULE_TERMINAL_PORT}" >"${TEST_RESULT_FILE_NAME}"
## save tio return code for check of time out error
#RC_TIO=$?

# TODO(jja) remove this, skip of test
echo "##teamcity[message text='${TEST_SUITE_NAME} skipped !' errorDetails='No peer communication, check on hardware v1.2 must be done!' status='WARNING']"
echo "No peer communication, check on hardware v1.2 must be done!" >"${TEST_RESULT_FILE_NAME}"
RC_TIO=0

# ------------------------------------------------------------
# 3. convert results to teamcity result
# ------------------------------------------------------------
lib_test::convert_results_to_teamcity_result
exit "${RC_TEST_SUITE}"
