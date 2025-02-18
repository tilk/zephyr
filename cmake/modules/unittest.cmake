# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

enable_language(C CXX ASM)

include(root)
include(boards)
include(arch)
include(configuration_files)
include(kconfig)

# Parameters:
#   SOURCES: list of source files, default main.c
#   INCLUDE: list of additional include paths relative to ZEPHYR_BASE

foreach(extra_flags EXTRA_CPPFLAGS EXTRA_LDFLAGS EXTRA_CFLAGS EXTRA_CXXFLAGS EXTRA_AFLAGS)
  list(LENGTH ${extra_flags} flags_length)
  if(flags_length LESS_EQUAL 1)
    # A length of zero means no argument.
    # A length of one means a single argument or a space separated list was provided.
    # In both cases, it is safe to do a separate_arguments on the argument.
    separate_arguments(${extra_flags}_AS_LIST UNIX_COMMAND ${${extra_flags}})
  else()
    # Already a proper list, no conversion needed.
    set(${extra_flags}_AS_LIST "${${extra_flags}}")
  endif()
endforeach()

set(ENV_ZEPHYR_BASE $ENV{ZEPHYR_BASE})
# This add support for old style boilerplate include.
if((NOT DEFINED ZEPHYR_BASE) AND (DEFINED ENV_ZEPHYR_BASE))
  set(ZEPHYR_BASE ${ENV_ZEPHYR_BASE} CACHE PATH "Zephyr base")
endif()

if(NOT SOURCES)
  set(SOURCES main.c)
endif()

add_executable(testbinary ${SOURCES})

set(KOBJ_TYPES_H_TARGET kobj_types_h_target)
include(${ZEPHYR_BASE}/cmake/kobj.cmake)
add_dependencies(testbinary ${KOBJ_TYPES_H_TARGET})
gen_kobj(KOBJ_GEN_DIR)

list(APPEND INCLUDE
  subsys/testsuite/ztest/include/zephyr
  subsys/testsuite/include/zephyr
  subsys/testsuite/ztest/include
  subsys/testsuite/include
  include/zephyr
  include
  .
)

if(CMAKE_HOST_APPLE)
else()

if(M64_MODE)
set (CMAKE_C_FLAGS "-m64")
set (CMAKE_CXX_FLAGS "-m64")
else()
set (CMAKE_C_FLAGS "-m32") #deprecated on macOS
set (CMAKE_CXX_FLAGS "-m32") #deprecated on macOS
endif(M64_MODE)

endif()

target_compile_options(testbinary PRIVATE
  -imacros ${AUTOCONF_H}
  -Wall
  -I ${KOBJ_GEN_DIR}
  ${EXTRA_CPPFLAGS_AS_LIST}
  ${EXTRA_CFLAGS_AS_LIST}
  $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXXFLAGS_AS_LIST}>
  $<$<COMPILE_LANGUAGE:ASM>:${EXTRA_AFLAGS_AS_LIST}>
  )

target_link_options(testbinary PRIVATE
  -T "${ZEPHYR_BASE}/subsys/testsuite/include/zephyr/ztest_unittest.ld"
  )

target_link_libraries(testbinary PRIVATE
  ${EXTRA_LDFLAGS_AS_LIST}
  )

if(COVERAGE)
  target_compile_options(testbinary PRIVATE
    -fno-default-inline
    -fno-inline
    -fprofile-arcs
    -ftest-coverage
    )

  target_link_libraries(testbinary PRIVATE
    -lgcov
    )
endif()

if(LIBS)
  message(FATAL_ERROR "This variable is not supported, see SOURCES instead")
endif()

if(CONFIG_ZTEST_NEW_API)
  target_sources(testbinary PRIVATE
      ${ZEPHYR_BASE}/subsys/testsuite/ztest/src/ztest_new.c
      ${ZEPHYR_BASE}/subsys/testsuite/ztest/src/ztest_mock.c
      ${ZEPHYR_BASE}/subsys/testsuite/ztest/src/ztest_rules.c
      ${ZEPHYR_BASE}/subsys/testsuite/ztest/src/ztest_defaults.c
      )
else()
  target_sources(testbinary PRIVATE
      ${ZEPHYR_BASE}/subsys/testsuite/ztest/src/ztest.c
      ${ZEPHYR_BASE}/subsys/testsuite/ztest/src/ztest_mock.c
      )
endif()

target_compile_definitions(testbinary PRIVATE ZTEST_UNITTEST)

foreach(inc ${INCLUDE})
  target_include_directories(testbinary PRIVATE ${ZEPHYR_BASE}/${inc})
endforeach()

find_program(VALGRIND_PROGRAM valgrind)
if(VALGRIND_PROGRAM)
  set(VALGRIND ${VALGRIND_PROGRAM})
  set(VALGRIND_FLAGS
    --leak-check=full
    --error-exitcode=1
    --log-file=valgrind.log
    )
endif()

add_custom_target(run-test
  COMMAND
  ${VALGRIND} ${VALGRIND_FLAGS}
  $<TARGET_FILE:testbinary>
  DEPENDS testbinary
  WORKING_DIRECTORY ${APPLICATION_BINARY_DIR}
  )
# TODO: Redirect output to unit.log
