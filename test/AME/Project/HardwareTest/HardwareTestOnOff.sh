#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# HardwareTestOnOff
# Switch MultiBox ports on and off 1000 times
# at the end switch on to show defect modules
# author: jja
# ------------------------------------------------------------
# prerequisites
# shell script parameters
#   $1 = TEST_SUITE_NAME
#   $2 = BUILD_LIB_DIR
# uses
#   LibConfig.sh library
#   LibMultiBox.sh library
#   LibTest.sh library
# ------------------------------------------------------------
# load LibTest.sh
. ../LibTest.sh

lib_test::check_script_parameter "${1}" "${2}" "${0}"
lib_test::load_libraries_config_setup

# ------------------------------------------------------------
# config test suite
# ------------------------------------------------------------
# description text to show in test / build log
TEST_DESCRIPTION="Switch power on and off 1000 times"

# ------------------------------------------------------------
# main
# ------------------------------------------------------------
echo "##teamcity[testSuiteStarted name='${TEST_SUITE_NAME}']"

lib_multi_box::init

for i in {1..1000} ; do
  echo "##teamcity[message text='Step $i of 1000: $(date +%Y-%m-%d-%H:%M:%S)']"
  lib_multi_box::socket_on +
  lib_multi_box::socket_off +
done

# switch all on
lib_multi_box::socket_on +

# ------------------------------------------------------------
echo "##teamcity[testFinished name='${TEST_DESCRIPTION}']"
echo "##teamcity[testSuiteFinished name='${TEST_SUITE_NAME}']"

exit 0
