#!/usr/bin/env python3
"""Check normal rendering on a private headless Wayland compositor."""
import argparse
from contextlib import contextmanager
import importlib.util
import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import sys
import tempfile
import time


@contextmanager
def compositor(command, environment, socket_path, log_path, timeout=15):
    with log_path.open('xb') as log:
        process = subprocess.Popen(command, env=environment, stdout=log, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + timeout
            while True:
                if process.poll() is not None:
                    raise RuntimeError('Wayland compositor exited before its socket was ready')
                if socket_path.exists() and stat.S_ISSOCK(socket_path.stat().st_mode):
                    break
                if time.monotonic() >= deadline:
                    raise RuntimeError('Wayland compositor socket did not become ready')
                time.sleep(0.05)
            yield process
            if process.poll() is not None:
                raise RuntimeError('Wayland compositor stopped during verification')
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=5)


def verify(app, cli, output):
    if not sys.platform.startswith('linux'):
        raise RuntimeError('Actual Wayland verification requires Linux')
    weston = shutil.which('weston')
    if not weston:
        raise RuntimeError('Weston is required for Wayland verification')
    output.mkdir(parents=True, exist_ok=False)
    help_text = subprocess.check_output([weston, '--backend=headless-backend.so', '--help'],
                                       stderr=subprocess.STDOUT, timeout=15).decode('utf-8')
    if '--renderer' in help_text:
        renderer = '--renderer=pixman'
    elif '--use-pixman' in help_text:
        renderer = '--use-pixman'
    else:
        raise RuntimeError('Weston does not advertise a supported software renderer option')
    version = subprocess.check_output([weston, '--version'], timeout=15).decode('utf-8').strip()
    spec = importlib.util.spec_from_file_location('runtime_probe',
        Path(__file__).resolve().parents[1]/'tests/test_runtime_probe.py')
    probe = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(probe)
    with tempfile.TemporaryDirectory(prefix='SecondBrain-wayland-') as temporary:
        runtime = Path(temporary)
        runtime.chmod(0o700)
        environment = dict(os.environ, XDG_RUNTIME_DIR=str(runtime),
                           WAYLAND_DISPLAY='secondbrain-wayland', SDL_VIDEO_DRIVER='wayland',
                           XDG_SESSION_TYPE='wayland', GSETTINGS_BACKEND='memory')
        for key in ('DISPLAY', 'SDL_VIDEODRIVER', 'SDL_RENDER_DRIVER'):
            environment.pop(key, None)
        command = [weston, '--backend=headless-backend.so', renderer,
                   '--shell=kiosk-shell.so', '--socket=secondbrain-wayland',
                   '--idle-time=0', '--no-config', '--width=1280', '--height=800',
                   '--log='+str(output/'weston.log')]
        (output/'compositor.json').write_text(json.dumps({
            'weston_version': version, 'command': command,
            'scope': 'headless-compositor-without-X11-fallback; no physical-device or human proof'},
            indent=2)+'\n')
        with compositor(command, environment, runtime/'secondbrain-wayland', output/'weston-output.log'):
            probe.verify(app, cli, output/'native-runtime', 'wayland', environment)
    print('Wayland normal-renderer probe passed; compositor stopped')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--app', required=True, type=Path)
    parser.add_argument('--cli', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    verify(args.app.resolve(), args.cli.resolve(), args.output.resolve())
