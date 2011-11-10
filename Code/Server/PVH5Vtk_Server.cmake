#-------------------------------------------------------------------------------
#
#  Copyright (c) 2009, 2010, Michael A. Jackson. BlueQuartz Software
#  All rights reserved.
#  BSD License: http://www.opensource.org/licenses/bsd-license.html
#
#-------------------------------------------------------------------------------
# --------------------------------------------------------------------



set (H5Vtk_Server_Wrapped_Sources    
    ${H5Vtk_SOURCE_DIR}/vtkH5PolyDataReader.cpp
    ${H5Vtk_SOURCE_DIR}/vtkH5UnstructuredGridReader.cpp
    ${H5Vtk_SOURCE_DIR}/vtkH5PolyDataWriter.cpp
    ${H5Vtk_SOURCE_DIR}/vtkH5DataReader.cpp
    ${H5Vtk_SOURCE_DIR}/vtkH5DataWriter.cpp
)
SOURCE_GROUP("H5Vtk\\\\Sources" FILES ${H5Vtk_Server_Wrapped_Sources} )

set (H5Vtk_SM_HDRS    
    ${H5Vtk_SOURCE_DIR}/vtkH5PolyDataReader.h
    ${H5Vtk_SOURCE_DIR}/vtkH5UnstructuredGridReader.h
    ${H5Vtk_SOURCE_DIR}/vtkH5PolyDataWriter.h
    ${H5Vtk_SOURCE_DIR}/vtkH5DataReader.h
    ${H5Vtk_SOURCE_DIR}/vtkH5DataWriter.h
)
SOURCE_GROUP("H5Vtk\\\\Headers" FILES ${H5Vtk_SM_HDRS} )

set (H5Vtk_Server_Sources 
    ${H5Vtk_SOURCE_DIR}/HDF5/H5Lite.cpp
    ${H5Vtk_SOURCE_DIR}/HDF5/H5Utilities.cpp
)
set (H5Vtk_HDRS 
    ${H5Vtk_SOURCE_DIR}/HDF5/H5Lite.h 
    ${H5Vtk_SOURCE_DIR}/HDF5/H5Utilities.h 
)
            
SOURCE_GROUP("H5Vtk\\\\Sources" FILES "${H5Vtk_Server_Sources}" )
SOURCE_GROUP("H5Vtk\\\\Headers" FILES "${H5Vtk_HDRS}" )

set(H5Vtk_SM_XML     ${H5Vtk_SOURCE_DIR}/H5Vtk_PVSM.xml)
set(H5Vtk_Client_XML ${H5Vtk_SOURCE_DIR}/H5Vtk_GUI.xml)

