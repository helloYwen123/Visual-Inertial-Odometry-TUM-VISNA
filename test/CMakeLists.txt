cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)



add_subdirectory(googletest EXCLUDE_FROM_ALL)
# ensure gtest include directory is added before any of the
# system-wide include directories that might contain gtest
include_directories(BEFORE ${gtest_SOURCE_DIR}/include)

find_package(fmt REQUIRED)

include_directories(${fmt_INCLUDE_DIRS})


include(GoogleTest)

enable_testing()


# NOTE: We add ceres to tests, in case students use ceres::sin etc in camera model templates.

#add_executable(test_ex0 src/test_ex0.cpp)
#target_link_libraries(test_ex0 gtest gtest_main Ceres::ceres)

#add_executable(test_ex1 src/test_ex1.cpp)
#target_link_libraries(test_ex1 gtest gtest_main Ceres::ceres Sophus::Sophus)

# add_executable(test_ex2 src/test_ex2.cpp)
# target_link_libraries(test_ex2 gtest gtest_main Ceres::ceres Sophus::Sophus)

#add_executable(test_ex3 src/test_ex3.cpp)
#target_link_libraries(test_ex3 gtest gtest_main Ceres::ceres Sophus::Sophus pango_image TBB::tbb OpenCV opengv)

#add_executable(test_ex4 src/test_ex4.cpp)
#target_link_libraries(test_ex4 gtest gtest_main Ceres::ceres Sophus::Sophus opengv TBB::tbb)

 #add_executable(test_ex5 src/test_ex5.cpp)
 #target_link_libraries(test_ex5 gtest gtest_main Ceres::ceres Sophus::Sophus opengv TBB::tbb)

#  add_executable(test_imu_dataloader src/test_imu_dataloader.cpp)
#  target_link_libraries(test_imu_dataloader gtest gtest_main Ceres::ceres fmt::fmt)

#gtest_discover_tests(test_ex0 DISCOVERY_TIMEOUT 120)

#gtest_discover_tests(test_ex1 DISCOVERY_TIMEOUT 120)
#gtest_discover_tests(test_ex2 DISCOVERY_TIMEOUT 120)
#add_test(NAME ex2_calibration_test COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_ex2.sh WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
#gtest_discover_tests(test_ex3 DISCOVERY_TIMEOUT 120)
#gtest_discover_tests(test_ex4 DISCOVERY_TIMEOUT 120)
#gtest_discover_tests(test_ex5 DISCOVERY_TIMEOUT 120)
#gtest_discover_tests(test_imu_dataloader DISCOVERY_TIMEOUT 120)
