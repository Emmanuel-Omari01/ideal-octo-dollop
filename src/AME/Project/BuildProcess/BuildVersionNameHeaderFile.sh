#!/bin/bash
# ------------------------------------------------------------
# BuildVersionNameHeaderFile.sh
# author: Tobias Hirsch, jja
# ------------------------------------------------------------
FILE_NAME_VERSION_NAME_HEADER=VersionName.h
TARGET_PATH_OF_VERSION_NAME_HEADER=../../Project/SystemBase
CURRENT_DIR="$(dirname "$0")"

echo "Build / update ${FILE_NAME_VERSION_NAME_HEADER} in ${TARGET_PATH_OF_VERSION_NAME_HEADER}"
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
CURRENT_TIME=$(date +"%d.%m.%Y %H:%M:%S")
echo "with ${CURRENT_BRANCH} ${CURRENT_TIME}"
cd "${CURRENT_DIR}" || exit 1
{
  echo "/* AUTO-GENERATED FILE -- DO NOT EDIT. */" ;
  echo "const char* mxSoftwareVersion = " ;
  echo '"'"${CURRENT_BRANCH}" '"' ;
  echo '"'"${CURRENT_TIME}"'";' ;
} >"${FILE_NAME_VERSION_NAME_HEADER}"

mv -f "${FILE_NAME_VERSION_NAME_HEADER}" ${TARGET_PATH_OF_VERSION_NAME_HEADER}
