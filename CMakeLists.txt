cmake_minimum_required(VERSION 3.25)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
project(join_server VERSION ${PROJECT_VESRION})

find_package(Threads)

# see https://www.sqlite.org/howtocompile.html
include(FetchContent)
FetchContent_Declare(sqlite3 URL "https://www.sqlite.org/2023/sqlite-autoconf-3420000.tar.gz")
FetchContent_MakeAvailable(sqlite3)
if (NOT sqlite3_POPULATED)
  FetchContent_Populate(sqlite3)
endif()

set(BOOST_INCLUDE_LIBRARIES algorithm asio)
set(BOOST_ENABLE_CMAKE ON)
#Windows help debug
#set(Boost_DEBUG ON)
FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.81.0
)
FetchContent_MakeAvailable(Boost)

add_executable(join_server join_server.cpp "${sqlite3_SOURCE_DIR}/sqlite3.c")

target_include_directories(join_server PUBLIC "${sqlite3_SOURCE_DIR}")

set_target_properties(join_server PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON)

if(WIN32)
    if (MSVC)
        target_link_libraries(
            join_server 
            Boost::asio
            Boost::algorithm 
            ${CMAKE_THREAD_LIBS_INIT}
            ws2_32
            wsock32
        )
    else ()
        target_link_libraries(
            join_server 
            Boost::asio
            Boost::algorithm 
            ${CMAKE_THREAD_LIBS_INIT} dl
            ws2_32
            wsock32
        )
    endif()
else()
  target_link_libraries(
      join_server 
      Boost::asio
      Boost::algorithm 
      ${CMAKE_THREAD_LIBS_INIT} dl
  )
endif()

if (MSVC)
    target_compile_options(join_server PRIVATE /W4)
else ()
    target_link_options(join_server PRIVATE -static -static-libgcc -static-libstdc++)

    target_compile_options(
        join_server 
        PRIVATE
        -Wall 
        -Wextra 
        -pedantic
    )
endif()

install (
    TARGETS 
    join_server 
    RUNTIME 
    DESTINATION 
    bin
)

# CPack area
if (WIN32)
    set(CPACK_GENERATOR "ZIP")
else ()
    set(CPACK_GENERATOR "DEB")
endif()
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT https://github.com/4l3x777/homework11)
include(CPack)

