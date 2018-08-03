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

        # Graph
        graph unordered move iterator tuple property_map concept_check
        multi_index foreach serialization optional range typeof conversion
        tti function_types numeric_conversion container_hash

        # Bimap
        bimap
    )

    foreach (dep ${BOOST_DEPENDENCIES})
        RequireExternal(
            TARGET Shinzui
            EXCLUDE
            SKIP_CONFIGURE
            SKIP_BUILD
            MODULE "boostorg/${dep}:develop"
            INSTALL_INCLUDE
        )
    endforeach()
endif()

# Require GLM
RequireExternal(
    TARGET Shinzui
    EXCLUDE
    SKIP_BUILD
    MODULE "g-truc/glm:0.9.8"
    INSTALL_NAME glm
)

# Require Threadpool
RequireExternal(
    TARGET Shinzui
    ENSURE_ORDER
    MODULE "gpascualg/threadpool11:2.0"
    INSTALL_NAME threadpool11
)

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
            BASE_DIR_LEN=${BASE_DIR_LEN}
            _WIN32_WINNT=0x0501
    )

    AddToSources(
        TARGET Shinzui
        GLOB_SEARCH ".hpp;.cpp"
        SOURCES 
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/debug
            ${CMAKE_CURRENT_SOURCE_DIR}/defs
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
