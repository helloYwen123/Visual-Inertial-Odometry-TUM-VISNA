cmake_minimum_required(VERSION 3.10)

project(visnav)

# FindTBB
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/" ${CMAKE_MODULE_PATH})

# find eigen with our adapted "config module", that uses our eigen source tree
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/eigen3" ${CMAKE_PREFIX_PATH})

set(EIGEN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eigen")
set(CERES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/build-ceres-solver")
set(PANGOLIN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/build-Pangolin")
set(OPENGV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/build-opengv")

# Set default build type if not specified otherwise.
# See https://cmake.org/pipermail/cmake/2012-May/050243.html
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified.")
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

# Configure CCache if available
if (NOT CMAKE_C_COMPILER_LAUNCHER AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
  else()
    message(STATUS "Dind't find ccache")
  endif()
else()
  message(STATUS "Compiler launcher already set. Not configuring ccache.")
  message(STATUS "CMAKE_C_COMPILER_LAUNCHER: ${CMAKE_C_COMPILER_LAUNCHER}")
  message(STATUS "CMAKE_CXX_COMPILER_LAUNCHER: ${CMAKE_CXX_COMPILER_LAUNCHER}")
endif()

# See comment in ./build_submodules.sh on the relation of arch=native
# and Eigen's alignment. TLDR: Passing arch=native for all build types
# is currently the only viable option to avoid subtle bugs with Eigen.
if(NOT CXX_MARCH)
  set(CXX_MARCH native)
endif()

IF(NOT APPLE OR NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
  set(VISNAV_MARCH_FLAGS "-march=${CXX_MARCH}")
ELSE()
  message(STATUS "Running on Apple ${CMAKE_SYSTEM_PROCESSOR}. Disabled -march flag.")
ENDIF()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# for some reason clang doesn't include them in Wall whereas gcc does, so include them explicitly.
SET(EXTRA_WARNING_FLAGS "-Wsign-compare")

# clang-specific compile flags to avoid warnings from thirdpary includes
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  set(EXTRA_WARNING_FLAGS "${EXTRA_WARNING_FLAGS} -Wno-exceptions")
endif()

set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g -DEIGEN_INITIALIZE_MATRICES_BY_NAN")                   # cmake default: "-g"
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -g -DEIGEN_INITIALIZE_MATRICES_BY_NAN")  # cmake default: "-O2 -g -DNDEBUG"
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")                                               # cmake default: "-O3 -DNDEBUG"
SET(CMAKE_CXX_FLAGS " -ftemplate-backtrace-limit=0 -Wall -Wextra ${EXTRA_WARNING_FLAGS} ${VISNAV_MARCH_FLAGS} ${CMAKE_CXX_FLAGS}")


set(EIGEN3_INCLUDE_DIR_HINTS ${EIGEN_ROOT})  # for opengv / pangolin
find_package(Eigen3 3.4.0 EXACT REQUIRED CONFIG) # find eigen with our adapted "config module", that uses our eigen source tree
set(EIGEN_INCLUDE_DIR ${EIGEN3_INCLUDE_DIR})  # for opengv / pangolin
include_directories(${EIGEN3_INCLUDE_DIR})
message(STATUS "Found Eigen headers in: ${EIGEN3_INCLUDE_DIR}")

option(BUILD_SOPHUS_TESTS "Build tests." OFF)
option(BUILD_SOPHUS_EXAMPLES "Build examples." OFF)
add_subdirectory(thirdparty/Sophus)

include_directories(thirdparty/cereal/include)
include_directories(thirdparty/CLI11/include)
include_directories(include)

# to use link to target "ceres"
# Note: always link to ceres first, as otherwise a system-wide install might
# be found before our custom build. For example on macOS a both pangolin and
# TBB have /usr/local/include as part of their include directories.
find_package(Ceres 2.0 REQUIRED CONFIG HINTS "${CERES_ROOT}" NO_DEFAULT_PATH)

# to use link to target "pangolin"
set(Pangolin_DIR ${PANGOLIN_ROOT})
find_package(Pangolin REQUIRED CONFIG NO_DEFAULT_PATH)
message(STATUS "Found Pangolin headers in: ${Pangolin_INCLUDE_DIR}")

# to use link to target "opengv"
find_package(opengv REQUIRED CONFIG HINTS ${OPENGV_ROOT} NO_DEFAULT_PATH)
message(STATUS "Found OpenGV headers in: ${opengv_DIR}")

# to use link to target "TBB"
find_package(TBB REQUIRED)
message(STATUS "Found TBB ${TBB_VERSION_MAJOR}.${TBB_VERSION_MINOR} (interface version ${TBB_INTERFACE_VERSION}) headers in: ${TBB_INCLUDE_DIRS}")


# to use link to target "OpenCV"
# NOTE: not specifying version, since 2, 3 or 4 is fine
find_package(OpenCV REQUIRED COMPONENTS core features2d)
message(STATUS "Found OpenCV ${OpenCV_VERSION} headers in: ${OpenCV_INCLUDE_DIRS}")
add_library(OpenCV INTERFACE IMPORTED)
set_property(TARGET OpenCV PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OpenCV_INCLUDE_DIRS})
set_property(TARGET OpenCV PROPERTY INTERFACE_LINK_LIBRARIES ${OpenCV_LIBS})
if(OpenCV_VERSION VERSION_LESS 3)
  set_property(TARGET OpenCV PROPERTY INTERFACE_COMPILE_DEFINITIONS DBATK_OPENCV2)
endif()

# Add sources as custom target so that they are shown in IDE's
add_custom_target(visnav_other SOURCES
  include/visnav/aprilgrid.h
  include/visnav/bow_db.h
  include/visnav/bow_voc.h
  include/visnav/calibration.h
  include/visnav/camera_models.h
  include/visnav/common_types.h
  include/visnav/ex1.h
  include/visnav/global.h
  include/visnav/gui_helper.h
  include/visnav/hash.h
  include/visnav/keypoints.h
  include/visnav/local_parameterization_se3.hpp
  include/visnav/map_utils.h
  include/visnav/matching_utils.h
  include/visnav/reprojection.h
  include/visnav/serialization.h
  include/visnav/tracks.h
  include/visnav/union_find.h
  include/visnav/vo_utils.h
)



#add_executable(test_ceres_se3 src/test_ceres_se3.cpp)
#target_link_libraries(test_ceres_se3 Ceres::ceres Sophus::Sophus)

# add_executable(calibration src/calibration.cpp)
# target_link_libraries(calibration Ceres::ceres Sophus::Sophus pango_display pango_image pango_plot pango_video TBB::tbb)

#add_executable(sfm src/sfm.cpp)
#target_link_libraries(sfm Ceres::ceres Sophus::Sophus pango_display pango_image pango_plot pango_video TBB::tbb OpenCV opengv)

add_executable(odometry src/odometry.cpp)
target_link_libraries(odometry Ceres::ceres Sophus::Sophus pango_display pango_image pango_plot pango_video TBB::tbb OpenCV opengv)



enable_testing()
add_subdirectory(test)

