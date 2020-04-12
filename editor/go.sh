#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

make -j$(nproc)
./mwgeditor
