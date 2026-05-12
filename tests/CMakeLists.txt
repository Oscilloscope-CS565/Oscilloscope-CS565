include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Top-level project sets CMAKE_AUTOMOC ON for Qt; disable for GTest and our pure C++ test binary.
foreach(_osc_gtest_tgt gtest gtest_main gmock gmock_main)
  if(TARGET ${_osc_gtest_tgt})
    set_target_properties(${_osc_gtest_tgt} PROPERTIES AUTOMOC OFF)
  endif()
endforeach()

add_executable(io_library_tests
  test_circular_buffer.cpp
  test_scale_shift_pipeline.cpp
  ${CMAKE_SOURCE_DIR}/ioLibrary/ioCircularBuffer.cpp
  ${CMAKE_SOURCE_DIR}/ioLibrary/ioScaleShiftPipeline.cpp
)

target_include_directories(io_library_tests PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/ioLibrary
)

target_link_libraries(io_library_tests PRIVATE
  GTest::gtest_main
)

set_target_properties(io_library_tests PROPERTIES AUTOMOC OFF)

if(UNIX AND NOT APPLE)
  target_link_libraries(io_library_tests PRIVATE pthread)
endif()

include(GoogleTest)
gtest_discover_tests(io_library_tests)
