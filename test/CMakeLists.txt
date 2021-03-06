add_executable(module-tests
        oatpp-mongo/bson/BooleanTest.cpp
        oatpp-mongo/bson/BooleanTest.hpp
        oatpp-mongo/bson/FloatTest.cpp
        oatpp-mongo/bson/FloatTest.hpp
        oatpp-mongo/bson/Int8Test.cpp
        oatpp-mongo/bson/Int8Test.hpp
        oatpp-mongo/bson/Int16Test.cpp
        oatpp-mongo/bson/Int16Test.hpp
        oatpp-mongo/bson/Int32Test.cpp
        oatpp-mongo/bson/Int32Test.hpp
        oatpp-mongo/bson/Int64Test.cpp
        oatpp-mongo/bson/Int64Test.hpp
        oatpp-mongo/bson/ListTest.cpp
        oatpp-mongo/bson/ListTest.hpp
        oatpp-mongo/bson/MapTest.cpp
        oatpp-mongo/bson/MapTest.hpp
        oatpp-mongo/bson/ObjectTest.cpp
        oatpp-mongo/bson/ObjectTest.hpp
        oatpp-mongo/bson/StringTest.cpp
        oatpp-mongo/bson/StringTest.hpp
        oatpp-mongo/bson/InlineDocumentTest.cpp
        oatpp-mongo/bson/InlineDocumentTest.hpp
        oatpp-mongo/TestUtils.cpp
        oatpp-mongo/TestUtils.hpp
        oatpp-mongo/tests.cpp
)

set_target_properties(module-tests PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(module-tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(module-tests ${LIB_OATPP_EXTERNAL})
endif()

add_dependencies(module-tests ${OATPP_THIS_MODULE_NAME})

target_link_oatpp(module-tests)

target_link_libraries(module-tests
        PRIVATE ${OATPP_THIS_MODULE_NAME}
)

###################################################################################################
## get dependencies

message("\n############################################################################")
message("## ${OATPP_THIS_MODULE_NAME} module-tests. Resolving dependencies...\n")

message("\n"
        "############################################################################\n"
        "# NOTE:                                                                    #\n"
        "#                                                                          #\n"
        "#    oatpp-mongo links libmongoxcc for module tests ONLY!!!                #\n"
        "#    TO BUILD oatpp-mongo with NO libmongoxcc use cmake option:            #\n"
        "#                                                                          #\n"
        "#        -------------->  -DOATPP_BUILD_TESTS=OFF  <--------------         #\n"
        "#                                                                          #\n"
        "############################################################################\n"
        "\n")

##############################
## Find libmongocxx dependency

find_package(mongocxx REQUIRED)

message("LIBBSONCXX_INCLUDE_DIRS=${LIBBSONCXX_INCLUDE_DIRS}")
message("LIBBSONCXX_LIBRARIES=${LIBBSONCXX_LIBRARIES}")

message("LIBMONGOCXX_INCLUDE_DIRS=${LIBMONGOCXX_INCLUDE_DIRS}")
message("LIBMONGOCXX_LIBRARIES=${LIBMONGOCXX_LIBRARIES}")

message("\n############################################################################\n")

###################################################################################################

if (TARGET mongo::mongocxx_shared)
    target_link_libraries(module-tests
            PUBLIC mongo::mongocxx_shared
    )
elseif(TARGET mongo::mongocxx_static)
    target_link_libraries(module-tests
            PUBLIC mongo::mongocxx_static
    )
endif()

add_test(module-tests module-tests)