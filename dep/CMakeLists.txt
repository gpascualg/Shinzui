# Git is required for patches
find_package(Git REQUIRED)

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
        utility predef winapi tti fusion mp11

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
        # Most package don't yet have a CMakeLists
        RequireExternal(
            TARGET Shinzui
            EXCLUDE
            SKIP_CONFIGURE
            SKIP_BUILD
            MODULE "boostorg/${dep}:boost-1.69.0"
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
    PACKAGE_NAME glm
)

# Require Threadpool
RequireExternal(
    TARGET Shinzui
    ENSURE_ORDER
    MODULE "gpascualg/threadpool11:2.0"
    PACKAGE_NAME threadpool11
)

#######################
# MONGOC 

    set(MONGOC_CONFIGURE
        "-DENABLE_SASL=OFF"     # TODO: Revisit this, do we want it?
        "-DENABLE_SSL=OFF"      # TODO: Revisit this, do we want it?
        "-DENABLE_TESTS=OFF"
        "-DENABLE_EXAMPLES=OFF"
    )

    RequireExternal(
        TARGET Shinzui
        MODULE "mongodb/mongo-c-driver:r1.12"
        CONFIGURE_ARGUMENTS ${MONGOC_CONFIGURE}
        PACKAGE_NAME libmongoc-static-1.0
        PACKAGE_TARGET MONGOC_STATIC
    )

#######################
# MONGOCXX

    set(MONGOCXX_PATCHES
        "${GIT_EXECUTABLE} stash"
        "${GIT_EXECUTABLE} clean -fx"
        "${GIT_EXECUTABLE} apply --reject --whitespace=fix ${CMAKE_CURRENT_SOURCE_DIR}/../patches/01-mongocxx-tests.patch"
    )

    set(MONGOCXX_CONFIGURE
        "-DENABLE_TESTS=OFF"     # TODO: No longer can be disabled
        "-DENABLE_EXAMPLES=OFF"  # TODO: No longer can be disabled
        "-DBUILD_SHARED_LIBS=OFF"
    )

    RequireExternal(
        TARGET Shinzui
        ENSURE_ORDER
        MODULE "mongodb/mongo-cxx-driver:r3.3.1"
        CONFIGURE_STEPS ${MONGOCXX_PATCHES}
        CONFIGURE_ARGUMENTS ${MONGOCXX_CONFIGURE} 
        PACKAGE_NAME libmongocxx-static
        PACKAGE_TARGET LIBMONGOCXX_STATIC
    )

# Other deps
AddToIncludes(
    TARGET Shinzui
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
)
