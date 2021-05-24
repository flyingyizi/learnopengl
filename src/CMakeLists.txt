
set(CMAKE_INSTALL_PREFIX  "C:\\home\\temp")




file(GLOB_RECURSE ORIGSRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(geo  ${ORIGSRC_FILES})
target_include_directories(geo PRIVATE ${PROJECT_SOURCE_DIR}/inc ${PROJECT_SOURCE_DIR}/src)

# file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.hpp  *.cpp)
# set(SOURCES ${SOURCES})

#set(jsoncpp_DIR "C:\\prog\\vcpkg\\installed\\x64-windows\\share\\jsoncpp")
# find_package(jsoncpp CONFIG REQUIRED)
# target_link_libraries(geo PRIVATE jsoncpp_lib jsoncpp_object)
# target_link_libraries(geo PRIVATE jsoncpp_object jsoncpp_static)

# target_link_libraries(geo PRIVATE jsoncpp::jsoncpp_object jsoncpp::jsoncpp_static)
# target_link_libraries(geo PRIVATE jsoncpp_object jsoncpp jsoncpp_static ${jsoncpp_libs})

# find_package(Eigen3 CONFIG REQUIRED)
# find_package(PkgConfig)
# pkg_search_module(MATIOMODULE REQUIRED matio)

# target_include_directories(geo PRIVATE 
#          ${MATIOMODULE_INCLUDE_DIRS}
# )
        #  ${jsoncpp_INCLUDE_DIRS}

# target_link_libraries(geo PRIVATE 
#              Eigen3::Eigen  
#              ${MATIOMODULE_LIBRARIES}          
#              )

find_package(glad CONFIG REQUIRED)
target_link_libraries(geo PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(geo PRIVATE glfw)

find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(geo PRIVATE Eigen3::Eigen)
include_directories(${EIGEN3_INCLUDE_DIRS})


find_package(PkgConfig REQUIRED) 
# pkg_search_module(EGL REQUIRED egl)
target_link_libraries(geo PRIVATE 
        # dl
        pthread             #needed by gtest
        ${EGL_LIBRARIES}   )
         
message(STATUS "=== EIGEN3_INCLUDE_DIRS: ${EIGEN3_INCLUDE_DIRS}")


