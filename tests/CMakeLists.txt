if (SHINZUI_BUILD_TESTS)
    project(ShinzuiTests)

    RequireExternal(
        TARGET ShinzuiTests
        MODULE philsquared/Catch:master
        EXCLUDE ON
        INC_PATH "include"
    )

    set(SOURCE_FOLDERS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/map
        ${CMAKE_CURRENT_SOURCE_DIR}/offset
    )

    foreach(folder ${SOURCE_FOLDERS})
        AddToSources(
            TARGET ShinzuiTests
            SRC_PATH ${folder}
            GLOB_SEARCH ".hpp;.cpp"
        )
    endforeach()

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

    if (UNIX AND SHINZUI_COVERAGE)
        include(Coverage)

        SETUP_TARGET_FOR_COVERAGE(
            ShinzuiCoverage
            ShinzuiTests
            coverage
        )
    endif()
endif()
