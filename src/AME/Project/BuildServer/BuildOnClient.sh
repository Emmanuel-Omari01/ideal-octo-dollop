#!/bin/bash
# shellcheck disable=SC1090
# ------------------------------------------------------------
# BuildOnClient.sh
# build firmware project from source code
# author: jja
# ------------------------------------------------------------
# prerequisites
# environment variables with local tool path on build client
#   PATH_TO_E2STUDIO
# shell script parameters
#   $1 = TEAMCITY_AGENT_WORK_DIR (teamcity_agent_work_dir)
#   $2 = TEAMCITY_BUILD_WORKING_DIR (teamcity_build_workingDir)
# uses
#   LibConfig.sh library
# ------------------------------------------------------------
# load LibConfig.sh
CURRENT_DIR="$(dirname "$0")"
. "${CURRENT_DIR}"/LibConfig.sh
# check script parameter
if [ "${PATH_TO_E2STUDIO}" == "" ]; then
  PATH_TO_E2STUDIO="/home/plcdev/.local/share/renesas/e2_studio/eclipse/e2studio"
fi
if [ "${1}" == "" ]; then
  TEAMCITY_AGENT_WORK_DIR="/home/plcdev/dev"
else
  TEAMCITY_AGENT_WORK_DIR="${1}"
fi
if [ "${2}" == "" ]; then
  TEAMCITY_BUILD_WORKING_DIR="$(lib_config::get_teamcity_build_working_dir)"
else
  TEAMCITY_BUILD_WORKING_DIR="${2}"
fi
cd "${TEAMCITY_BUILD_WORKING_DIR}" || exit 1

if [ "${3}" == "" ]; then
  ECLIPSE_BUILD_CONFIG="$(lib_config::get_eclipse_build_config_default)"
else
  ECLIPSE_BUILD_CONFIG="${3}"
fi

ECLIPSE_IMPORT_WORKSPACE="${TEAMCITY_AGENT_WORK_DIR}"/workspaceBuild
# set return code of script
RC_BUILD_ON_CLIENT=0
# ------------------------------------------------------------
# e2studio shows a warning in headless mode on sl4j; to get it off redirect error messages of eclipse to /dev/null
# "${PATH_TO_E2STUDIO}" -nosplash -debug -consolelog -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
#   -data "${TEAMCITY_AGENT_WORK_DIR}"/workspaceBuild \
#   -import "${TEAMCITY_BUILD_WORKING_DIR}" \
#   -build all
"${PATH_TO_E2STUDIO}" \
  --launcher.suppressErrors \
  -nosplash \
  -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
  -data "${ECLIPSE_IMPORT_WORKSPACE}" \
  -import "${TEAMCITY_BUILD_WORKING_DIR}" \
  -cleanBuild "PLCF/${ECLIPSE_BUILD_CONFIG}" \
  2>/dev/null
RC_BUILD_ON_CLIENT=$?

# disable ignore off build errors
## if the eclipse build ends with errors and there is a mot file we ignore the errors for now and proceed
#if [ "${RC_BUILD_ON_CLIENT}" == "1" ]; then
#  FIRMWARE_FILENAME_WITH_PATH="$(lib_config::get_firmware_filename_with_path "${TEAMCITY_BUILD_WORKING_DIR}")"
#  if [ "${FIRMWARE_FILENAME_WITH_PATH}" != "" ]; then
#    echo "Reset error from build to proceed, because found mot file ${FIRMWARE_FILENAME_WITH_PATH}"
#    RC_BUILD_ON_CLIENT=0
#  else
#    echo "No mot file found!"
#  fi
#fi

# remove eclipse import workspace
rm -f -r "${ECLIPSE_IMPORT_WORKSPACE}"

exit $RC_BUILD_ON_CLIENT
