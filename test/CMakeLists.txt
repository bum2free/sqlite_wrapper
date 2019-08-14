set(project_name "test_sqlite_wrapper")
project(${project_name})

set(SOURCE_DIRS ${${project_name}_SOURCE_DIR}
)
set(SOURCE_FILES "")
foreach (dir ${SOURCE_DIRS})
    file(GLOB_RECURSE srcs ${dir}/*.cpp ${dir}/*.c)
    list(APPEND SOURCE_FILES ${srcs})
endforeach ()
add_executable(${project_name} ${SOURCE_FILES})

set(LINK_LIBS sqlite_wrapper gtest gmock pthread)
target_link_libraries(${project_name} ${LINK_LIBS})

unset(project_name)
unset(SOURCE_DIRS)
unset(SOURCE_FILES)
unset(INCLUDE_DIRS)
unset(LINK_LIBS)
