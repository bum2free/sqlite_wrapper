cmake_minimum_required(VERSION 3.10)
set(project_name "sqlite_wrapper")
project(${project_name})

set(SOURCE_DIRS ${${project_name}_SOURCE_DIR}/src)
set(SOURCE_FILES "")
foreach (dir ${SOURCE_DIRS})
    file(GLOB_RECURSE srcs ${dir}/*.cpp ${dir}/*.c)
    list(APPEND SOURCE_FILES ${srcs})
endforeach ()
add_library(${project_name} SHARED ${SOURCE_FILES})

set(INCLUDE_DIRS ${${project_name}_SOURCE_DIR}/include)
target_include_directories(${project_name} PUBLIC ${INCLUDE_DIRS})

set(LINK_LIBS sqlite3)
target_link_libraries(${project_name} PUBLIC ${LINK_LIBS})

unset(project_name)
unset(SOURCE_DIRS)
unset(SOURCE_FILES)
unset(INCLUDE_DIRS)
unset(LINK_LIBS)

add_subdirectory(test)
