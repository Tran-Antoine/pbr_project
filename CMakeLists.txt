cmake_minimum_required (VERSION 3.8)
project(nori)

add_subdirectory(ext ext_build)

set(OpenCV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/opencv/build)
# Find OpenCV package
find_package(OpenCV REQUIRED)

include_directories(
  # Nori include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include/nori
  # tinyformat string formatting library
  ${TFM_INCLUDE_DIR}
  # Eigen linear algebra library
  SYSTEM ${EIGEN_INCLUDE_DIR}
  # OpenEXR high dynamic range bitmap library
  SYSTEM ${OPENEXR_INCLUDE_DIRS}
  # Intel Thread Building Blocks
  SYSTEM ${TBB_INCLUDE_DIR}
  # Pseudorandom number generator
  ${PCG32_INCLUDE_DIR}
  # PugiXML parser
  ${PUGIXML_INCLUDE_DIR}
  # Helper functions for statistical hypothesis tests
  ${HYPOTHESIS_INCLUDE_DIR}
  # GLFW library for OpenGL context creation
  SYSTEM ${GLFW_INCLUDE_DIR}
  # GLEW library for accessing OpenGL functions
  SYSTEM ${GLEW_INCLUDE_DIR}
  # NanoVG drawing library
  SYSTEM ${NANOVG_INCLUDE_DIR}
  # NanoGUI user interface library
  SYSTEM ${NANOGUI_INCLUDE_DIR}
  SYSTEM ${NANOGUI_EXTRA_INCS}
  # Portable filesystem API
  SYSTEM ${FILESYSTEM_INCLUDE_DIR}
  # STB Image Write
  SYSTEM ${STB_IMAGE_WRITE_INCLUDE_DIR}
  # OpenCV
  SYSTEM ${OPENCV_INCLUDE_DIR}
)

# The following lines build the main executable. If you add a source
# code file to Nori, be sure to include it in this list.
add_executable(nori

  # Header files
  include/nori/core/bbox.h
  include/nori/core/camera.h
  include/nori/core/color.h
  include/nori/core/common.h
  include/nori/core/frame.h
  include/nori/core/mesh.h
  include/nori/core/object.h
  include/nori/core/ray.h
  include/nori/core/timer.h
  include/nori/core/transform.h
  include/nori/core/vector.h
  include/nori/core/scene.h

  include/nori/gui/bitmap.h
  include/nori/gui/block.h

  include/nori/bsdf/bsdf.h
  include/nori/bsdf/diffusemap.h
  include/nori/bsdf/uniformmap.h
  include/nori/bsdf/texturemap.h

  include/nori/collection/accel.h
  include/nori/collection/octree.h

  include/nori/stats/dpdf.h
  include/nori/stats/sampler.h
  include/nori/stats/warp.h
  include/nori/stats/perlin.h
  include/nori/stats/fbm.h

  include/nori/integrator/integrator.h

  include/nori/emitter/area.h
  include/nori/emitter/emitter.h
  
  include/nori/parser/parser.h
  include/nori/parser/proplist.h

  include/nori/postprocessing/rfilter.h
  
  
  # Source code files
  src/core/common.cpp
  src/core/object.cpp
  src/core/mesh.cpp
  src/core/perspective.cpp
  src/core/scene.cpp

  src/gui/bitmap.cpp
  src/gui/block.cpp
  src/gui/gui.cpp

  src/bsdf/diffuse.cpp
  src/bsdf/microfacet.cpp
  src/bsdf/mirror.cpp
  src/bsdf/dielectric.cpp
  src/bsdf/uniformmap.cpp
  src/bsdf/texturemap.cpp

  src/collection/accel.cpp
  src/collection/octree.cpp

  src/stats/chi2test.cpp
  src/stats/independent.cpp
  src/stats/ttest.cpp
  src/stats/warp.cpp
  src/stats/perlin.cpp
  src/stats/fbm.cpp

  src/integrator/pointlight.cpp
  src/integrator/normals.cpp
  src/integrator/ambient.cpp
  src/integrator/whitted.cpp
  src/integrator/path_simple.cpp
  src/integrator/path_mis.cpp

  src/emitter/area.cpp

  src/parser/obj.cpp
  src/parser/heightmap.cpp
  src/parser/parser.cpp
  src/parser/proplist.cpp
  src/postprocessing/rfilter.cpp

  src/main.cpp
  #test/perlin_test.cpp
  #test/fbm_test.cpp
)

# Link OpenCV libraries
#target_link_libraries(nori PRIVATE ${OpenCV_LIBS})
# The following lines build the warping test application


add_definitions(${NANOGUI_EXTRA_DEFS})



if (WIN32)
  target_link_libraries(nori tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS} zlibstatic ${OpenCV_LIBS})
else()
  target_link_libraries(nori tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS} ${OpenCV_LIBS})
endif()

#target_link_libraries(warptest tbb_static nanogui ${NANOGUI_EXTRA_LIBS})

# Force colored output for the ninja generator
if (CMAKE_GENERATOR STREQUAL "Ninja")
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  endif()
endif()

#target_compile_features(warptest PRIVATE cxx_std_17)
target_compile_features(nori PRIVATE cxx_std_17)

# vim: set et ts=2 sw=2 ft=cmake nospell:
