#!/bin/sh
set -eu
for case in host-as-user kernel-as-host user-as-kernel host-library-as-user unknown-default-crt unknown-bootstrap-crt non-user-crt unknown-user-library unknown-system-library mixed-system-default explicit-libc-in-app-list; do
  if make -s -f tests/mk-backends/invalid.mk "CASE=$case" -n > /tmp/plan9-backend-invalid-$$.log 2>&1; then
    echo "FAIL: invalid target $case was accepted" >&2
    exit 1
  fi
  echo "PASS rejected: $case"
done
rm -f /tmp/plan9-backend-invalid-$$.log
