#!/usr/bin/env bash
set -euo pipefail
configuration="${1:-Debug}"
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
bash "$project_root/scripts/build.sh" "$configuration" "${2:-System}"
ctest --test-dir "$project_root/build/$configuration" -C "$configuration" --output-on-failure
