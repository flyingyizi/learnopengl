cmake_minimum_required(VERSION 3.0.0)

# 
# set(VCPKG_TARGET_TRIPLET  "x86-windows" )
# set(VCPKG_TARGET_TRIPLET  "x64-windows-static" )
set(VCPKG_TARGET_TRIPLET  "x64-linux" )

project(computer_geo VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)  

include(CTest)

add_compile_definitions(UTILDEBUG)
add_compile_definitions(TOPPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR})
set(CMAKE_BUILD_TYPE Debug)  #vs Release

add_subdirectory(src   bin)
# add_subdirectory(tests bin_tests)


#打包
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

