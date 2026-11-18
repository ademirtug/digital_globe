#!/usr/bin/env bash
set -euo pipefail
configuration="${1:-Debug}"
case "$configuration" in Debug|Release) ;; *) echo 'Choose Debug or Release.' >&2; exit 2 ;; esac
provider="${2:-SYSTEM}"
case "$provider" in System|SYSTEM) provider=SYSTEM ;; Bundled|BUNDLED) provider=BUNDLED ;; *) echo 'Choose System or Bundled for wxWidgets.' >&2; exit 2 ;; esac
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$project_root" -B "$project_root/build/$configuration" -DCMAKE_BUILD_TYPE="$configuration" -DDIGITAL_GLOBE_BUILD_TESTS=ON -DDIGITAL_GLOBE_WX_PROVIDER="$provider"
cmake --build "$project_root/build/$configuration" --parallel
