# -*- cmake -*-

find_package(OpenMP)

find_package(ZLIB)

# Use included boost unless explicitly specified
if (NOT SPADES_BOOST_ROOT)
  set(BOOST_ROOT "${EXT_DIR}/include")
else()
  set(BOOST_ROOT SPADES_BOOST_ROOT)
endif()
set(Boost_USE_MULTITHREADED ON)
find_package(Boost)

# Build external dependencies (if any)
add_subdirectory("${EXT_DIR}/src" "${Project_BINARY_DIR}/ext")
