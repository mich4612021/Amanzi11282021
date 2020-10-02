# -*- mode: cmake -*-

#
# Amanzi ALQUIMIA Find Module
#
# Usage:
#    Control the search through ALQUIMIA_DIR or setting environment variable
#    ALQUIMIA_ROOT to the ALQUIMIA installation prefix.
#
#    This module does not search default paths! 
#
#    Following variables are set:
#    ALQUIMIA_FOUND            (BOOL)       Flag indicating if ALQUIMIA was found
#    ALQUIMIA_INCLUDE_DIR      (PATH)       Path to the ALQUIMIA include file
#    ALQUIMIA_INCLUDE_DIRS     (LIST)       List of all required include files
#    ALQUIMIA_LIBRARY_DIR      (PATH)       Path to the ALQUIMIA library
#    ALQUIMIA_LIBRARIES        (LIST)       List of all required ALQUIMIA libraries
#
# #############################################################################

include(AddImportedLibrary)

# Amanzi CMake functions see <root>/tools/cmake for source
include(PrintVariable)
include(AddPackageDependency)

if (ALQUIMIA_LIBRARIES AND ALQUIMIA_INCLUDE_DIRS)

  # Do nothing. Variables are set. No need to search again

else(ALQUIMIA_LIBRARIES AND ALQUIMIA_INCLUDE_DIRS)

  message(STATUS "HERE ALQUIMIA_DIR=${ALQUIMIA_DIR}")

  # Cache variables
  if (ALQUIMIA_DIR)
    set(ALQUIMIA_DIR "${ALQUIMIA_DIR}" CACHE PATH "Path to search for Alquimia include and library files")
  endif()

  if (ALQUIMIA_INCLUDE_DIR)
    set(ALQUIMIA_INCLUDE_DIR "${ALQUIMIA_INCLUDE_DIR}" CACHE PATH "Path to search for Alquimia include files")
  endif()

  if (ALQUIMIA_LIBRARY_DIR)
    set(ALQUIMIA_LIBRARY_DIR "${ALQUIMIA_LIBRARY_DIR}" CACHE PATH "Path to search for Alquimia library files")
  endif()

    
  # Search for include files
  # Search order preference:
  #  (1) ALQUIMIA_INCLUDE_DIR - check existence of path AND if the include files exist
  #  (2) ALQUIMIA_DIR/<include>
  #  (3) Default CMake paths See cmake --html-help=out.html file for more information.
  #
  set(alquimia_inc_names "alquimia.h")
  if (ALQUIMIA_INCLUDE_DIR)
    if (EXISTS "${ALQUIMIA_INCLUDE_DIR}")

      find_path(alquimia_test_include_path
                NAMES ${alquimia_inc_names}
                HINTS ${ALQUIMIA_INCLUDE_DIR}
                NO_DEFAULT_PATH)

      if (NOT alquimia_test_include_path)
        message(SEND_ERROR "Can not locate ${alquimia_inc_names} in ${ALQUIMIA_INCLUDE_DIR}")
      endif()
      set(ALQUIMIA_INCLUDE_DIR "${alquimia_test_include_path}")

    else()
      message(SEND_ERROR "ALQUIMIA_INCLUDE_DIR=${ALQUIMIA_INCLUDE_DIR} does not exist")
      set(ALQUIMIA_INCLUDE_DIR "ALQUIMIA_INCLUDE_DIR-NOTFOUND")
    endif()

  else() 

    set(alquimia_inc_suffixes "include/alquimia")
    if (ALQUIMIA_DIR)
      if (EXISTS "${ALQUIMIA_DIR}")

        find_path(ALQUIMIA_INCLUDE_DIR
                  NAMES ${alquimia_inc_names}
                  HINTS ${ALQUIMIA_DIR}
                  PATH_SUFFIXES ${alquimia_inc_suffixes}
                  NO_DEFAULT_PATH)

      else()
        message(SEND_ERROR "ALQUIMIA_DIR=${ALQUIMIA_DIR} does not exist")
        set(ALQUIMIA_INCLUDE_DIR "ALQUIMIA_INCLUDE_DIR-NOTFOUND")
      endif()    

    else()

      find_path(ALQUIMIA_INCLUDE_DIR
                NAMES ${alquimia_inc_names}
                PATH_SUFFIXES ${alquimia_inc_suffixes})

    endif()
  endif()

  if (NOT ALQUIMIA_INCLUDE_DIR)
    message(SEND_ERROR "Can not locate Alquimia include directory")
  endif()

  # Search for libraries 
  # Search order preference:
  #  (1) ALQUIMIA_LIBRARY_DIR - check existence of path AND if the library file exists
  #  (2) ALQUIMIA_DIR/<lib,Lib>
  #  (3) Default CMake paths See cmake --html-help=out.html file for more information.
  #
  set(alquimia_lib_names "alquimia")
  if (ALQUIMIA_LIBRARY_DIR)
    if (EXISTS "${ALQUIMIA_LIBRARY_DIR}")

      find_library(_ALQUIMIA_LIBRARY
                   NAMES ${alquimia_lib_names}
                   HINTS ${ALQUIMIA_LIBRARY_DIR}
                   NO_DEFAULT_PATH)

    else()
      message(SEND_ERROR "ALQUIMIA_LIBRARY_DIR=${ALQUIMIA_LIBRARY_DIR} does not exist")
      set(_ALQUIMIA_LIBRARY "ALQUIMIA_LIBRARY-NOTFOUND")
      set(_ALQUIMIA_Fortran_LIBRARY "ALQUIMIA_Fortran_LIBRARY-NOTFOUND")
    endif()

  else() 

    list(APPEND alquimia_lib_suffixes "lib" "Lib")
    if (ALQUIMIA_DIR)
      if (EXISTS "${ALQUIMIA_DIR}")

        find_library(_ALQUIMIA_LIBRARY
                     NAMES ${alquimia_lib_names}
                     HINTS ${ALQUIMIA_DIR}
                     PATH_SUFFIXES ${alquimia_lib_suffixes}
                     NO_DEFAULT_PATH)
                
      else()
        message(SEND_ERROR "ALQUIMIA_DIR=${ALQUIMIA_DIR} does not exist")
        set(ALQUIMIA_LIBRARY "ALQUIMIA_LIBRARY-NOTFOUND")
        set(ALQUIMIA_Fortran_LIBRARY "ALQUIMIA_Fortran_LIBRARY-NOTFOUND")
      endif()    

    else()

      find_library(_ALQUIMIA_LIBRARY
                   NAMES ${alquimia_lib_names}
                   PATH_SUFFIXES ${alquimia_lib_suffixes})

    endif()
  endif()

  # Create the library target store the name in ALQUIMIA_LIBRARY
  if ( _ALQUIMIA_LIBRARY )
    set(ALQUIMIA_LIBRARY alquimia)
    add_imported_library(${ALQUIMIA_LIBRARY}
                        LOCATION ${ALQUIMIA_LIBRARY})
  else()
    message(SEND_ERROR "Can not locate ALQUIMIA library")
  endif()

  # Define prerequisite packages
  set(ALQUIMIA_INCLUDE_DIRS ${ALQUIMIA_INCLUDE_DIR})
  set(ALQUIMIA_LIBRARIES    ${ALQUIMIA_LIBRARY})

  # Alquimia generates a CMake configuration file that contains the
  # required TPLs. I use an include here instead of find_package
  # to prevent a recursive call.
  if (ALQUIMIA_DIR)
    set(ALQUIMIA_CMAKE_CONFIG_FILE ${ALQUIMIA_DIR}/share/alquimia/alquimia.cmake)
    if (EXISTS ${ALQUIMIA_CMAKE_CONFIG_FILE})
      include(${ALQUIMIA_CMAKE_CONFIG_FILE})

      # Include paths
      if (ALQUIMIA_PACKAGE_INCLUDES)
	      list(APPEND ALQUIMIA_INCLUDE_DIRS ${ALQUIMIA_PACKAGE_INCLUDES})
	      list(REMOVE_DUPLICATES ALQUIMIA_INCLUDE_DIRS)
      endif()

      # TPL libraries, some of the items in this list are not defined!
      if (ALQUIMIA_PACKAGE_LIBS)
	      foreach(lib ${ALQUIMIA_PACKAGE_LIBS})
	        if (lib)
	          list(APPEND ALQUIMIA_LIBRARIES ${lib})
	        endif()
        endforeach()
      endif()  
    endif()
  endif()  
   
endif(ALQUIMIA_LIBRARIES AND ALQUIMIA_INCLUDE_DIRS) 

# Send useful message if everything is found
find_package_handle_standard_args(Alquimia DEFAULT_MSG
                                  ALQUIMIA_INCLUDE_DIR
                                  ALQUIMIA_LIBRARIES)

mark_as_advanced(
  ALQUIMIA_INCLUDE_DIR
  ALQUIMIA_INCLUDE_DIRS
  ALQUIMIA_LIBRARY
  ALQUIMIA_LIBRARIES
  ALQUIMIA_LIBRARY_DIR
)
