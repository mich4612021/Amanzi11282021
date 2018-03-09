#  -*- mode: cmake -*-

#
# Build TPL:  HYPRE 
#    

# --- Define all the directories and common external project flags
define_external_project_args(HYPRE 
                             TARGET hypre
                             DEPENDS SuperLU)

# add version version to the autogenerated tpl_versions.h file
amanzi_tpl_version_write(FILENAME ${TPL_VERSIONS_INCLUDE_FILE}
  PREFIX HYPRE
  VERSION ${HYPRE_VERSION_MAJOR} ${HYPRE_VERSION_MINOR} ${HYPRE_VERSION_PATCH})


# --- Define configure parameters

# Disable OpenMP with HYPRE for now
# Is OpenMP available
# if (ENABLE_OpenMP)
#   find_package(OpenMP)

set(hypre_openmp_opt)
# if (OPENMP_FOUND)
#   set(hypre_openmp_opt "-DHYPRE_USING_OPENMP:BOOL=TRUE")
# endif()
#else()
set(hypre_openmp_opt "-DHYPRE_USING_OPENMP:BOOL=FALSE")
#endif()

# Locate LAPACK and BLAS
set(hypre_blas_opt)
find_package(BLAS)
if (BLAS_FOUND)
  set(hypre_blas_opt "-DHYPRE_USING_HYPRE_BLAS:BOOL=TRUE")
endif()

set(hypre_lapack_opt)
find_package(LAPACK)
if (LAPACK_FOUND)
  set(hypre_lapack_opt "-DHYPRE_USING_HYPRE_LAPACK:BOOL=TRUE")
endif()

# set(hypre_fortran_opt -"--disable-fortran)

# Locate SuperLU
set(hyper_superlu_opt)
if (ENABLE_PETSC)
  set(hypre_superlu_opt "-DHYPRE_USING_SUPERLU:BOOL=FALSE"
                        "-DHYPRE_USING_FEI:BOOL=FALSE")
endif()

# shared/static libraries (shared FEI is broken in HYPRE)
set(hypre_shared_opt)
if (BUILD_SHARED_LIBS)
  set(hypre_shared_opt "-DHYPRE_SHARED:BOOL=TRUE")
endif()

set(hypre_install_opt "-DHYPRE_INSTALL_PREFIX:PATH=${TPL_INSTALL_PREFIX}")


# --- Set the name of the patch
set(HYPRE_patch_file hypre-cmake3_7.patch)
# --- Configure the bash patch script
set(HYPRE_sh_patch ${HYPRE_prefix_dir}/hypre-patch-step.sh)
configure_file(${SuperBuild_TEMPLATE_FILES_DIR}/hypre-patch-step.sh.in
               ${HYPRE_sh_patch}
               @ONLY)
# --- Configure the CMake patch step
set(HYPRE_cmake_patch ${HYPRE_prefix_dir}/hypre-patch-step.cmake)
configure_file(${SuperBuild_TEMPLATE_FILES_DIR}/hypre-patch-step.cmake.in
               ${HYPRE_cmake_patch}
               @ONLY)
# --- Set the patch command
set(HYPRE_PATCH_COMMAND ${CMAKE_COMMAND} -P ${HYPRE_cmake_patch})     


# --- Add external project build and tie to the ZLIB build target
ExternalProject_Add(${HYPRE_BUILD_TARGET}
                    DEPENDS   ${HYPRE_PACKAGE_DEPENDS}         # Package dependency target
                    TMP_DIR   ${HYPRE_tmp_dir}                 # Temporary files directory
                    STAMP_DIR ${HYPRE_stamp_dir}               # Timestamp and log directory
                    # -- Download and URL definitions
                    DOWNLOAD_DIR   ${TPL_DOWNLOAD_DIR}          
                    URL            ${HYPRE_URL}                # URL may be a web site OR a local file
                    URL_MD5        ${HYPRE_MD5_SUM}            # md5sum of the archive file
                    DOWNLOAD_NAME  ${HYPRE_SAVEAS_FILE}        # file name to store (if not end of URL)
                    # -- Patch 
                    PATCH_COMMAND  ${HYPRE_PATCH_COMMAND}
                    # -- Configure
                    SOURCE_DIR    ${HYPRE_source_dir}
                    # SOURCE_SUBDIR src   # cmake 3.7+ feature 
		    CMAKE_ARGS    ${AMANZI_CMAKE_CACHE_ARGS}   # Global definitions from root CMakeList
                                  ${hypre_openmp_opt} 
                                  ${hypre_lapack_opt} ${hypre_blas_opt}
                                  ${hypre_superlu_opt} ${hypre_shared_opt}
                                  ${hypre_install_opt}
                                  -DCMAKE_C_FLAGS:STRING=${Amanzi_COMMON_CFLAGS}  # Ensure uniform build
                                  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
                                  -DCMAKE_CXX_FLAGS:STRING=${Amanzi_COMMON_CXXFLAGS}
                                  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
                                  -DMPI_CXX_COMPILER:FILEPATH=${MPI_CXX_COMPILER}
                                  -DMPI_C_COMPILER:FILEPATH=${MPI_C_COMPILER}
                                  -DMPI_Fortran_COMPILER:FILEPATH=${MPI_Fortran_COMPILER}
                    # -- Build
                    BINARY_DIR       ${HYPRE_build_dir}        # Build directory 
                    BUILD_COMMAND    ${MAKE} 
                    # -- Install
                    INSTALL_DIR      ${TPL_INSTALL_PREFIX}     # Install directory
      		    INSTALL_COMMAND  $(MAKE) install
                    # -- Output control
                    ${HYPRE_logging_args})


# --- Useful variables that depend on HYPRE
include(BuildLibraryName)
build_library_name(HYPRE HYPRE_LIBRARY APPEND_PATH ${TPL_INSTALL_PREFIX}/lib)
set(HYPRE_DIR "${TPL_INSTALL_PREFIX}")
set(HYPRE_INCLUDE_DIRS "${TPL_INSTALL_PREFIX}/include")
set(HYPRE_LIBRARIES    "${HYPRE_LIBRARY}")
