#!/usr/bin/env bash
set -euo pipefail

for cmd in groff ghostscript; do
    if ! command -v "$cmd" &>/dev/null; then
        echo "ERROR: '$cmd' is not installed or not in PATH" >&2
        exit 1
    fi
done

find /data -type f -regex '.*\.[0-9]' | while read -r f; do
    echo "MAN2PDF: Converting: $f"
    groff -t -man -Tpdf "$f" > "${f}.pdf"
done