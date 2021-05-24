
set(CMAKE_INSTALL_PREFIX  "C:\\home\\temp")


file(GLOB_RECURSE ORIGSRC_FILES ${PROJECT_SOURCE_DIR}/src2/*.cpp ${PROJECT_SOURCE_DIR}/src2/*.c)
add_executable(gtkmmgl  ${ORIGSRC_FILES})
target_include_directories(gtkmmgl PRIVATE ${PROJECT_SOURCE_DIR}/inc ${PROJECT_SOURCE_DIR}/src2)

set(ENV{PKG_CONFIG_PATH} "C:/tools/msys64/mingw64/lib/pkgconfig" )

find_package(PkgConfig REQUIRED) 
pkg_search_module(GTK4 REQUIRED gtk4) 
target_link_directories(gtkmmgl PRIVATE     ${GTK4_LIBRARY_DIRS}  )        
target_link_libraries(gtkmmgl PRIVATE  ${GTK4_LIBRARIES}   )
include_directories(${GTK4_INCLUDE_DIRS})

pkg_search_module(GTKMM4 PRIVATE gtkmm-4.0)
target_link_directories(gtkmmgl PRIVATE     ${GTKMM4_LIBRARY_DIRS}   )        
target_link_libraries(gtkmmgl PRIVATE  ${GTKMM4_LIBRARIES}  )
include_directories(${GTKMM4_INCLUDE_DIRS} )

pkg_search_module(ASSIMP PRIVATE assimp)
target_link_directories(gtkmmgl PRIVATE     ${ASSIMP_LIBRARY_DIRS}   )        
target_link_libraries(gtkmmgl PRIVATE  ${ASSIMP_LIBRARIES}  )
include_directories(${ASSIMP_INCLUDE_DIRS} )

pkg_search_module(EIGEN3 PRIVATE eigen3)
target_link_directories(gtkmmgl PRIVATE     ${EIGEN3_LIBRARY_DIRS}   )        
target_link_libraries(gtkmmgl PRIVATE  ${EIGEN3_LIBRARIES}  )
include_directories(${EIGEN3_INCLUDE_DIRS} )

pkg_search_module(EPOXY PRIVATE epoxy)
target_link_directories(gtkmmgl PRIVATE     ${EPOXY_LIBRARY_DIRS}   )        
target_link_libraries(gtkmmgl PRIVATE  ${EPOXY_LIBRARIES}  )
include_directories(${EPOXY_INCLUDE_DIRS} )

message(STATUS "=== GTKMM4_INCLUDE_DIRS: ${GTKMM4_INCLUDE_DIRS}")

