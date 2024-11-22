#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# TestSuite018
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
TEST_COMMAND_PREFIX="autoSubSteps 0 0"
# command(s) to execute the test suite
TEST_COMMAND="setTestSuite 1 18"
# description text to show in test / build log
TEST_DESCRIPTION="018 UpdateModule translate PLC start update"
# text from firmware to mark a successful test run
TEST_PASSED_TEXT="Testcase passed"

# ------------------------------------------------------------
# main
# ------------------------------------------------------------
echo "##teamcity[testSuiteStarted name='${TEST_SUITE_NAME}']"
# ------------------------------------------------------------
# 1. reset and run firmware on used modules
# ------------------------------------------------------------
lib_test::reset_and_run_firmware_on_coordinator_and_count_of_peers 0

echo "##teamcity[testStarted name='${TEST_DESCRIPTION}']"
# ------------------------------------------------------------
# 2. run test suite in firmware on device and get results
# ------------------------------------------------------------
# Send command to serial device and wait for line response or return on failed device:
# shellcheck disable=SC2086
printf "%s\r\n%s\r\n" "${TEST_COMMAND_PREFIX}" "${TEST_COMMAND}" \
  | "${PATH_TO_TIO}" ${MODULE_TIO_OPTIONS} "${MODULE_TERMINAL_PORT}" >"${TEST_RESULT_FILE_NAME}"
# save tio return code for check of time out error
RC_TIO=$?
# ------------------------------------------------------------
# 3. convert results to teamcity result
# ------------------------------------------------------------
lib_test::convert_results_to_teamcity_result
exit "${RC_TEST_SUITE}"
