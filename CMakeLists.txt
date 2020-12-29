cmake_minimum_required(VERSION 3.17)
project(lstructural)

set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

include(${CMAKE_SOURCE_DIR}/conan.cmake)
conan_cmake_run(CONANFILE ${CMAKE_SOURCE_DIR}/conanfile.txt
        BUILD_TYPE ${CMAKE_BUILD_TYPE}
        BASIC_SETUP CMAKE_TARGETS
        BUILD missing)

#add_executable(lstructural main.cpp
#        src/logger.h
#        test/test_log.cpp
#)

include_directories(src)

### Testing
enable_testing()

add_executable(lltest
        test/test-log.cpp
        src/logger.cpp src/log_sink.cpp src/log_sink.h src/backtrace.cpp src/clock.cpp src/clock.h)
target_link_libraries(lltest
        CONAN_PKG::gtest
        CONAN_PKG::fmt
        )

add_test(AllTestsInFoo lltest)
