
include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME util_zip)
pr_add_library(${PROJ_NAME} STATIC)

pr_add_dependency(${PROJ_NAME} sharedutils TARGET PUBLIC)
if(WIN32)
    pr_add_dependency(${PROJ_NAME} 7zpp TARGET)
endif()
pr_add_dependency(${PROJ_NAME} zip TARGET)

pr_add_external_dependency(${PROJ_NAME} bit7z LIBRARY PUBLIC)
pr_add_compile_definitions(${PROJ_NAME} -DBIT7Z_AUTO_FORMAT PRIVATE)

if(WIN32)
	set(ZIP_LIBS "libzip" "7zpp" "bit7z")
else()
	set(ZIP_LIBS "libzip" "bit7z")
endif()

if(WIN32)
	set(ZIP_READ_LIB_DEFAULT "bit7z")
	set(ZIP_WRITE_LIB_DEFAULT "libzip")
else()
	set(ZIP_READ_LIB_DEFAULT "bit7z")
	set(ZIP_WRITE_LIB_DEFAULT "libzip")
endif()

string(JOIN " " ZIP_LIBS_STR ${ZIP_LIBS})
set(ZIP_READ_LIB "${ZIP_READ_LIB_DEFAULT}" CACHE STRING "Library for reading zip files (${ZIP_LIBS_STR})")
set_property(CACHE ZIP_READ_LIB PROPERTY STRINGS ${ZIP_LIBS})

set(ZIP_WRITE_LIB "${ZIP_WRITE_LIB_DEFAULT}" CACHE STRING "Library for writing zip files (${ZIP_LIBS_STR})")
set_property(CACHE ZIP_WRITE_LIB PROPERTY STRINGS ${ZIP_LIBS})

if(${ZIP_READ_LIB} STREQUAL "libzip")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_READ_LIB=0 PRIVATE)
elseif(${ZIP_READ_LIB} STREQUAL "7zpp")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_READ_LIB=1 PRIVATE)
elseif(${ZIP_READ_LIB} STREQUAL "bit7z")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_READ_LIB=2 PRIVATE)
else()
    message(FATAL_ERROR "Invalid library selection for ZIP_READ_LIB. Choose from ${ZIP_LIBS_STR}.")
endif()

if(${ZIP_WRITE_LIB} STREQUAL "libzip")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_WRITE_LIB=0 PRIVATE)
elseif(${ZIP_WRITE_LIB} STREQUAL "7zpp")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_WRITE_LIB=1 PRIVATE)
elseif(${ZIP_WRITE_LIB} STREQUAL "bit7z")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_WRITE_LIB=2 PRIVATE)
else()
    message(FATAL_ERROR "Invalid library selection for ZIP_WRITE_LIB. Choose from ${ZIP_LIBS_STR}.")
endif()

if(WIN32)
    set(SOURCE_EXCLUSION_FILTER)
    set(MODULE_EXCLUSION_FILTER)
else()
    set(SOURCE_EXCLUSION_FILTER "src/implementation/libraries/windows/*.cpp")
    set(MODULE_EXCLUSION_FILTER "src/implementation/libraries/windows/*.cppm")
endif()

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/" EXCLUDE "${SOURCE_EXCLUSION_FILTER}")
pr_add_modules(${PROJ_NAME} "src/" EXCLUDE "${MODULE_EXCLUSION_FILTER}")

pr_finalize(${PROJ_NAME})
