# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation

include(${TESTS_ROOT_DIR}/cmake/exec_functions.cmake)

setup()

execute(${EXECUTABLE} a ${DIR}/testfile)
run_with_gdb(${TESTS_ROOT_DIR}/integrity/append_break.gdb ${EXECUTABLE} b ${DIR}/testfile)
execute(${EXECUTABLE} i ${DIR}/testfile)

finish()
