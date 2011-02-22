///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "vtkH5DataReader.h"
#include "VTKH5Constants.h"

#include <vector>
#include <list>

#include "HDF5/H5Lite.h"
#include "HDF5/H5Utilities.h"


#include "vtkAbstractArray.h"
#include "vtkBitArray.h"
#include "vtkByteSwap.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkRectilinearGrid.h"
#include "vtkShortArray.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkTypeInt64Array.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkVariantArray.h"
#include <vtksys/ios/sstream>

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


vtkCxxRevisionMacro(vtkH5DataReader, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkH5DataReader);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkH5DataReader::vtkH5DataReader()
{
 // std::cout << "vtkH5DataReader Constructor" << std::endl;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  FileName = NULL;
  ScalarsName = NULL;
  VectorsName = NULL;
  TensorsName = NULL;
  TCoordsName = NULL;
  NormalsName = NULL;
  LookupTableName = NULL;
  FieldDataName = NULL;
  ScalarLut = NULL;
  InputString = NULL;
  Header = NULL;
  InputArray = NULL;

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkH5DataReader::~vtkH5DataReader()
{
  //std::cout << "vtkH5DataReader Destructor" << std::endl;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void vtkH5DataReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataReader::ProcessRequest(vtkInformation* request,
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector* outputVector)
{
  // generate the data
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
    return this->RequestData(request, inputVector, outputVector);
    }

  if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
    {
    return this->RequestUpdateExtent(request, inputVector, outputVector);
    }

  // execute information
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
    return this->RequestInformation(request, inputVector, outputVector);
    }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// Read point coordinates. Return 0 if error.
