cmake_minimum_required(VERSION 3.16)
project(PCPerformanceMonitor VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    # Enable high performance optimization for Release builds
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG /fp:fast")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1")
    
    # Enable modern C++ features
    add_compile_options(/std:c++17)
    
    # Suppress specific warnings
    add_compile_options(/wd4996) # Disable deprecated function warnings
    
    # Enable parallel compilation
    add_compile_options(/MP)
    
    # Check for VS version
    if(MSVC_VERSION GREATER_EQUAL 1920 AND MSVC_VERSION LESS 1930)
        message(STATUS "Configuring for Visual Studio 2019")
    elseif(MSVC_VERSION GREATER_EQUAL 1930)
        message(STATUS "Configuring for Visual Studio 2022")
    endif()
else()
    # GCC/Clang options
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -ffast-math")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Windows-specific setup
if(WIN32)
    # These are Windows system libraries - no need to find them
    set(WINDOWS_LIBS
        pdh
        wbemuuid
        psapi
        ntdll
        ole32
        oleaut32
    )
    
    # NVIDIA Management Library (optional)
    find_path(NVML_INCLUDE_DIR nvml.h 
        PATHS 
        "C:/Program Files/NVIDIA Corporation/NVSMI"
        "C:/Program Files (x86)/NVIDIA Corporation/NVSMI"
        "$ENV{CUDA_PATH}/include"
        "$ENV{ProgramFiles}/NVIDIA GPU Computing Toolkit/CUDA/*/include"
        PATH_SUFFIXES include
    )
    
    find_library(NVML_LIBRARY nvml
        PATHS 
        "C:/Program Files/NVIDIA Corporation/NVSMI"
        "C:/Program Files (x86)/NVIDIA Corporation/NVSMI"
        "$ENV{CUDA_PATH}/lib/x64"
        "$ENV{ProgramFiles}/NVIDIA GPU Computing Toolkit/CUDA/*/lib/x64"
        PATH_SUFFIXES lib lib/x64
    )
    
    if(NVML_INCLUDE_DIR AND NVML_LIBRARY)
        set(NVML_FOUND TRUE)
        message(STATUS "Found NVML: ${NVML_LIBRARY}")
    else()
        set(NVML_FOUND FALSE)
        message(STATUS "NVML not found. GPU monitoring will be limited to estimates.")
    endif()
else()
    message(FATAL_ERROR "This project currently supports Windows only")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NVML_FOUND)
    include_directories(${NVML_INCLUDE_DIR})
endif()

# Source files
set(SOURCES
    src/performance_monitor.cpp
    src/thermal_monitor.cpp
    src/power_monitor.cpp
    src/data_logger.cpp
    src/web_interface.cpp
)

set(HEADERS
    include/performance_monitor.h
    include/metrics_types.h
    include/data_logger.h
    include/thermal_monitor.h
    include/power_monitor.h
    include/web_interface.h
)

# Create main executable
add_executable(pc_monitor
    src/main.cpp
    ${SOURCES}
    ${HEADERS}
)

# Link libraries
target_link_libraries(pc_monitor
    Threads::Threads
    ${WINDOWS_LIBS}
)

if(NVML_FOUND)
    target_link_libraries(pc_monitor ${NVML_LIBRARY})
    target_compile_definitions(pc_monitor PRIVATE NVML_AVAILABLE)
endif()

# Set output directories
set_target_properties(pc_monitor
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy web assets to output directory
add_custom_command(TARGET pc_monitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/web $<TARGET_FILE_DIR:pc_monitor>/web
    COMMENT "Copying web assets to output directory"
)

# Also copy to Release directory for direct execution
add_custom_command(TARGET pc_monitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/web ${CMAKE_BINARY_DIR}/Release/web
    COMMENT "Copying web assets to Release directory"
)

# Visual Studio specific settings
if(MSVC)
    # Set startup project for VS IDE
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT pc_monitor)
    
    # Set working directory for debugging
    set_target_properties(pc_monitor PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    # Group source files in VS Solution Explorer
    source_group("Header Files" FILES ${HEADERS})
    source_group("Source Files" FILES ${SOURCES} src/main.cpp)
    
    # Enable link-time optimization for Release
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(pc_monitor PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION TRUE
        )
    endif()
endif()

# Installation
install(TARGETS pc_monitor
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS} DESTINATION include/pc_monitor)

# Print configuration summary
message(STATUS "")
message(STATUS "=== PC Monitor Configuration Summary ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if(WIN32)
    message(STATUS "Windows SDK: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
endif()
message(STATUS "NVML Support: ${NVML_FOUND}")
if(NVML_FOUND)
    message(STATUS "NVML Include: ${NVML_INCLUDE_DIR}")
    message(STATUS "NVML Library: ${NVML_LIBRARY}")
endif()
message(STATUS "Source Files: ${SOURCES}")
message(STATUS "==========================================")
message(STATUS "")
