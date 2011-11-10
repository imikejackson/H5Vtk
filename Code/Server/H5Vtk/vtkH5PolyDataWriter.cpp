///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "vtkH5PolyDataWriter.h"
#include "VTKH5Constants.h"

#include "HDF5/H5Utilities.h"

#include "vtkObjectFactory.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkPolyData.h>
#include <vtkFieldData.h>
#include <vtkDataSetAttributes.h>
#include <vtkCellData.h>
#include <vtkPointData.h>

//-- Our includes


// #include <MXA/HDF5/H5Utilities.h>
// #include <MXA/Utilities/StringUtils.h>

#define APPEND_DATA_TRUE 1
#define APPEND_DATA_FALSE 0


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkCxxRevisionMacro( vtkH5PolyDataWriter, "$Revision: 1.4 $" );
vtkStandardNewMacro( vtkH5PolyDataWriter );


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkH5PolyDataWriter::vtkH5PolyDataWriter()
{
  this->FileName = NULL;
  this->HDFPath = NULL;
  this->SetNumberOfInputPorts(1);
  this->AppendData = APPEND_DATA_TRUE;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkH5PolyDataWriter::~vtkH5PolyDataWriter()
{
  this->SetFileName( NULL );
  this->SetHDFPath(NULL);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void vtkH5PolyDataWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5PolyDataWriter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkPolyData* vtkH5PolyDataWriter::GetInput()
{
  return vtkPolyData::SafeDownCast(this->Superclass::GetInput());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkPolyData* vtkH5PolyDataWriter::GetInput(int port)
{
  return vtkPolyData::SafeDownCast(this->Superclass::GetInput(port));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void vtkH5PolyDataWriter::WriteData()
{
  // std::cout << "  vtkH5PolyDataWriter::WriteData() Starting" << std::endl;

  hid_t fileId = -1;
  // Try to open a file to append data into
  if (APPEND_DATA_TRUE == this->AppendData)
  {
    fileId = H5Vtk::H5Utilities::openFile(this->FileName, false);
  }
  // No file was found or we are writing new data only to a clean file
  if (APPEND_DATA_FALSE == this->AppendData || fileId < 0)
  {
    fileId = H5Vtk::H5Utilities::createFile (this->FileName);
  }

  //Something went wrong either opening or creating the file. Error messages have
  // Alread been written at this point so just return.
  if (fileId < 0)
  {
    // std::cout << logTime() << "The hdf5 file could not be opened or created.\n The Given filename was:\n\t[" << this->FileName << "]" << std::endl;
  }


  herr_t err = H5Vtk::H5Utilities::createGroupsFromPath(this->HDFPath, fileId);
  if (err < 0)
  {
    H5Vtk::H5Utilities::closeFile(fileId);
    return;
  }
  hid_t fp = H5Gopen(fileId, this->HDFPath, H5P_DEFAULT );
  err = H5Vtk::H5Lite::writeStringAttribute(fileId, this->HDFPath, H5_VTK_DATA_OBJECT, H5_VTK_POLYDATA );
  if(err < 0)
  {
    H5Vtk::H5Utilities::closeFile(fileId);
    return;
  }
  vtkPolyData *input = this->GetInput();
  // Write data owned by the dataset
  int errorOccured = 0;
  vtkFieldData* field = input->GetFieldData();
  if (field && field->GetNumberOfTuples() > 0)
  {
    if (!this->WriteFieldData(fp, field))
    {
      errorOccured = 1; // we tried to write field data, but we couldn't
    }
  }

  if (!errorOccured && this->WritePoints(fp, input->GetPoints()) < 0 )
    {
    // std::cout << "Error writing Points" << std::endl;
    errorOccured = 1;
    }

  if (!errorOccured && input->GetVerts())
    {
    if (this->WriteCells(fp, input->GetVerts(),H5_VERTICES) < 0)
      {
      // std::cout << "Error writing Vertices" << std::endl;
      errorOccured = 1;
      }
    }

  if (!errorOccured && input->GetLines())
    {
    if (this->WriteCells(fp, input->GetLines(),H5_LINES) < 0)
      {
      errorOccured = 1;
      }
    }

  if (!errorOccured && input->GetPolys())
    {
    if (this->WriteCells(fp, input->GetPolys(),H5_POLYGONS) < 0)
      {
      errorOccured = 1;
      }
    }

  if (!errorOccured && input->GetStrips())
    {
    if (this->WriteCells(fp, input->GetStrips(),H5_TRIANGLE_STRIPS) < 0)
      {
      errorOccured = 1;
      }
    }

  vtkCellData* cd = input->GetCellData();
  if (!errorOccured && (this->WriteDatasetArrays(fp, input, cd, input->GetNumberOfCells(), H5_CELL_DATA_GROUP_NAME) < 0) )
    {
    errorOccured = 1;
    }

  vtkPointData* pd = input->GetPointData();
  if (!errorOccured && (this->WriteDatasetArrays(fp, input, pd, input->GetNumberOfPoints(), H5_POINT_DATA_GROUP_NAME) < 0) )
    {
    errorOccured = 1;
    }

  if(errorOccured)
    {
    vtkErrorMacro(<< "Error occured writing PolyData to HDF5 file.")
    }


  // Close the PolyData group when we are finished with it
  err = H5Gclose(fp);
  if (err < 0)
  {
    // std::cout << "Error closing group: " << this->HDFPath << std::endl;
  }

  // Close the file when we are finished with it
  H5Vtk::H5Utilities::closeFile(fileId);
  // std::cout << "  vtkH5PolyDataWriter::WriteData() Ending" << std::endl;
}

int vtkH5PolyDataWriter::writeVtkObjectIndex(std::vector<std::string> &paths)
{
  hid_t fileId = -1;
  herr_t err = 0;
  // Try to open a file
  fileId = H5Vtk::H5Utilities::openFile(this->FileName, false);
  if (fileId < 0)
  {
    return -1;
  }
  err = this->writeObjectIndex(fileId, paths);

  // Close the file when we are finished with it
  H5Vtk::H5Utilities::closeFile(fileId);

  return err;
}