int vtkH5DataReader::ReadPoints(hid_t parentId, const std::string &dsetName, vtkPointSet* ps)
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
vtkAbstractArray* vtkH5DataReader::ReadArray(hid_t parentId, const std::string& dsetName)
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
  vtkTypeUInt8* dest = NULL;
  switch(attr_type)
  {
  case H5T_STRING:
    res.clear(); //Clear the string out first
    array = vtkStringArray::New();
    array->SetNumberOfComponents(numComp);
    array->Allocate(numElements, 100);
    dest = static_cast<vtkTypeUInt8*>(array->GetVoidPointer(0) );
    if (NULL != dest) {
      err = H5Vtk::H5Lite::readPointerDataset(parentId, dsetName, dest);
    }
    break;
  case H5T_INTEGER:
    if ( H5Tequal(typeId, H5T_STD_U8BE) || H5Tequal(typeId,H5T_STD_U8LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkUnsignedChar, numComp, numElements, parentId, dsetName, vtkTypeUInt8 );
     } else if ( H5Tequal(typeId, H5T_STD_U16BE) || H5Tequal(typeId,H5T_STD_U16LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkUnsignedShort, numComp, numElements, parentId, dsetName, vtkTypeUInt16 );
     } else if ( H5Tequal(typeId, H5T_STD_U32BE) || H5Tequal(typeId,H5T_STD_U32LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkUnsignedInt, numComp, numElements, parentId, dsetName, vtkTypeUInt32 );
     } else if ( H5Tequal(typeId, H5T_STD_U64BE) || H5Tequal(typeId,H5T_STD_U64LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkTypeUInt64, numComp, numElements, parentId, dsetName, vtkTypeUInt64 );
     } else if ( H5Tequal(typeId, H5T_STD_I8BE) || H5Tequal(typeId,H5T_STD_I8LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkChar, numComp, numElements, parentId, dsetName, char );
     } else if ( H5Tequal(typeId, H5T_STD_I16BE) || H5Tequal(typeId,H5T_STD_I16LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkShort, numComp, numElements, parentId, dsetName, vtkTypeInt16);
     } else if ( H5Tequal(typeId, H5T_STD_I32BE) || H5Tequal(typeId,H5T_STD_I32LE) ) {
       ALLOCATE_AND_READ_ARRAY(array, vtkInt, numComp, numElements, parentId, dsetName, vtkTypeInt32);
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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataReader::ReadDataSetArrays(vtkDataSet *ds, vtkDataSetAttributes *a, int num,
                                      hid_t parentId, hid_t gid, const char* groupName )
{
  hsize_t nObjs = 0;
  herr_t err = H5Gget_num_objs(gid, &nObjs);
  char name[1024];


  // Loop on all the arrays for this data set
  for (hsize_t i = 0; i < nObjs; ++i)
  {
    ::memset(name, 0, 1024);
    err = H5Gget_objname_by_idx(gid, i, name, 1024);
    //TODO: Error trap here.

    err = ReadDataHelper(a, num, gid, name);
    if (err == 0)
    {
      vtkErrorMacro(<< "Could not successfully read data set attribute array with name '" << name << "'");
      return 0;
    }
  }


  std::string data;
  err =  H5Vtk::H5Lite::readStringAttribute(parentId, groupName, H5_ACTIVE_SCALARS, data);
  if (data.size() > 0)
  {
    a->SetActiveScalars(data.c_str());
  }
  data.clear();

  err =  H5Vtk::H5Lite::readStringAttribute(parentId, groupName, H5_ACTIVE_VECTORS, data);
  if (data.size() > 0)
  {
    a->SetActiveVectors(data.c_str());
  }
  data.clear();

  err =  H5Vtk::H5Lite::readStringAttribute(parentId, groupName, H5_ACTIVE_NORMALS, data);
  if (data.size() > 0)
  {
    a->SetActiveNormals(data.c_str());
  }
  data.clear();

  err =  H5Vtk::H5Lite::readStringAttribute(parentId, groupName, H5_ACTIVE_TEXTURE_COORDINATES, data);
  if (data.size() > 0)
  {
    a->SetActiveTCoords(data.c_str());
  }
  data.clear();

  err =  H5Vtk::H5Lite::readStringAttribute(parentId, groupName, H5_ACTIVE_TENSORS, data);
  if (data.size() > 0)
  {
    a->SetActiveTensors(data.c_str());
  }
  data.clear();

  err =  H5Vtk::H5Lite::readStringAttribute(parentId, groupName, H5_ACTIVE_GLOBAL_IDS, data);
  if (data.size() > 0)
  {
    a->SetActiveGlobalIds(data.c_str());
  }
  data.clear();

  err =  H5Vtk::H5Lite::readStringAttribute(parentId, groupName, H5_ACTIVE_PEDIGREE_IDS, data);
  if (data.size() > 0)
  {
    a->SetActivePedigreeIds(data.c_str());
  }
  data.clear();

  return 1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// Read the cell data of a vtk data file. The number of cells (from the
// dataset) must match the number of cells defined in cell attributes (unless
// no geometry was defined).
int vtkH5DataReader::ReadCellData(vtkDataSet *ds, hid_t parentId, hid_t gid)
{
  //char line[256];
  vtkDataSetAttributes *a=ds->GetCellData();
  vtkIdType num = ds->GetNumberOfCells();
  vtkDebugMacro(<< "Reading vtk cell data");

  return ReadDataSetArrays(ds, a, num, parentId, gid, H5_CELL_DATA_GROUP_NAME);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// Read the point data of a vtk data file. The number of points (from the
// dataset) must match the number of points defined in point attributes (unless
// no geometry was defined).
int vtkH5DataReader::ReadPointData(vtkDataSet *ds, hid_t parentId, hid_t gid)
{
  vtkDataSetAttributes* a=ds->GetPointData();
  vtkIdType num = ds->GetNumberOfPoints();
  vtkDebugMacro(<< "Reading vtk point data");

  return ReadDataSetArrays(ds, a, num, parentId, gid, H5_POINT_DATA_GROUP_NAME);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkFieldData* vtkH5DataReader::ReadFieldData(hid_t parentId, hid_t gid)
{
  int skipField=0;
  hsize_t numArrays;
  vtkFieldData *f = NULL;
  char name[256], type[256], fieldName[256];
  ::memset(name, 0, 256);
  ::memset(type, 0, 256);
  ::memset(fieldName, 0, 256);

  //int numComp, numTuples;
  vtkAbstractArray *data = NULL;
  herr_t err = -1;
  hsize_t nameSize = 0;
  //Read the name as an attribute from the "FIELD_DATA" group
  err = H5Vtk::H5Lite::readStringAttribute(parentId, H5_FIELD_DATA_GROUP_NAME, H5_NAME, (unsigned char*)name);
  err = H5Gget_num_objs(gid, &numArrays);
  //TODO: Implement Error Trapping

  // See whether field data name (if specified)
  if ( this->FieldDataName && strcmp( (const char*)(name), (const char*)(this->FieldDataName) ) )
    {
    skipField = 1;
    }

  f = vtkFieldData::New();
  f->AllocateArrays(numArrays);
  char buffer[1024];
  ::memset(buffer, 0, 1024);
  // Read the number of arrays specified
  for (size_t i = 0; i < numArrays; i++)
  {

    nameSize = H5Gget_objname_by_idx(gid, i, buffer, 1024);
    if (nameSize < 0) { continue; } // Could not read this data set so skip it
    if (strcmp(buffer, "NULL_ARRAY") == 0)
    {
      continue;
    }
//    this->DecodeString(name, buffer);
//    this->Read(&numComp);
//    this->Read(&numTuples);
//    this->ReadString(type);
    data = this->ReadArray(gid, std::string(buffer, nameSize));
    if (data != NULL)
    {
      data->SetName(buffer);
      if (!skipField || this->ReadAllFields)
      {
        f->AddArray(data);
      }
      data->Delete();
    }
    else
    {
      f->Delete();
      return NULL;
    }
  }

  if ( skipField && ! this->ReadAllFields )
    {
    f->Delete();
    return NULL;
    }
  else
    {
    return f;
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataReader::ReadDataHelper(vtkDataSetAttributes *a, int num,
                                     hid_t gid,
                                    const char* name)
{
  int err = 0;
 // int skipNormal=0;
  std::string dsName;
  vtkDataArray *data = NULL;

  // try reading the data set
  data = vtkDataArray::SafeDownCast( this->ReadArray(gid, name));
  dsName = name;

  if (data != NULL)
  {
    if (num != data->GetNumberOfTuples()) // Number of cells or points must match
    {
      data->Delete();
      err = 0;
    }
    data->SetName(dsName.c_str());
    a->AddArray(data);
    err = 1;
  }
  else
  {
    err = 0;
  }

  float progress = this->GetProgress();
  this->UpdateProgress(progress + 0.5*(1.0 - progress));
  return err;
}


std::vector<std::string> vtkH5DataReader::ReadObjectIndex(hid_t file_id)
{
  hid_t dataDimId = H5Gopen(file_id, H5_VTK_OBJECT_INDEX_PATH);
  std::list<std::string> names;
  herr_t err = H5Vtk::H5Utilities::getGroupObjects(dataDimId, H5Vtk::H5Utilities::MXA_DATASET,  names);
  std::vector<std::string> objects;
  for (std::list<std::string>::iterator iter = names.begin(); iter != names.end(); ++iter)
  {
    std::string data;
    H5Vtk::H5Lite::readStringDataset(dataDimId, *iter, data);
    objects.push_back(data);
  }
  H5Gclose(dataDimId);
  return objects;
}

