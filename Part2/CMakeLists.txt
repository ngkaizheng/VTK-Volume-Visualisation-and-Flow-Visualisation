SET(VTK_DIR "C://Program Files//VTK//include//vtk-9.3" CACHE PATH "VTK directory override" FORCE)

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(flowVisSample)

find_package(VTK COMPONENTS 
  CommonColor
  CommonCore
  CommonDataModel
  FiltersCore
  FiltersFlowPaths
  FiltersGeometry
  FiltersGeneral
  FiltersModeling
  IOGeometry
  IOImage
  IOXML
  ImagingCore
  ImagingHybrid
  InteractionStyle
  InteractionWidgets
  RenderingAnnotation
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingVolume
  RenderingVolumeOpenGL2
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
)


if (NOT VTK_FOUND)
  message(FATAL_ERROR "flowVisSample: Unable to find the VTK build folder.")
endif()

# Prevent a "command line is too long" failure in Windows.
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")
add_executable(flowVisSample MACOSX_BUNDLE flowVisSample.cpp )
  target_link_libraries(flowVisSample PRIVATE ${VTK_LIBRARIES}
)
# vtk_module_autoinit is needed
vtk_module_autoinit(
  TARGETS flowVisSample
  MODULES ${VTK_LIBRARIES}
)
