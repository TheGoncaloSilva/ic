cmake_minimum_required(VERSION 3.16)
project(sndfile-example)

#SET (CMAKE_BUILD_TYPE "Release")
SET (CMAKE_BUILD_TYPE "Debug")

find_library(sndfile libsndfile)

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

SET (BASE_DIR ${CMAKE_SOURCE_DIR} )
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/../sndfile-example-bin)

add_executable (wav_cp wav_cp.cpp)
target_link_libraries (wav_cp sndfile)

add_executable (wav_quant wav_quant.cpp)
target_link_libraries (wav_quant sndfile)

add_executable (wav_cmp wav_cmp.cpp)
target_link_libraries (wav_cmp sndfile)

add_executable (wav_hist wav_hist.cpp)
target_link_libraries (wav_hist sndfile)

add_executable (wav_dct wav_dct.cpp)
target_link_libraries (wav_dct sndfile fftw3)

add_executable (wav_effects wav_effects.cpp)
target_link_libraries (wav_effects sndfile fftw3)

add_executable (encoder encoder.cpp)
target_link_libraries (encoder sndfile)

add_executable (decoder decoder.cpp)
target_link_libraries (decoder sndfile)

add_executable (encoder_lossy encoder_lossy.cpp)
target_link_libraries (encoder_lossy sndfile fftw3)

add_executable (decoder_lossy decoder_lossy.cpp)
target_link_libraries (decoder_lossy sndfile fftw3)