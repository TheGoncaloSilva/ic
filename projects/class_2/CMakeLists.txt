cmake_minimum_required(VERSION 3.16)
project(image_processing)
enable_testing()

##############################################################################
###################           Compilation Flags            ###################
##############################################################################

#SET (CMAKE_BUILD_TYPE "Release")
SET (CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED True)
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
#SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

##############################################################################
###################     Get Google Test automatically      ###################
##############################################################################

# Get Google test
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})

# Get Google test automatically
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

##############################################################################
###################            Other Packages              ###################
##############################################################################

# Use the usually pre-installed one
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_library(sndfile libsndfile)

##############################################################################
###################          Variables and DIRS            ###################
##############################################################################

SET (BASE_DIR ${CMAKE_SOURCE_DIR} )
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/bin)
SET (TEST_FOLDER ${BASE_DIR}/test)
# Source Code Folder
SET (SOURCE_FOLDER ${BASE_DIR}/src)

# Header folder
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

##############################################################################
###################           Executable files             ###################
##############################################################################

add_executable(system_test ${SOURCE_FOLDER}/systemCheck.cpp)
target_link_libraries( system_test ${OpenCV_LIBS} )

add_executable(extract_image ${SOURCE_FOLDER}/extract_image.cpp)
target_link_libraries( extract_image ${OpenCV_LIBS} )

add_executable(negative_image ${SOURCE_FOLDER}/negative_image.cpp)
target_link_libraries( negative_image ${OpenCV_LIBS} )

add_executable(mirrored_image ${SOURCE_FOLDER}/mirrored_image.cpp)
target_link_libraries( mirrored_image ${OpenCV_LIBS} )

add_executable(rotated_image ${SOURCE_FOLDER}/rotated_image.cpp)
target_link_libraries( rotated_image ${OpenCV_LIBS} )

add_executable(bright_image ${SOURCE_FOLDER}/bright_image.cpp)
target_link_libraries( bright_image ${OpenCV_LIBS} )

add_executable(golomb_encoder ${SOURCE_FOLDER}/golomb.cpp ${SOURCE_FOLDER}/golomb_codec.cpp ${SOURCE_FOLDER}/golomb_encoder.cpp)
target_link_libraries( golomb_encoder ${OpenCV_LIBS} sndfile )

add_executable(golomb_decoder ${SOURCE_FOLDER}/golomb.cpp ${SOURCE_FOLDER}/golomb_codec.cpp ${SOURCE_FOLDER}/golomb_decoder.cpp)
target_link_libraries( golomb_decoder ${OpenCV_LIBS} sndfile )

##############################################################################
###################                 Tests                  ###################
##############################################################################

set(TEST_SOURCES
    ${SOURCE_FOLDER}/golomb.cpp
    ${TEST_FOLDER}/test_golomb.cpp
    ${TEST_FOLDER}/test_bitStream.cpp
    ${SOURCE_FOLDER}/golomb_codec.cpp
    ${TEST_FOLDER}/test_predictor.cpp
    ${TEST_FOLDER}/test_gencoder.cpp
    )

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests gtest)
target_link_libraries(tests gtest_main)
target_link_libraries(tests ${OpenCV_LIBS})
add_test(tests tests)
