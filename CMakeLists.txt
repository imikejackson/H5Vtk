#-------------------------------------------------------------------------------
#
#  Copyright (c) 2009, 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#
#-------------------------------------------------------------------------------
# --------------------------------------------------------------------
# H5P, H5U File Reader
# --------------------------------------------------------------------
PROJECT(PVH5Vtk)

cmake_minimum_required(VERSION 2.8)
IF( COMMAND CMAKE_POLICY )
  CMAKE_POLICY( SET CMP0003 NEW )
ENDIF( COMMAND CMAKE_POLICY )

set (H5Vtk_SOURCE_DIR "${PVH5Vtk_SOURCE_DIR}/Code/Server/H5Vtk")

#----
# Include the Non GUI Directories
INCLUDE_DIRECTORIES(
    ${PARAVIEW_INCLUDE_DIRS}
    ${PARAVIEW_KWSYS_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIR}
    ${HDF5_INCLUDE_DIR}
    ${ParaView_BINARY_DIR}
    "${PVH5Vtk_SOURCE_DIR}/Code/Server/H5Vtk"
)

#----
# include the Server side of the code
include ( ${PVH5Vtk_SOURCE_DIR}/Code/Server/PVH5Vtk_Server.cmake )


#----
# If we built the main ParaView Qt based app - build a client side plugin
IF (PARAVIEW_BUILD_QT_GUI)

  INCLUDE(${QT_USE_FILE})
  
  INCLUDE_DIRECTORIES(
    ${PARAVIEW_GUI_INCLUDE_DIRS}
    ${ParaView_SOURCE_DIR}/Qt/Core
    ${ParaView_BINARY_DIR}/Qt/Core
    )

    ADD_PARAVIEW_PLUGIN ("PVH5VtkPlugin" "1.0"
        SERVER_MANAGER_SOURCES ${H5Vtk_Server_Wrapped_Sources}
        SERVER_MANAGER_XML     ${H5Vtk_SM_XML}
        SERVER_SOURCES         ${H5Vtk_Server_Sources}
        GUI_RESOURCE_FILES     ${H5Vtk_Client_XML}
        REQUIRED_ON_SERVER )
        
        
ELSE (PARAVIEW_BUILD_QT_GUI)
    

    ADD_PARAVIEW_PLUGIN ("PVH5VtkPlugin" "1.0"
        SERVER_MANAGER_SOURCES ${H5Vtk_Server_Wrapped_Sources}
        SERVER_MANAGER_XML     ${H5Vtk_SM_XML}
        SERVER_SOURCES         ${H5Vtk_Server_Sources}
        GUI_RESOURCE_FILES     ${H5Vtk_Client_XML}
        REQUIRED_ON_SERVER )

    
ENDIF (PARAVIEW_BUILD_QT_GUI)

set(PVH5Vtk_INCLUDE_DIRS ${H5Vtk_SOURCE_DIR} CACHE FILEPATH "The include directory for the H5Vtk plugin" FORCE)
mark_as_advanced( PVH5Vtk_INCLUDE_DIRS)

