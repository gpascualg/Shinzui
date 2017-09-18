CreateTarget(TARGET Shinzui)

if (SHINZUI_USE_SYSTEM_BOOST)
    find_package(Boost 1.62 REQUIRED)
    set(BOOST_DEPENDENCIES "")

    AddDependency(
        TARGET Shinzui
        DEPENDENCY Boost
    )
else()
    set(BOOST_DEPENDENCIES
        # Lockfree dependencies
        lockfree core config assert static_assert type_traits mpl preprocessor
        array throw_exception functional align parameter atomic detail
        utility predef winapi

        # Asio dependencies
        asio system date_time smart_ptr bind regex

        # Pool dependencies
        pool integer
    )

    foreach (dep ${BOOST_DEPENDENCIES})
        RequireExternal(
            TARGET Shinzui
            EXCLUDE
            SKIP_BUILD
            SKIP_LINK
            MODULE "boostorg/${dep}:develop"
            INC_PATH "include"
        )
    endforeach()
endif()

# Require GLM
RequireExternal(
    TARGET Shinzui
    EXCLUDE
    SKIP_BUILD
    SKIP_LINK
    MODULE "g-truc/glm:0.9.8"
    INC_PATH "."
)

ResolveExternal(TARGET Shinzui)
if (Shinzui_IS_RESOLVED)
    string(LENGTH ${CMAKE_SOURCE_DIR} BASE_DIR_LEN)
    set(DEFINES
        BUILD_TESTS=${SHINZUI_BUILD_TESTS}
        BOOST_ERROR_CODE_HEADER_ONLY
        BOOST_SYSTEM_NO_DEPRECATED
        BOOST_ALL_NO_LIB
        BASE_DIR_LEN=${BASE_DIR_LEN}
        _WIN32_WINNT=0x0501
    )

    set(SOURCE_FOLDERS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/debug
        ${CMAKE_CURRENT_SOURCE_DIR}/defs
        ${CMAKE_CURRENT_SOURCE_DIR}/io
        ${CMAKE_CURRENT_SOURCE_DIR}/map
        ${CMAKE_CURRENT_SOURCE_DIR}/map/map-cluster
        ${CMAKE_CURRENT_SOURCE_DIR}/movement
        ${CMAKE_CURRENT_SOURCE_DIR}/server
        ${CMAKE_CURRENT_SOURCE_DIR}/threadpool
    )

    foreach(folder ${SOURCE_FOLDERS})
        AddToSources(
            TARGET Shinzui
            SRC_PATH ${folder}
            GLOB_SEARCH ".hpp;.cpp"
        )
    endforeach()

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

    BuildNow(
        TARGET Shinzui
        DEFINES ${DEFINES}
        STATIC_LIB
        OUTPUT_NAME Shinzui
    )

    if (SHINZUI_LINT)
        include(Lint)
        setup_target_for_lint(ShinzuiLint "${Shinzui_SOURCES}")
    endif()
endif()
