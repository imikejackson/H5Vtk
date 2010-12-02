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
#project (H5Vtk)
#cmake_minimum_required(VERSION 2.6)
#if(COMMAND cmake_policy)
#      cmake_policy(SET CMP0003 NEW)
#endif(COMMAND cmake_policy)

IF (QT_USE_FILE)
  INCLUDE(${QT_USE_FILE})
ENDIF (QT_USE_FILE)
set (H5Vtk_SOURCE_DIR ${PVH5Vtk_SOURCE_DIR}/H5Vtk)
# --------------------------------------------------------------------
INCLUDE_DIRECTORIES( ${HDF5_INCLUDE_DIR}
                      ${VTK_INCLUDE_DIR}
                      ${ParaView_SOURCE_DIR}/Qt/Core
                      ${ParaView_BINARY_DIR}/Qt/Core
                      ${ParaView_SOURCE_DIR}/Utilities/VTKClientServer
                      ${ParaView_BINARY_DIR}/Utilities/VTKClientServer
                      ${ParaView_SOURCE_DIR}/Servers/Common
                      ${ParaView_BINARY_DIR}
                      ${H5Vtk_SOURCE_DIR}
)


set (H5Vtk_SM_Wrapped_SRCS    
    ${H5Vtk_SOURCE_DIR}/vtkH5PolyDataReader.cpp
    ${H5Vtk_SOURCE_DIR}/vtkH5UnstructuredGridReader.cpp
    ${H5Vtk_SOURCE_DIR}/vtkH5PolyDataWriter.cpp
    ${H5Vtk_SOURCE_DIR}/vtkH5DataReader.cpp
    ${H5Vtk_SOURCE_DIR}/vtkH5DataWriter.cpp
)
SOURCE_GROUP("H5Vtk\\\\Sources" FILES ${H5Vtk_SM_Wrapped_SRCS} )

set (H5Vtk_SM_HDRS    
    ${H5Vtk_SOURCE_DIR}/vtkH5PolyDataReader.h
    ${H5Vtk_SOURCE_DIR}/vtkH5UnstructuredGridReader.h
    ${H5Vtk_SOURCE_DIR}/vtkH5PolyDataWriter.h
    ${H5Vtk_SOURCE_DIR}/vtkH5DataReader.h
    ${H5Vtk_SOURCE_DIR}/vtkH5DataWriter.h
)
SOURCE_GROUP("H5Vtk\\\\Headers" FILES ${H5Vtk_SM_HDRS} )

set (H5Vtk_SRCS 
    ${H5Vtk_SOURCE_DIR}/HDF5/H5Lite.cpp
    ${H5Vtk_SOURCE_DIR}/HDF5/H5Utilities.cpp
)
set (H5Vtk_HDRS 
    ${H5Vtk_SOURCE_DIR}/HDF5/H5Lite.h 
    ${H5Vtk_SOURCE_DIR}/HDF5/H5Utilities.h 
)
            
SOURCE_GROUP("H5Vtk\\\\Sources" FILES "${H5Vtk_SRCS}" )
SOURCE_GROUP("H5Vtk\\\\Headers" FILES "${H5Vtk_HDRS}" )

set(H5Vtk_SM_XML     ${H5Vtk_SOURCE_DIR}/H5Vtk_PVSM.xml)
set(H5Vtk_Client_XML ${H5Vtk_SOURCE_DIR}/H5Vtk_GUI.xml)

# create a plugin
#  A plugin may contain only server code, only gui code, or both.
#  SERVER_MANAGER_SOURCES will be wrapped
#  SERVER_MANAGER_XML will be embedded and give to the client when loaded
#  SERVER_SOURCES is for other source files
#  PYTHON_MODULES allows you to embed python sources as modules
#  GUI_INTERFACES is to specify which GUI plugin interfaces were implemented
#  GUI_RESOURCES is to specify qrc files
#  GUI_RESOURCE_FILES is to specify xml files to create a qrc file from
#  GUI_SOURCES is to other GUI sources
#  SOURCES is deprecated, please use SERVER_SOURCES or GUI_SOURCES
#  REQUIRED_ON_SERVER is to specify whether this plugin should be loaded on server
#  REQUIRED_ON_CLIENT is to specify whether this plugin should be loaded on client
#  REQUIRED_PLUGINS is to specify the plugin names that this plugin depends on
#ADD_PARAVIEW_PLUGIN ("H5VtkPlugin" "1.0"
#    SERVER_MANAGER_SOURCES ${H5Vtk_SM_Wrapped_SRCS}
#    SERVER_MANAGER_XML     ${H5Vtk_SM_XML}
#    SERVER_SOURCES         ${H5Vtk_SRCS}
#    GUI_RESOURCE_FILES     ${H5Vtk_Client_XML}
#    REQUIRED_ON_SERVER )


# -- Add the Server Manager XML
PARAVIEW_INCLUDE_SERVERMANAGER_RESOURCES( "${H5Vtk_SM_XML}" )
# -- Add the Client side xml
PARAVIEW_INCLUDE_GUI_RESOURCES( "${H5Vtk_Client_XML}" )
# -- Add the wrapped sources
PARAVIEW_INCLUDE_WRAPPED_SOURCES("${H5Vtk_SM_Wrapped_SRCS}")
# -- Add additional sources that are NOT wrapped
PARAVIEW_INCLUDE_SOURCES ("${H5Vtk_SRCS}")


