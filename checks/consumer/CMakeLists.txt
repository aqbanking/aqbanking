# Builds consumer.c against the installed AqBanking for "make installcheck".
# Pass -Daqbanking_DIR=<directory of the package files>,
# -DCMAKE_PREFIX_PATH=<Gwenhywfar's prefix> and
# -DAQBANKING_EXPECTED_VERSION=<version>.
cmake_minimum_required(VERSION 3.10)
project(aqbanking_consumer C)

find_package(aqbanking ${AQBANKING_EXPECTED_VERSION} EXACT REQUIRED)

add_executable(consumer consumer.c)
target_link_libraries(consumer PRIVATE aqbanking::aqbanking)

enable_testing()
add_test(NAME consumer COMMAND consumer)
