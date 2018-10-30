cmake_minimum_required(VERSION 3.8)
project(ccmix)

add_library(ccmix INTERFACE)
target_include_directories(ccmix INTERFACE "${PROJECT_SOURCE_DIR}")
target_compile_features(ccmix INTERFACE cxx_std_14)
install(DIRECTORY ccmix DESTINATION include)

add_library(test_ccmix test_machine.cpp test_word.cpp)
target_link_libraries(test_ccmix PRIVATE ccmix)
