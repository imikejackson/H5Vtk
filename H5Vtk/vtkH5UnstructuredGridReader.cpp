///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "vtkH5UnstructuredGridReader.h"

#include "VTKH5Constants.h"
#include "HDF5/H5Lite.h"
#include "HDF5/H5Utilities.h"

#include <vtkUnstructuredGrid.h>
#include <vtkCellArray.h>
#include <vtkFieldData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkAbstractArray.h>
#include <vtkBitArray.h>
#include <vtkByteSwap.h>
#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkErrorCode.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkShortArray.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStringArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkAppendPolyData.h>

vtkCxxRevisionMacro(vtkH5UnstructuredGridReader, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkH5UnstructuredGridReader);


// We only have vtkTypeUInt64Array if we have long long
// or we have __int64_t with conversion to double.
#if defined(VTK_TYPE_USE_LONG_LONG) || (defined(VTK_TYPE_USE___INT64) && defined(VTK_TYPE_CONVERT_UI64_TO_DOUBLE))
#include "vtkTypeUInt64Array.h"
#endif

#include <ctype.h>
#include <sys/stat.h>

#define ALLOCATE_AND_READ_ARRAY(array, VTK_TYPE, numComp, numTuples, parentId, dsetName, dType ) \
array = VTK_TYPE##Array::New();\
array->SetNumberOfComponents(numComp);\
dType* dest = static_cast<dType*>( ((VTK_TYPE##Array*)array)->WritePointer(0,numTuples));\
if (NULL != dest) {\
  err = H5Vtk::H5Lite::readPointerDataset(parentId, dsetName, dest);\
}\




//----------------------------------------------------------------------------
vtkH5UnstructuredGridReader::vtkH5UnstructuredGridReader()
{
//  std::cout << "vtkH5UnstructuredGridReader Constructor" << std::endl;
  this->FileName = NULL;
  this->HDFPath = NULL;
  this->SetNumberOfInputPorts(0);
  this->HDFError = 0;
}

//----------------------------------------------------------------------------
vtkH5UnstructuredGridReader::~vtkH5UnstructuredGridReader()
{
  this->SetFileName(NULL);
  this->SetHDFPath(NULL);
 // std::cout << "========> vtkH5UnstructuredGridReader Destructor" << std::endl;
}

//----------------------------------------------------------------------------
int vtkH5UnstructuredGridReader::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkUnstructuredGrid");
  return 1;
}

#if 1
//----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkH5UnstructuredGridReader::GetOutput()
{
  return this->GetOutput(0);
}

//----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkH5UnstructuredGridReader::GetOutput(int idx)
{
 // std::cout << "++{vtkH5UnstructuredGridReader::GetOutput} Ref Count: " << this->GetOutputDataObject(idx)->GetReferenceCount() << std::endl;
  return vtkUnstructuredGrid::SafeDownCast(this->GetOutputDataObject(idx));
}

//----------------------------------------------------------------------------
void vtkH5UnstructuredGridReader::SetOutput(vtkUnstructuredGrid *output)
{
  this->GetExecutive()->SetOutputData(0, output);
}

#endif

//----------------------------------------------------------------------------
int vtkH5UnstructuredGridReader::RequestData( vtkInformation *vtkNotUsed(request),
                                      vtkInformationVector **vtkNotUsed(inputVector),
                                      vtkInformationVector *outputVector)
{
  herr_t err = -1;
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast( outInfo->Get(vtkDataObject::DATA_OBJECT()));

  HDF_ERROR_HANDLER_OFF;
  hid_t fileId = H5Vtk::H5Utilities::openFile(this->FileName, false);
  // Something went wrong either opening or creating the file. Error messages have
  // Alread been written at this point so just return.
  if (fileId < 0)
  {
  // std::cout << logTime() << "The hdf5 file could not be opened.\n The Given filename was:\n\t[" << this->FileName << "]" << std::endl;
   this->HDFError= fileId;
   return 1;
  }

  vtkUnstructuredGrid* p = loadUnstructuredGridData(fileId, this->HDFPath);
  if (NULL != p)
  {
      output->ShallowCopy(p);
      p->Delete();
    }

  // Close the file
  err = H5Vtk::H5Utilities::closeFile(fileId);
  this->HDFError = err;
  HDF_ERROR_HANDLER_ON;
  return 1;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkH5UnstructuredGridReader::loadUnstructuredGridData(hid_t fileId, const std::string &hdfpath)
{
  vtkDebugMacro(<<"Reading vtk polygonal data...");
  vtkUnstructuredGrid* output = NULL;
  herr_t err = 0;
  // now Open the "Dataset", which is actually a group
//  hid_t rootId = H5Vtk::H5Utilities::openHDF5Object(fileId, hdfpath);
  hid_t rootId = H5Gopen(fileId, hdfpath.c_str());

  if (rootId < 0)
  {
    //std::cout << logTime() << "The Group " << this->HDFPath << " could not be found."<< std::endl;
//    err = H5Vtk::H5Utilities::closeFile(fileId);
    this->HDFError = rootId;
    return output;
  }

  if (rootId > 0)
  {
    //
    // Make sure we're reading right type of geometry
    //
    std::string dataObjectType;
    err = H5Vtk::H5Lite::readStringAttribute(fileId, hdfpath, H5_VTK_DATA_OBJECT, dataObjectType);
    if (err < 0)
    {
      std::cout << "Could not find the 'VTK_DATA_OBJECT' attribute for HDF group " << this->HDFPath << ". This is needed to read the file." << std::endl;
      err = H5Gclose(rootId);
      err = H5Vtk::H5Utilities::closeFile(fileId);
      this->HDFError = err;
      return output;
    }
    else
    {
      if (dataObjectType.compare(H5_VTK_UNSTRUCTURED_GRID) != 0)
      {
        std::cout << "HDF Group " << hdfpath << " is NOT type vtkUnstructuredGrid. It is " << dataObjectType << std::endl;
        err = H5Gclose(rootId);
        err = H5Vtk::H5Utilities::closeFile(fileId);
        this->HDFError = -1;
        return output;
      }
    }

    // Now read the points, vertices, lines, polygons and triangle strips
    output = vtkUnstructuredGrid::New();

    // Read the POINTS
    err = this->ReadPoints(rootId, H5_POINTS, output);
    //        if (err == 0 )
    //        {
    //          std::cout << "Error Reading Points data." << std::endl;
    //          return 1;
    //        }


    //Read the VERTICES from the file
    vtkCellArray* cells = vtkCellArray::New();
    vtkIntArray*  cell_types = vtkIntArray::New();
    err = readCells(output, rootId, cells, H5_CELLS);
    if (err == 1)
    {
      err = readCellTypes(output, rootId, cell_types, H5_CELL_TYPES);
      if (err >= 0)
      {
        output->SetCells(cell_types->GetPointer(0), cells);
      }
      else
      {
        this->HDFError = -1;
        return output;
      }
    }
    else
    {
      this->HDFError = -1;
      return output;
    }
    cells->Delete();
    cell_types->Delete();

    output->BuildLinks();
    output->ComputeBounds();

    // Read any FIELD_DATA
    hid_t gid = H5Gopen(rootId, H5_FIELD_DATA_GROUP_NAME);
    if (gid > 0)
    {
      vtkFieldData* fd = this->ReadFieldData(rootId, gid);
      output->SetFieldData(fd);
      fd->Delete(); // ?
      H5Gclose(gid);
    }

    // Read CELL_DATA
    gid = H5Gopen(rootId, H5_CELL_DATA_GROUP_NAME);
    if (gid > 0)
    {
      int err = this->ReadCellData(output, rootId, gid);
      H5Gclose(gid);
      if (err == 0)
      {
        H5Gclose(rootId);
        H5Vtk::H5Utilities::closeFile(fileId);
        return output;
      }
    }

    // Read any POINT_DATA
    gid = H5Gopen(rootId, H5_POINT_DATA_GROUP_NAME);
    if (gid > 0)
    {
      int err = this->ReadPointData(output, rootId, gid);
      H5Gclose(gid);
      if (err == 0)
      {
        H5Gclose(rootId);
        H5Vtk::H5Utilities::closeFile(fileId);
        return output;
      }
    }

    if (!output->GetPoints())
    vtkWarningMacro(<<"No points read!");

    err = H5Gclose(rootId);
    if (err < 0)
    {
      err = 0;
    }
    else
    {
      err = 1;
    } // Because HDF5 returns 0 to indicate NO_ERROR, but we need to return 0 ON error

  }

  return output;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5UnstructuredGridReader::readCells(vtkUnstructuredGrid* output,
                                           hid_t rootId,
                                           vtkCellArray* cells,
                                           const std::string &dsetname)
{
  int  ncells, i;
  herr_t err = -1;
  vtkAbstractArray* array = this->ReadArray(rootId, dsetname);
  if (NULL != array)
  {
    vtkIdType size = array->GetSize();
    // Read the number of cells
    err = H5Vtk::H5Lite::readScalarAttribute(rootId, dsetname, "Number Of Cells", ncells);
    hid_t typeId = H5Vtk::H5Lite::getDatasetType(rootId, dsetname);
    if (typeId < 0)
    {
      return 0;
    }

    vtkIdTypeArray* data = vtkIdTypeArray::New();
    vtkIdType* dataPtr = data->WritePointer(0, size);
    if (H5Tequal(typeId, H5T_NATIVE_INT64))
    {
      vtkTypeInt64Array* arrayPtr = vtkTypeInt64Array::SafeDownCast(array);
      vtkTypeInt64* ptr = arrayPtr->GetPointer(0);
      for (i = 0; i < size; i++)
      {
        dataPtr[i] = static_cast<vtkIdType>(ptr[i]);
      }
    }
    else
    {
      vtkIntArray* arrayPtr = vtkIntArray::SafeDownCast(array);
      vtkTypeInt32* ptr = arrayPtr->GetPointer(0);
      for (i = 0; i < size; i++)
      {
        dataPtr[i] = static_cast<vtkIdType>(ptr[i]);
      }
    }
    cells->SetCells(ncells, data);
    data->Delete();


    err = H5Tclose(typeId);
    if (err < 0)
    {
      //std::cout << "Error closing typeId for " << dsetname << std::endl;
      return 0;
    }
    return 1;
  }

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5UnstructuredGridReader::readCellTypes(vtkUnstructuredGrid* output,
                          hid_t parentId,
                          vtkIntArray* cell_types,
                          const std::string &dsetName)
{
  if (parentId < 0)
  {
    return -1;
  }
  herr_t err = -1;
  hid_t did = -1;
  H5T_class_t attr_type;
  size_t attr_size;

  did = H5Dopen(parentId, dsetName.c_str());
  if (did < 0)
  {
    return -1;
  }
  else
  {
    H5Dclose(did);
  }
  vtkTypeInt32 numComp = 1;

  std::vector<hsize_t > dims; //Reusable for the loop
  err = H5Vtk::H5Lite::getDatasetInfo(parentId, dsetName, dims, attr_type, attr_size);
  if (err < 0)
  {
    return -1;
  }
  if (dims.size() == 0)
  {
    vtkDebugMacro ( << "vtkH5DataReader::ReadArray(): dims.size() == 0. This is REALLY BAD." );
    return -1;
  }

  vtkIdType numElements = 1;
  for (std::vector<vtkTypeUInt64 >::size_type i = 0; i < dims.size(); ++i)
  {
    numElements = numElements * dims[i];
  }

  cell_types->SetNumberOfComponents(numComp);
  vtkTypeInt32* dest = static_cast<vtkTypeInt32* > (((vtkIntArray*)cell_types)->WritePointer(0, numElements));
  if (0 != dest)
  {
    err = H5Vtk::H5Lite::readPointerDataset(parentId, dsetName, dest);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void vtkH5UnstructuredGridReader::PrintSelf(std::ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

#if 0
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// Read point coordinates. Return 0 if error.
int vtkH5UnstructuredGridReader::ReadPoints(hid_t parentId, const std::string &dsetName, vtkPointSet* ps)
{
  vtkAbstractArray* absArray = this->ReadArray(parentId, dsetName);
  vtkDataArray* data;
  data = vtkDataArray::SafeDownCast(absArray );
  if ( data != NULL )
    {
    vtkPoints* points=vtkPoints::New();
    points->SetData(data);
    data->Delete();
    ps->SetPoints(points);
    points->Delete();
    }
  else
    {
    return 0;
    }

  vtkDebugMacro( <<"Read " << ps->GetNumberOfPoints() << " points" );

  return 1;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkAbstractArray* vtkH5UnstructuredGridReader::ReadArray(hid_t parentId, const std::string& dsetName)
{
  vtkAbstractArray* array = NULL;
  if (parentId < 0)
  {
    return array;
  }
  herr_t err = -1;
  herr_t retErr = 1;
  hid_t typeId = -1;
  H5T_class_t attr_type;
  size_t attr_size;
  std::string res;
  hid_t did = -1;


  did = H5Dopen( parentId, dsetName.c_str() );
  if ( did  < 0 ) {
   return array;
  }
  else
  {
    H5Dclose(did);
  }

  vtkTypeInt32 numComp = 1;
  err = H5Vtk::H5Lite::readScalarAttribute(parentId, dsetName, H5_NUMCOMPONENTS, numComp);
  if (err < 0 )
  {
    vtkDebugMacro(<< "Error reading 'NumComponents' attribute from the dataset " << dsetName);
    numComp = 1;
  }
  std::vector<hsize_t> dims;  //Reusable for the loop
  err = H5Vtk::H5Lite::getDatasetInfo(parentId, dsetName, dims, attr_type, attr_size);
  if (err < 0 )
  {
    return array;
  }
  if(dims.size() == 0)
  {
    vtkDebugMacro ( << "vtkH5DataReader::ReadArray(): dims.size() == 0. This is REALLY BAD." );
    return array;
  }
  vtkIdType numElements = 1;
  for (std::vector<vtkTypeUInt64>::size_type i = 0; i < dims.size(); ++i)
  {
    numElements = numElements * dims[i];
  }
  typeId = H5Vtk::H5Lite::getDatasetType(parentId, dsetName);
  if (typeId < 0)
  {
    return array;
  }
  uint8_t* dest = NULL;
  switch(attr_type)
  {
  case H5T_STRING:
    res.clear(); //Clear the string out first
    array = vtkStringArray::New();
    array->SetNumberOfComponents(numComp);
    array->Allocate(numElements, 100);
    dest = static_cast<uint8_t*>(array->GetVoidPointer(0) );
    if (NULL != dest) {
      err = H5Vtk::H5Lite::readPointerDataset(parentId, dsetName, dest);
    }
    break;
  case H5T_INTEGER:
    if ( H5Tequal(typeId, H5T_STD_U8BE) || H5Tequal(typeId,H5T_STD_U8LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkUnsignedChar, numComp, numElements, parentId, dsetName, uint8_t );
     } else if ( H5Tequal(typeId, H5T_STD_U16BE) || H5Tequal(typeId,H5T_STD_U16LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkUnsignedShort, numComp, numElements, parentId, dsetName, uint16_t );
     } else if ( H5Tequal(typeId, H5T_STD_U32BE) || H5Tequal(typeId,H5T_STD_U32LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkUnsignedInt, numComp, numElements, parentId, dsetName, uint32_t );
     } else if ( H5Tequal(typeId, H5T_STD_U64BE) || H5Tequal(typeId,H5T_STD_U64LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkTypeUInt64, numComp, numElements, parentId, dsetName, vtkTypeUInt64 );
     } else if ( H5Tequal(typeId, H5T_STD_I8BE) || H5Tequal(typeId,H5T_STD_I8LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkChar, numComp, numElements, parentId, dsetName, char );
     } else if ( H5Tequal(typeId, H5T_STD_I16BE) || H5Tequal(typeId,H5T_STD_I16LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkShort, numComp, numElements, parentId, dsetName , int16_t);
     } else if ( H5Tequal(typeId, H5T_STD_I32BE) || H5Tequal(typeId,H5T_STD_I32LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkInt, numComp, numElements, parentId, dsetName ,vtkTypeInt32);
     } else if ( H5Tequal(typeId, H5T_STD_I64BE) || H5Tequal(typeId,H5T_STD_I64LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkTypeInt64, numComp, numElements, parentId, dsetName, vtkTypeInt64 );
    } else {
      std::cout << "Unknown Type: " << typeId << " at " <<  dsetName << std::endl;
      err = -1;
      retErr = -1;
    }
    break;
  case H5T_FLOAT:
    if (attr_size == 4) {
      ALLOCATE_AND_READ_ARRAY(array, vtkFloat, numComp, numElements, parentId, dsetName, float );
    } else if (attr_size == 8 ) {
      ALLOCATE_AND_READ_ARRAY(array, vtkDouble, numComp, numElements, parentId, dsetName, double );
    } else {
      std::cout << "Unknown Floating point type" << std::endl;
      err = -1;
      retErr = -1;
    }
    break;
  default:
    std::cout << "Error: H5Vtk::H5Utilities::readDatasetArray() Unknown attribute type: " << attr_type << std::endl;
    H5Vtk::H5Utilities::printHDFClassType(attr_type);
  }
  CloseH5T(typeId, err, retErr); //Close the H5A type Id that was retrieved during the loop

  return array;
}
#endif



