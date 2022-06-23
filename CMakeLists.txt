CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF (POLICY CMP0048)
    CMAKE_POLICY(SET CMP0048 NEW)
ENDIF ()

PROJECT(NAIVE_RAY_TRACER LANGUAGES CXX VERSION 0.0.1)

IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF ()

ADD_SUBDIRECTORY(deps/vecmath)

SET(NAIVE_RAY_TRACER_SOURCES
    src/main.cpp
    src/utils.cpp)

SET(NAIVE_RAY_TRACER_INCLUDES
    include/geometry/geometry.hpp
    include/geometry/sphere.hpp
    include/renderer/hit.hpp
    include/renderer/ray.hpp
    include/utils/scene_parser.hpp
    include/renderer/camera.hpp
    include/utils/random_engine.hpp
    include/utils/image.hpp
    include/renderer/material.hpp
    include/utils/kdtree.hpp
    include/photon/photon.hpp
    include/photon/photon_map.hpp)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

ADD_EXECUTABLE(${PROJECT_NAME} ${NAIVE_RAY_TRACER_SOURCES} ${NAIVE_RAY_TRACER_INCLUDES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} vecmath)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE include)
