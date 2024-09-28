
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

set(ZIP_LIBS "libzip" "7zpp" "bit7z")

if(WIN32)
	set(ZIP_READ_LIB_DEFAULT "bit7z")
	set(ZIP_WRITE_LIB_DEFAULT "libzip")
else()
	set(ZIP_READ_LIB_DEFAULT "libzip")
	set(ZIP_WRITE_LIB_DEFAULT "libzip")
endif()

set(ZIP_READ_LIB "${ZIP_READ_LIB_DEFAULT}" CACHE STRING "Library for reading zip files (bit7z, 7zpp, libzip)")
set_property(CACHE ZIP_READ_LIB PROPERTY STRINGS ${ZIP_LIBS})

set(ZIP_WRITE_LIB "${ZIP_WRITE_LIB_DEFAULT}" CACHE STRING "Library for writing zip files (bit7z, 7zpp, libzip)")
set_property(CACHE ZIP_WRITE_LIB PROPERTY STRINGS ${ZIP_LIBS})

if(${ZIP_READ_LIB} STREQUAL "libzip")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_READ_LIB=0 PRIVATE)
elseif(${ZIP_READ_LIB} STREQUAL "7zpp")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_READ_LIB=1 PRIVATE)
elseif(${ZIP_READ_LIB} STREQUAL "bit7z")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_READ_LIB=2 PRIVATE)
else()
    message(FATAL_ERROR "Invalid library selection for ZIP_READ_LIB. Choose from bit7z, 7zpp, libzip.")
endif()

if(${ZIP_WRITE_LIB} STREQUAL "libzip")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_WRITE_LIB=0 PRIVATE)
elseif(${ZIP_WRITE_LIB} STREQUAL "7zpp")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_WRITE_LIB=1 PRIVATE)
elseif(${ZIP_WRITE_LIB} STREQUAL "bit7z")
	pr_add_compile_definitions(${PROJ_NAME} -DZIP_WRITE_LIB=2 PRIVATE)
else()
    message(FATAL_ERROR "Invalid library selection for ZIP_WRITE_LIB. Choose from bit7z, 7zpp, libzip.")
endif()

pr_init_module(${PROJ_NAME})

pr_finalize(${PROJ_NAME})
