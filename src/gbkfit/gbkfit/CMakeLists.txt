
# ==============================================================================
#
#   CMake configuration file
#
# ==============================================================================

project(gbkfit-project)

# ==============================================================================

find_package(cfitsio)
find_package(Boost REQUIRED)

# ==============================================================================

set(HEADER_FILES
    include/gbkfit/core.hpp
    include/gbkfit/fits.hpp
    include/gbkfit/fitter.hpp
    include/gbkfit/instrument.hpp
    include/gbkfit/math.hpp
    include/gbkfit/model.hpp
    include/gbkfit/ndarray.hpp
    include/gbkfit/ndarray_host.hpp
    include/gbkfit/nddataset.hpp
    include/gbkfit/ndshape.hpp
    include/gbkfit/parameters_fit_info.hpp
    include/gbkfit/prerequisites.hpp
    include/gbkfit/spread_functions.hpp
    include/gbkfit/utility.hpp

)

set(SOURCE_FILES
    src/core.cpp
    src/fits.cpp
    src/fitter.cpp
    src/instrument.cpp
    src/model.cpp
    src/ndarray.cpp
    src/ndarray_host.cpp
    src/nddataset.cpp
    src/ndshape.cpp
    src/parameters_fit_info.cpp
    src/spread_functions.cpp
    src/utility.cpp
)

# ==============================================================================

set(GBKFIT_LIBRARY_STATIC $<TARGET_FILE:target_gbkfit_static> CACHE INTERNAL "" FORCE)
set(GBKFIT_LIBRARY_SHARED $<TARGET_FILE:target_gbkfit_shared> CACHE INTERNAL "" FORCE)
set(GBKFIT_INCLUDE_DIR $<TARGET_PROPERTY:target_gbkfit_object,INTERFACE_INCLUDE_DIRECTORIES> CACHE INTERNAL "" FORCE)
set(GBKFIT_INCLUDE_DIRS ${GBKFIT_INCLUDE_DIR} CACHE INTERNAL "" FORCE)

# ==============================================================================

add_library(target_gbkfit_object OBJECT ${HEADER_FILES} ${SOURCE_FILES})
add_library(target_gbkfit_static STATIC $<TARGET_OBJECTS:target_gbkfit_object>)
add_library(target_gbkfit_shared SHARED $<TARGET_OBJECTS:target_gbkfit_object>)

target_include_directories(target_gbkfit_object PUBLIC "include")
target_include_directories(target_gbkfit_object PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(target_gbkfit_object PRIVATE ${CFITSIO_INCLUDE_DIRS})

target_link_libraries(target_gbkfit_static PRIVATE ${CFITSIO_LIBRARIES})
target_link_libraries(target_gbkfit_shared PRIVATE ${CFITSIO_LIBRARIES})

target_compile_definitions(target_gbkfit_object PRIVATE "GBKFIT_SELF_BUILD")

target_compile_options(target_gbkfit_object PRIVATE "-Wall")
target_compile_options(target_gbkfit_object PRIVATE "-Wextra")
target_compile_options(target_gbkfit_object PRIVATE "-Wpedantic")
target_compile_options(target_gbkfit_object PRIVATE "-Wsign-conversion")

set_target_properties(target_gbkfit_object PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(target_gbkfit_object PROPERTIES CXX_STANDARD 14)
set_target_properties(target_gbkfit_object PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(target_gbkfit_object PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_target_properties(target_gbkfit_static PROPERTIES OUTPUT_NAME "gbkfit_static")
set_target_properties(target_gbkfit_static PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties(target_gbkfit_shared PROPERTIES OUTPUT_NAME "gbkfit_shared")
set_target_properties(target_gbkfit_shared PROPERTIES DEBUG_POSTFIX "_d")

# ==============================================================================

install(TARGETS
        target_gbkfit_static
        target_gbkfit_shared
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)

install(DIRECTORY include/ DESTINATION include)

# ==============================================================================
