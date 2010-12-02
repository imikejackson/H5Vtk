#-------------------------------------------------------------------------------
#
#  Copyright (c) 2009, 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#
#-------------------------------------------------------------------------------

# specify the name of the module
SET(MODULE_NAME PVH5Vtk)
SET(BUILD_PARAVIEW_PLUGINLIST ON)


# This may fail the first time through? 
GET_TARGET_PROPERTY(PARAVIEW_PROCESS_XML_EXECUTABLE kwProcessXML LOCATION)

include( ${PVH5Vtk_SOURCE_DIR}/H5Vtk/H5Vtk.cmake )


set(PVH5Vtk_INCLUDE_DIRS ${PVH5Vtk_SOURCE_DIR}/H5Vtk CACHE FILEPATH "The include directory for the H5Vtk plugin")
mark_as_advanced( PVH5Vtk_INCLUDE_DIRS)

