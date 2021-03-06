
# ==============================================================================
#
#   CMake configuration file
#
# ==============================================================================

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(gbkfit-workspace)

# ==============================================================================
#
#   Setup paths and directories
#
# ==============================================================================

# Set module search directories
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

# Set output directories
set(GBKFIT_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(GBKFIT_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(GBKFIT_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(GBKFIT_INCLUDE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/include")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${GBKFIT_RUNTIME_OUTPUT_DIRECTORY}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${GBKFIT_ARCHIVE_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${GBKFIT_LIBRARY_OUTPUT_DIRECTORY}")

# ==============================================================================
#
#   Create user options
#
# ==============================================================================

option(GBKFIT_BUILD_GBKFIT_APP_CLI "build gbkfit_app_cli" TRUE)

option(GBKFIT_BUILD_GBKFIT_CUDA "build gbkfit_cuda" FALSE)
option(GBKFIT_BUILD_GBKFIT_FFTW3 "build gbkfit_fftw3" TRUE)

option(GBKFIT_BUILD_GBKFIT_FITTERS_MPFIT "build gbkfit_fitters_mpfit" TRUE)
option(GBKFIT_BUILD_GBKFIT_FITTERS_MULTINEST "build gbkfit_fitters_multinest" FALSE)

option(GBKFIT_BUILD_GBKFIT_MODELS_MODEL01 "build gbkfit_models_model01" TRUE)
option(GBKFIT_BUILD_GBKFIT_MODELS_MODEL01_CUDA "build gbkfit_models_model01_cuda" TRUE)
option(GBKFIT_BUILD_GBKFIT_MODELS_MODEL01_OMP "build gbkfit_models_model01_omp" TRUE)

# ==============================================================================
#
#   Add subdirectories
#
# ==============================================================================

add_subdirectory(src)

# ==============================================================================
#
#   Setup uninstall target
#
# ==============================================================================

configure_file("cmake/templates/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	       @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
