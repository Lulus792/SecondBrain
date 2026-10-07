#!/usr/bin/env python3
"""Start checked CTest with an entry log before CMake/CTest can fail.

Developer/CI tool. Native argument passing avoids shell path conversion on Windows.
"""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--test-dir', required=True)
    parser.add_argument('--config', required=True)
    parser.add_argument('--log', required=True)
    args = parser.parse_args()
    directory = Path(args.test_dir).resolve()
    log_path = Path(args.log)
    log_path.parent.mkdir(parents=True, exist_ok=True)
    cmake = shutil.which('cmake')
    with log_path.open('wb') as log:
        def write(data):
            log.write(data)
            log.flush()
            sys.stdout.buffer.write(data)
            sys.stdout.buffer.flush()

        write(('Checked test entry\nPlatform: ' + sys.platform + '\nDirectory: ' +
               str(directory) + '\nConfiguration: ' + args.config + '\nCMake: ' +
               str(cmake) + '\n').encode('utf-8'))
        if not cmake:
            write(b'Failed before CTest: CMake not found\n')
            return 1
        command = [cmake, '-DSB_TEST_DIR=' + directory.as_posix(),
                   '-DSB_TEST_CONFIG=' + args.config, '-P',
                   str(Path(__file__).resolve().with_name('test_checked.cmake'))]
        try:
            with subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT) as process:
                while True:
                    chunk = process.stdout.read1(8192)
                    if not chunk:
                        break
                    write(chunk)
                result = process.wait()
        except OSError as error:
            write(('Failed before CTest: ' + str(error) + '\n').encode('utf-8'))
            return 1
        write(('CMake exit status: ' + str(result) + '\n').encode('utf-8'))
        return result if result >= 0 else 1


if __name__ == '__main__':
    sys.exit(main())
