ResolveExternal(TARGET Shinzui)
if (Shinzui_IS_RESOLVED)
    string(LENGTH ${CMAKE_SOURCE_DIR} BASE_DIR_LEN)
    AddToDefinitions(
        TARGET Shinzui
        DEFINITIONS
            BUILD_TESTS=${SHINZUI_BUILD_TESTS}
            BOOST_ERROR_CODE_HEADER_ONLY
            BOOST_SYSTEM_NO_DEPRECATED
            BOOST_ALL_NO_LIB
            _WIN32_WINNT=0x0501
            PRIVATE 
                BASE_DIR_LEN=${BASE_DIR_LEN}
    )

    AddToSources(
        TARGET Shinzui
        GLOB_SEARCH ".hpp;.cpp"
        SOURCES 
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/database
            ${CMAKE_CURRENT_SOURCE_DIR}/debug
            ${CMAKE_CURRENT_SOURCE_DIR}/defs
            ${CMAKE_CURRENT_SOURCE_DIR}/executor
            ${CMAKE_CURRENT_SOURCE_DIR}/io
            ${CMAKE_CURRENT_SOURCE_DIR}/map
            ${CMAKE_CURRENT_SOURCE_DIR}/map/map-cluster
            ${CMAKE_CURRENT_SOURCE_DIR}/movement
            ${CMAKE_CURRENT_SOURCE_DIR}/physics
            ${CMAKE_CURRENT_SOURCE_DIR}/server
            ${CMAKE_CURRENT_SOURCE_DIR}/threadpool
    )

    AddToIncludes(
        TARGET Shinzui
        INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    )

    if (UNIX AND SHINZUI_COVERAGE)
        AddDependency(
            TARGET Shinzui
            DEPENDENCY gcov
        )
    endif()

    AddLibrary(
        TARGET Shinzui
        PLATFORM UNIX
        LIBRARY pthread
    )

    BuildNow(
        TARGET Shinzui
        STATIC_LIB
        OUTPUT_NAME Shinzui
    )

    WarningAll(TARGET Shinzui)

    if (SHINZUI_LINT)
        include(Lint)
        setup_target_for_lint(ShinzuiLint "${Shinzui_SOURCES}")
    endif()
endif()
