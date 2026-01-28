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
BROWSER_SHELL=${BROWSER_SHELL-${PWD}/browser_shell/browser_shell}
ENACT_BROWSER=${ENACT_BROWSER-${PWD}/dist/index.html}
APP_ID=${APP_ID-com.webos.app.test.browsershell}
SHELL_WEB_APIS=${SHELL_WEB_APIS-v8/browser_shell,v8/chrome_extensions}
USER_DATA_DIR=${USER_DATA_DIR-/tmp/browser_shell}

echo "BROWSER_SHELL :" ${BROWSER_SHELL}
echo "ENACT_BROWSER :" ${ENACT_BROWSER}
echo "APP_ID :" ${APP_ID}
echo "SHELL_WEB_APIS :" ${SHELL_WEB_APIS}
echo "USER_DAT_DIR: " ${USER_DATA_DIR}
echo "\${@} :" ${@}

check_exe ${BROWSER_SHELL}

set -x
exec -a enact_browser "${BROWSER_SHELL}" --shell-app-path="file://${ENACT_BROWSER}" --shell-app-id="${APP_ID}" --disable-web-security --user-data-dir="${USER_DATA_DIR}" --shell-web-apis="${SHELL_WEB_APIS}" "${@}"
