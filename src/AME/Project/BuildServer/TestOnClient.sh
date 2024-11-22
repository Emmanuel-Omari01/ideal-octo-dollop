#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# TestOnClient.sh
# run the firmware test suites on the device
# author: jja
# ------------------------------------------------------------
# prerequisites
# shell script parameters
#   $1 = teamcity_agent_work_dir (not used)
#   $2 = TEAMCITY_BUILD_WORKING_DIR (teamcity_build_workingDir)
#   $3 = given test suite directories (i.e. "TestSuite008 TestSuite029") or if empty collect all directories
# uses
#   LibConfig.sh library
#   LibFlashTool.sh library
#   LibMultiBox.sh library
# ------------------------------------------------------------
# load LibConfig.sh
CURRENT_DIR="$(dirname "$0")"
. "${CURRENT_DIR}"/LibConfig.sh
# check script parameter
if [ "${2}" = "" ]; then
  TEAMCITY_BUILD_WORKING_DIR="$(lib_config::get_teamcity_build_working_dir)"
else
  TEAMCITY_BUILD_WORKING_DIR="${2}"
fi
# load LibMultiBox.sh
. "${CURRENT_DIR}"/LibMultiBox.sh
# set return code of script
RC_TEST_ON_CLIENT=0
lib_config::is_version_of_hw_module_and_software_config_identical
RC_IS_IDENTICAL=$?
if [ $RC_IS_IDENTICAL -ne 0 ]; then
  echo "##teamcity[message text='Test on client stopped ! Version of hardware module and software ProgramConfig.h mismatch !' status='ERROR']"
  RC_TEST_ON_CLIENT=1
else
  # define lib path for test suites
  TEST_LIB_DIR="$(lib_config::get_lib_path "${TEAMCITY_BUILD_WORKING_DIR}")"
  export TEST_LIB_DIR
  # change to directory with test suites
  TEAMCITY_TEST_DIR="${TEAMCITY_BUILD_WORKING_DIR}/test/AME/Project"
  cd "${TEAMCITY_TEST_DIR}" || exit 1
  # use given test suite directories or collect all directories
  if [ "${3}" != "" ]; then
    # shellcheck disable=SC2206
    TEST_SUITE_DIRS=(${3})
  else
    # collect all directories
    shopt -s dotglob
    shopt -s nullglob
    TEST_SUITE_DIRS=(TestSuite*)
  fi
  # main, run all test suites
  for TEST_SUITE_DIR in "${TEST_SUITE_DIRS[@]}"; do
    cd "${TEAMCITY_TEST_DIR}/$TEST_SUITE_DIR" || exit 1
    bash "$TEST_SUITE_DIR".sh "$TEST_SUITE_DIR" "$TEST_LIB_DIR"
    RC_TEST_ON_CLIENT=$?
    if [ "${RC_TEST_ON_CLIENT}" == "1" ]; then
      if [ "${IS_HARDWARE_TEST_RETURN_FAILS}" = "1" ]; then
        exit $RC_TEST_ON_CLIENT
      else
        break
      fi
    fi
  done
  if [ "$(lib_config::use_multibox)" -gt 0 ]; then
    lib_multi_box::shutdown
  fi
fi
exit $RC_TEST_ON_CLIENT
