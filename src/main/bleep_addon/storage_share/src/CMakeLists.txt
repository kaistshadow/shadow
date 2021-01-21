add_subdirectory(core)

if(${TEST} STREQUAL ON)
    add_subdirectory(test)
endif(${TEST} STREQUAL ON)