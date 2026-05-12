#!/bin/bash

check_exe() {
  if [[ ! -f ${1} ]]; then
   echo -n "No '${1}' "
   echo -n "found (try to download jenkins artifacts or "
   echo "build `basename ${1}`' manually"
   exit 1;
  fi
  if [[ ! -x ${1} ]]; then
    chmod +x ${1}
    echo "Execution permission added to ${1} "
  fi
}

# Main
APP_SHELL=${APP_SHELL-${PWD}/app_shell/app_shell}
ENACT_BROWSER=${ENACT_BROWSER-${PWD}/enact_browser_app_shell/dist/}
USER_DATA_DIR=${USER_DATA_DIR-/tmp/app_shell}

echo "APP_SHELL :" ${APP_SHELL}
echo "ENACT_BROWSER :" ${ENACT_BROWSER}
echo "USER_DATA_DIR :" ${USER_DATA_DIR}
echo "\${@} :" ${@}

check_exe ${APP_SHELL}

set -x
exec -a enact_browser "${APP_SHELL}" --start-fullscreen --ignore-gpu-blocklist --user-data-dir="${USER_DATA_DIR}" --load-apps="${ENACT_BROWSER}" "${@}"
