ResolveExternal(TARGET Shinzui)
if (Shinzui_IS_RESOLVED AND SHINZUI_BUILD_TESTS)
    project(ShinzuiTests)

    RequireExternal(
        TARGET ShinzuiTests
        MODULE philsquared/Catch:master
        EXCLUDE
        SKIP_LINK
        INC_PATH "include"
    )

    set(SOURCE_FOLDERS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/map
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/offset
    )

    foreach(folder ${SOURCE_FOLDERS})
        AddToSources(
            TARGET ShinzuiTests
            SRC_PATH ${folder}
            GLOB_SEARCH ".hpp;.cpp"
        )
    endforeach()

    AddToIncludes(
        TARGET ShinzuiTests
        INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    )

    AddDependency(
        TARGET ShinzuiTests
        DEPENDENCY Shinzui
    )

    string(LENGTH ${CMAKE_SOURCE_DIR} BASE_DIR_LEN)
    set(DEFINES
        BUILD_TESTS=${SHINZUI_BUILD_TESTS}
        BOOST_ERROR_CODE_HEADER_ONLY
        BOOST_SYSTEM_NO_DEPRECATED
        BOOST_ALL_NO_LIB
        BASE_DIR_LEN=${BASE_DIR_LEN}
        SHINZUI_TESTS
        _WIN32_WINNT=0x0501
    )

    BuildNow(
        TARGET ShinzuiTests
        DEFINES ${DEFINES}
        EXECUTABLE
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
