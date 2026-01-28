#!/bin/bash

TESTPORT=${TESTPORT:-8888}
SLEEPSTEP=${SLEEPSTEP:-.5}
BACKOFFTRIES=${BACKOFFTRIES:-10}
CUR_DIR=$PWD

cleanup() {
    echo "TestServer (PID=$TESTSRVPID) will be terminated"
    kill -TERM $TESTSRVPID
    exit ${1:-0}
}

trap cleanup SIGINT

with_backoff() {
  "$@" ||
  for backoff in `seq 1 $BACKOFFTRIES`
  do
    sleep $SLEEPSTEP
    "$@" && break
  done
}

while read bin package
do
    [[ -z `which $bin` ]] && {
        echo >&2 "$bin is not installed: try 'sudo apt-get install $package'"
        exit 1
    }
done <<EOF
fuser psmisc
EOF

echo "Serving on port $TESTPORT..."
uwsgi -M --die-on-term --wsgi-file tests_server.py --http-socket :$TESTPORT \
--pyargv $CUR_DIR/../tests --static-map /media=../tests/media \
--honour-range --add-header "Accept-Ranges: bytes" &

TESTSRVPID=$!

# Waiting for a while to get the server started
with_backoff fuser -n tcp $TESTPORT -s || {
    echo >&2 "Can not start TestServer, exiting..."
    cleanup 1
}

echo "TestServer (PID=$TESTSRVPID)"
echo "has been started and configured. Type [q|Q] to terminate..."

grep -qLiw q

cleanup
