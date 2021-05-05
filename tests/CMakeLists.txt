# Remove strings matching given regular expression from a list.
# @param(in,out) aItems Reference of a list variable to filter.
# @param aRegEx Value of regular expression to match.
function (filter_items aItems aRegEx)
    # For each item in our list
    foreach (item ${${aItems}})
        # Check if our items matches our regular expression
        if ("${item}" MATCHES ${aRegEx})
            # Remove current item from our list
            list (REMOVE_ITEM ${aItems} ${item})
        endif ("${item}" MATCHES ${aRegEx})
    endforeach(item)
    # Provide output parameter
    set(${aItems} ${${aItems}} PARENT_SCOPE)
endfunction (filter_items)

IF (MSVC)
	MESSAGE(STATUS "current platform: Windows")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")    
ENDIF (MSVC)



file(GLOB_RECURSE ORIGSRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
filter_items(ORIGSRC_FILES ".*main.cpp.*")
aux_source_directory(./src  TESTS_SRCS)

add_executable(test_geo ${TESTS_SRCS} ${ORIGSRC_FILES})
target_include_directories(test_geo PRIVATE ${PROJECT_SOURCE_DIR}/inc )

# enable_testing() # 可以不需要，因为“include(CTest)”的执行会调用它
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)
# set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE   PRE_TEST)
target_link_libraries(test_geo PRIVATE  GTest::gtest)
# target_link_libraries(main PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
target_include_directories(test_geo PRIVATE ${GTEST_INCLUDE_DIRS}  )

# target_link_libraries(test_geo PRIVATE GTest::gmock GTest::gtest GTest::gmock_main)
gtest_discover_tests(test_geo)

find_path(MATPLOTLIBCPP_INCLUDE_DIR "matplotlibcpp.h")
target_include_directories(test_geo PRIVATE ${MATPLOTLIBCPP_INCLUDE_DIR})

find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(test_geo PRIVATE Eigen3::Eigen        )
target_include_directories(test_geo PRIVATE ${EIGEN3_INCLUDE_DIRS}   )

# find_package(jsoncpp CONFIG REQUIRED)
# # target_link_libraries(test_geo PRIVATE jsoncpp_lib jsoncpp_object)   
# target_link_libraries(test_geo PRIVATE jsoncpp_object jsoncpp_static)
# target_link_directories(test_geo PRIVATE ${JSONCPP_LIBRARY_DIRS} )

#CGAL_Qt5 is needed for the drawing.
find_package(CGAL CONFIG REQUIRED)
target_link_libraries(test_geo PRIVATE CGAL::CGAL)
find_package(CGAL REQUIRED OPTIONAL_COMPONENTS Qt5)
if(CGAL_Qt5_FOUND)
  #required to use basic_viewer
  add_definitions(-DCGAL_USE_BASIC_VIEWER -DQT_NO_KEYWORDS)
  target_link_libraries(test_geo PUBLIC CGAL::CGAL_Qt5)
endif()

find_package(OpenCV CONFIG REQUIRED)
# Note: 12 target(s) were omitted.
target_link_libraries(test_geo PRIVATE opencv_ml opencv_dnn opencv_core opencv_flann)



# the `pkg_check_modules` function is created with this call
find_package(PkgConfig REQUIRED) 
# pkg_search_module(MATIOMODULE REQUIRED matio)
# pkg_search_module(PYTHON REQUIRED python-3.6)

pkg_search_module(PYTHON REQUIRED python-3.8-embed)

pkg_search_module(EGL REQUIRED egl)


target_include_directories(test_geo PRIVATE 
        ${PYTHON_INCLUDE_DIRS} 
        ${MATIOMODULE_INCLUDE_DIRS} )
target_link_libraries(test_geo PRIVATE 
        pthread             #needed by gtest
        ${PYTHON_LIBRARIES}  
        ${EGL_LIBRARIES}  
        ${MATIOMODULE_LIBRARIES} )
target_link_directories(  test_geo PRIVATE 
${PYTHON_LIBRARY_DIRS}
${MATIOMODULE_LIBRARY_DIRS} )



# if(MSVC)
#     set(CompilerFlags
#     CMAKE_CXX_FLAGS
#     CMAKE_CXX_FLAGS_DEBUG
#     CMAKE_CXX_FLAGS_RELEASE
#     CMAKE_C_FLAGS
#     CMAKE_C_FLAGS_DEBUG
#     CMAKE_C_FLAGS_RELEASE
#     CMAKE_CXX_FLAGS_RELWITHDEBINFO
#     )
#     foreach(CompilerFlag ${CompilerFlags})
#     string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
#     endforeach()
# endif(MSVC)




message(STATUS "=== CMAKE_PREFIX_PATH:    ${CMAKE_PREFIX_PATH}")
message(STATUS "=== PYTHON_LIBRARIES:    ${PYTHON_LIBRARIES}")
message(STATUS "=== PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}")
message(STATUS "=== EIGEN3_INCLUDE_DIRS: ${EIGEN3_INCLUDE_DIRS}")
message(STATUS "=== MATPLOTLIBCPP_INCLUDE_DIR: ${MATPLOTLIBCPP_INCLUDE_DIR}")
# message(STATUS " qt  ${Qt5Widgets_INCLUDE_DIRS}")
