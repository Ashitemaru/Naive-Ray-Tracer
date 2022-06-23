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
    src/scene_parser.cpp
    src/mesh.cpp
    src/octree.cpp)

SET(NAIVE_RAY_TRACER_INCLUDES
    include/geometry/group.hpp
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
    include/photon/photon_map.hpp
    include/renderer/renderer.hpp
    include/renderer/light.hpp
    include/utils/trans.hpp
    include/geometry/object3d.hpp
    include/geometry/plane.hpp
    include/geometry/rectangle.hpp
    include/geometry/transform.hpp
    include/geometry/triangle.hpp
    include/geometry/bbox.hpp
    include/geometry/mesh.h
    include/utils/octree.h)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

FIND_PACKAGE(OpenMP REQUIRED)
IF (OPENMP_FOUND)
    SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS})
    SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS})
    SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS})
ENDIF ()

ADD_EXECUTABLE(${PROJECT_NAME} ${NAIVE_RAY_TRACER_SOURCES} ${NAIVE_RAY_TRACER_INCLUDES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} vecmath)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE include)
