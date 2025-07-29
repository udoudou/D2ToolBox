#!/usr/bin/env python
#
# SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
import argparse
import os
import subprocess
import sys
from typing import Iterable
from typing import List


def _check_git_filemode(full_path: str) -> bool:
    try:
        stdout = subprocess.check_output(['git', 'ls-files', '--stage', full_path]).strip().decode('utf-8')
    except subprocess.CalledProcessError:
        return True

    mode = stdout.split(' ', 1)[0]  # e.g. 100644 for a rw-r--r--
    if any([int(i, 8) & 1 for i in mode[-3:]]):
        return True
    return False


def is_executable(full_path: str) -> bool:
    """
    os.X_OK will always return true on windows. Use git to check file mode.
    :param full_path: file full path
    :return: True is it's an executable file
    """
    if sys.platform == 'win32':
        return _check_git_filemode(full_path)
    return os.access(full_path, os.X_OK)


def _strip_each_item(iterable: Iterable) -> List:
    res = []
    for item in iterable:
        if item:
            res.append(item.strip())
    return res


IDF_PATH = os.getenv('IDF_PATH', os.getcwd())
EXECUTABLE_LIST_FN = os.path.join(IDF_PATH, 'tools/ci/executable-list.txt')
known_executables = _strip_each_item(open(EXECUTABLE_LIST_FN).readlines())


def check_executable_list() -> int:
    ret = 0
    for index, fn in enumerate(known_executables):
        if not os.path.exists(os.path.join(IDF_PATH, fn)):
            print('{}:{} {} not exists. Please remove it manually'.format(EXECUTABLE_LIST_FN, index + 1, fn))
            ret = 1
    return ret


def check_executables(files: List) -> int:
    ret = 0
    for fn in files:
        fn_executable = is_executable(fn)
        fn_in_list = fn in known_executables
        if fn_executable and not fn_in_list:
            print('"{}" is not in {}'.format(fn, EXECUTABLE_LIST_FN))
            ret = 1
        if not fn_executable and fn_in_list:
            print('"{}" is not executable but is in {}'.format(fn, EXECUTABLE_LIST_FN))
            ret = 1
    return ret


def check() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('--file_list', type=str, help='file list txt')
    parser.add_argument('--action', choices=['executables', 'list'], required=True,
                        help='if "executables", pass all your executables to see if it\'s in the list.'
                             'if "list", check if all items on your list exist')
    parser.add_argument('filenames', nargs='*', help='Filenames to check.')
    args = parser.parse_args()

    if args.file_list:
        global EXECUTABLE_LIST_FN
        global known_executables
        EXECUTABLE_LIST_FN = args.file_list
        known_executables = _strip_each_item(open(EXECUTABLE_LIST_FN).readlines())

    if args.action == 'executables':
        ret = check_executables(args.filenames)
    elif args.action == 'list':
        ret = check_executable_list()
    else:
        raise ValueError

    return ret


if __name__ == '__main__':
    sys.exit(check())
