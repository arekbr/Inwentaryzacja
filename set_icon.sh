#!/bin/bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
  echo "Uzycie: $0 <sciezka_do_pliku> <sciezka_do_ikony>" >&2
  exit 1
fi

if ! command -v gio >/dev/null 2>&1; then
  echo "Brak polecenia 'gio' w PATH." >&2
  exit 1
fi

EXECUTABLE="$1"
ICON="$2"

if [[ ! -e "$EXECUTABLE" ]]; then
  echo "Plik nie istnieje: $EXECUTABLE" >&2
  exit 1
fi

if [[ ! -f "$ICON" ]]; then
  echo "Ikona nie istnieje: $ICON" >&2
  exit 1
fi

ABS_ICON="$(readlink -f "$ICON")"
gio set "$EXECUTABLE" metadata::custom-icon "file://$ABS_ICON"
