#!/bin/sh
set -eu
# A private bus and in-memory settings isolate accessibility activation from user preferences.
export GSETTINGS_BACKEND=memory
exec dbus-run-session -- sh -eu -c '
    gdbus call --session --dest org.a11y.Bus --object-path /org/a11y/bus \
        --method org.freedesktop.DBus.Properties.Set org.a11y.Status IsEnabled "<true>"
    exec xvfb-run -a "$@"
' secondbrain-atspi "$@"
