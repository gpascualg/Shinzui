if (SHINZUI_BUILD_TESTS)
    project(ShinzuiTests)

    RequireExternal(
        TARGET ShinzuiTests
        MODULE philsquared/Catch:master
        EXCLUDE ON
        INC_PATH "include"
    )

    AddToSources(
        TARGET ShinzuiTests
        SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
        GLOB_SEARCH ".hpp;.cpp"
    )

    AddDependency(
        TARGET ShinzuiTests
        DEPENDENCY Shinzui
    )

    BuildNow(
        TARGET ShinzuiTests
        DEFINES SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}
        EXECUTABLE ON
        OUTPUT_NAME ShinzuiTests
    )
endif()
