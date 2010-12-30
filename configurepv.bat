@rem /////////////////////////////////////////////////////////////////////////////
@rem //
@rem //  Copyright (c) 2009, 2010, Michael A. Jackson. BlueQuartz Software
@rem //  All rights reserved.
@rem //  BSD License: http://www.opensource.org/licenses/bsd-license.html
@rem //
@rem //
@rem ///////////////////////////////////////////////////////////////////////////////
@rem This script can be used to quickly configure ParaView to a known good build configuration in order to build
@rem the PVOIM project which contains many plugins that help with Crystal Structure visualization,
@rem specifically when used with the TSL OIM .ang data files and processed data from the DARPA D3D project.
@rem The script will configure ParaView with the PVOIM project using the PARAVIEW_EXTRA_EXTERNAL_MODULES CMake
@rem variable. This does require two passes of CMake in order to properly configure everything corretly.
@rem 
@rem  The PVOIM project depends on the source code to the MXADataModel project and will build a
@rem version of the MXADataModel library into ParaView. PVOIM also REQUIRES the use of an externally
@rem built HDF5 library as the library included with ParaView 3.8 is not new enough.
@rem 
@rem The ARCH_TYPE is predefined in another batch file that is run to setup the build enviornment. The value
@rem is either "i386" for 32 bit or "x64" for 64 bit compiles
@rem
@rem 

@echo Configuring ParaView for D3D OIM Plugins

@set WORKSPACE=C:\Users\mjackson\Workspace
@set MXADATAMODEL_DIR=%WORKSPACE%\MXADataModel
@set PARAVIEW_DIR=%WORKSPACE%\ParaView
@echo Patching in our own FindHDF5.cmake file into the ParaView distribution.
@copy %MXADATAMODEL_DIR%\Resources\cmp\Modules\FindHDF5.cmake %PARAVIEW_DIR%\VTK\CMake\FindHDF5.cmake
@copy %MXADATAMODEL_DIR%\Resources\cmp\Modules\cmpAdjustLibVars.cmake %PARAVIEW_DIR%\VTK\CMake\cmpAdjustLibVars.cmake

@cd %PARAVIEW_DIR%
@mkdir %ARCH_TYPE%
@cd %ARCH_TYPE%

@if "%ARCH_TYPE%" == "x64" (
	@set CMAKE_GENERATOR=Visual Studio 10 Win64
)
@if "%ARCH_TYPE%" == "i386" (
	@set CMAKE_GENERATOR=Visual Studio 10
)
@echo Using "%CMAKE_GENERATOR%" Generator

@echo Running First CMake pass
@%CMAKE_INSTALL%\bin\cmake.exe -G "%CMAKE_GENERATOR%" -DQT_QMAKE_EXECUTABLE=C:\Developer\x64\Qt-4.7.1\bin\qmake.exe -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug "-DPARAVIEW_EXTRA_EXTERNAL_MODULES=PVH5Vtk;PVOIM;PVDislocation;PVMXA"  -DPARAVIEW_BUILD_PLUGIN_Moments=OFF -DPARAVIEW_BUILD_PLUGIN_SLACTools=OFF -DPARAVIEW_BUILD_PLUGIN_PointSprite=OFF -DPARAVIEW_BUILD_PLUGIN_Prism=OFF -DPARAVIEW_BUILD_PLUGIN_CosmoFilters=OFF -DPARAVIEW_BUILD_PLUGIN_H5PartReader=OFF -DBUILD_TESTING=OFF -DVTK_USE_SYSTEM_HDF5=ON -DHDF5_INCLUDE_DIRS=C:\Developer\x64\hdf5-169\include "-DHDF5_LIBRARIES=optimized;C:/Developer/x64/hdf5-169/lib/hdf5dll.lib;debug;C:/Developer/x64/hdf5-169/lib/hdf5dll_D.lib" ..\ 

@echo Running Second CMake Pass
@%CMAKE_INSTALL%\bin\cmake.exe -G "%CMAKE_GENERATOR%" -DPARAVIEW_USE_PVOIM=ON -DPVOIM_SOURCE_DIR=C:\Users\mjackson\Workspace\PVOIM -DPARAVIEW_USE_PVMXA=ON -DPVMXA_SOURCE_DIR=C:\Users\mjackson\Workspace\PVMXA -DMXA_BUILD_EXAMPLES=OFF -DMXA_BUILD_UTILITIES=OFF -DPARAVIEW_USE_PVH5Vtk=ON -DPVH5Vtk_SOURCE_DIR=C:\Users\mjackson\Workspace\PVH5Vtk -DPARAVIEW_USE_PVDislocation=ON -DPVDislocation_SOURCE_DIR=C:\Users\mjackson\Workspace\PVDislocation ..\
