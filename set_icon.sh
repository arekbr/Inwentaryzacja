#!/bin/bash
EXECUTABLE=$1
ICON=$2
gio set "$EXECUTABLE" metadata::custom-icon "file://$ICON"
