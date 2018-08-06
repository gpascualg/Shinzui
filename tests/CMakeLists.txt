ResolveExternal(TARGET Shinzui SILENT)
if (Shinzui_IS_RESOLVED AND SHINZUI_BUILD_TESTS)
    project(ShinzuiTests)
    CreateTarget(TARGET ShinzuiTests)

    set(CATCH_CONFIGURE
        "-DCATCH_BUILD_TESTING=OFF"
        "-DCATCH_INSTALL_DOCS=OFF"
    )

    RequireExternal(
        TARGET ShinzuiTests
        MODULE catchorg/Catch2:master
        CONFIGURE_ARGUMENTS ${CATCH_CONFIGURE}
        EXCLUDE
        PACKAGE_NAME Catch2
        PACKAGE_TARGET Catch2::Catch2
    )

    ResolveExternal(TARGET ShinzuiTests)
    if (ShinzuiTests_IS_RESOLVED)
        AddToSources(
            TARGET ShinzuiTests
            GLOB_SEARCH ".hpp;.cpp"
            SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/map
                ${CMAKE_CURRENT_SOURCE_DIR}/mocks
                ${CMAKE_CURRENT_SOURCE_DIR}/offset
            NO_DEDUCE_FOLDER
        )

        AddToIncludes(
            TARGET ShinzuiTests
            INCLUDES 
                ${PROJECT_SOURCE_DIR}/../src
                ${PROJECT_SOURCE_DIR}/../tests
        )

        AddDependency(
            TARGET ShinzuiTests
            DEPENDENCY Shinzui
        )

        string(LENGTH ${CMAKE_SOURCE_DIR} BASE_DIR_LEN)
        AddToDefinitions(
            TARGET ShinzuiTests
            DEFINITIONS
                BUILD_TESTS=${SHINZUI_BUILD_TESTS}
                BOOST_ERROR_CODE_HEADER_ONLY
                BOOST_SYSTEM_NO_DEPRECATED
                BOOST_ALL_NO_LIB
                _WIN32_WINNT=0x0501
                PRIVATE
                    BASE_DIR_LEN=${BASE_DIR_LEN}
                    SHINZUI_TESTS
        )

        BuildNow(
            TARGET ShinzuiTests
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
endif()
