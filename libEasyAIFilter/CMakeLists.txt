cmake_minimum_required(VERSION 2.6)

project(libEasyAIFilter)


add_definitions(-fPIC)

set(LIB_NAME "EasyAIFilter")

include_directories(.)
include_directories(./Include)
include_directories(./Include/FFmpeg4.0.2)

aux_source_directory(. SRC_LIST_EASYAIFILTER)


add_library(${LIB_NAME} SHARED ${SRC_LIST_EASYAIFILTER})
