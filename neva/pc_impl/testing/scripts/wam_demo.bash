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

check_tool() {
    `which ${1} > /dev/null`
    if [[ ! ${?} ]]; then
      echo "No '${1}' found (try to apt-get install ${2:-$1})"
      exit 1
    fi
}

check_tool_apt() {
    if [[ ! `apt list | grep ${1} | wc -l` ]]; then
      echo "No '${1}' found (try to apt-get install ${2:-$1})"
      exit 1;
    fi
}

kill_processes() {
  echo kill -9 ${chromium_pid} ${wam_demo_pid} ${mock_server_pid}
  killall uwsgi
  kill -9 ${chromium_pid} ${wam_demo_pid} ${mock_server_pid}
}

check_launch_error() {
  if [[ ${?} = 0 ]]; then
    echo ${1} launched
  else
    echo ${1} launch error
    kill_processes
    exit
  fi
}

trap 'rm -fR "$USER_DATA_DIR"' EXIT

WAM_EMULATOR_PAGE=${WAM_EMULATOR_PAGE-http://localhost:8888/wam_emulator_page_new.html}
USER_DATA_DIR=${USER_DATA_DIR-$(mktemp -d /tmp/$$.XXXXXX)}
PIPE=${PIPE-${USER_DATA_DIR}/pipe}
WAM_DEMO=${WAM_DEMO-${PWD}/wam_example/wam_demo}
DEFAULT_WAM_DEMO_FLAG="--ignore-gpu-blocklist"
CHROMIUM=${CHROMIUM-${PWD}/chrome_gcc/chrome}
MOCK_SERVER=${MOCK_SERVER-${PWD}/testing/mock_server/run_emulator_server.bash}
TESTS_SERVER=${TESTS_SERVER-${PWD}/testing/mock_server/tests_server.py}

echo "WAM_EMULATOR_PAGE :" ${WAM_EMULATOR_PAGE}
echo "USER_DATA_DIR :" ${USER_DATA_DIR}
echo "PIPE :" ${PIPE}
echo "WAM_DEMO :" ${WAM_DEMO}
echo "DEFAULT_WAM_DEMO_FLAG :" ${DEFAULT_WAM_DEMO_FLAG}
echo "CHROMIUM :" ${CHROMIUM}
echo "MOCK_SERVER :" ${MOCK_SERVER}
echo "TESTS_SERVER :" ${TESTS_SERVER}
echo "\${@} :" ${@}

for exe in "${WAM_DEMO}" "${MOCK_SERVER}" "${TESTS_SERVER}" "${CHROMIUM}"
do
  check_exe ${exe}
done

check_tool weston

mkfifo ${PIPE}
(
  cd $(dirname "${MOCK_SERVER}")
  exec "${MOCK_SERVER}"
) <> ${PIPE} & mock_server_pid=$!
check_launch_error ${MOCK_SERVER}

set -x
"${WAM_DEMO}" ${DEFAULT_WAM_DEMO_FLAG} ${@} & wam_demo_pid=${!}
check_launch_error ${WAM_DEMO}

"${CHROMIUM}" --start-maximized --user-data-dir=${USER_DATA_DIR} ${WAM_EMULATOR_PAGE} & chromium_pid=${!}
check_launch_error ${CHROMIUM}

wait ${chromium_pid}
trap kill_processes EXIT
