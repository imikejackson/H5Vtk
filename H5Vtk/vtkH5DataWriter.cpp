///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "vtkH5DataWriter.h"

#include <sstream>

//VTK/ParaView includes
#include "vtkObjectFactory.h"
#include <vtkSmartPointer.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkPolyData.h>
#include "vtkBitArray.h"
#include "vtkByteSwap.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkGraph.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkShortArray.h"
#include "vtkStringArray.h"
#include "vtkTypeTraits.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkVariantArray.h"
#include "vtkDataArray.h"
#include "vtkDataSetAttributes.h"

#include "HDF5/H5Utilities.h"

#define H5G_CREATE_GROUP(outId, parentid, name, estimate, errorReturnValue)\
  hid_t outId = H5Gcreate(parentid, name, estimate);\
  if (outId < 0) {\
    vtkErrorMacro(<< "Error creating group with name " << name);\
    return errorReturnValue;\
  }\
  if (this->Debug) {\
    vtkDebugMacro(<< "Created Group with name '" << name << "' with hdf5 id=" << outId);  }



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkCxxRevisionMacro( vtkH5DataWriter, "$Revision: 1.3 $" );
vtkStandardNewMacro( vtkH5DataWriter );


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkH5DataWriter::vtkH5DataWriter()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkH5DataWriter::~vtkH5DataWriter()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void vtkH5DataWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void vtkH5DataWriter::WriteData()
{
  vtkErrorMacro(<<"WriteData() should be implemented in concrete subclass");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::WriteDatasetArrays(hid_t parentId, vtkDataSet* ds,
                                        vtkDataSetAttributes* pd,
                                        int numPts, const char* groupName)
{
  herr_t err = 1;

  vtkDebugMacro(<<"Writing " << groupName << " data...");

  H5G_CREATE_GROUP(fp, parentId, groupName, 1, 0)
  if (fp < 0)
  {
    return -1;
  }

  // Loop on all the arrays for the Point Data and write them to the HDF5 file
  vtkAbstractArray* array = NULL;
  int nArrays = pd->GetNumberOfArrays();
  for (int i = 0; i < nArrays; ++i)
  {
    array = pd->GetAbstractArray(i);
    err = this->WriteArray(fp, array->GetDataType(), array,
                     array->GetName(),
                     numPts, array->GetNumberOfComponents() );
    if (err == 0)
    {
      std::cout << "Error writing data set attribute " << array->GetName() << std::endl;
      return err;
    }
  }


  /* We are NOT going to iterate on the "Field Data" because the above loop
   * will capture ALL the Field Data Arrays that are assigned as attributes to
   * the current data set. Then below we will interrogate each of the specific
   * types of Data Set attributes (scalars, vectors, normals, etc... ) and set
   * and HDF5 attribute onto the Group stating the name of the array for each
   * valid Active* array.
   */

  // Now Write the names of the "Active*" as HDF5 attributes to the group
  vtkDataArray* scalars = pd->GetScalars();
  if(scalars && scalars->GetNumberOfTuples() > 0)
  {
    err = H5Vtk::H5Lite::writeStringAttribute(parentId, groupName, H5_ACTIVE_SCALARS, scalars->GetName() );
    if (err < 0) { err = 0; } else { err = 1;}
  }

  vtkDataArray* vectors = pd->GetVectors();
  if(vectors && vectors->GetNumberOfTuples() > 0)
  {
    err = H5Vtk::H5Lite::writeStringAttribute(parentId, groupName, H5_ACTIVE_VECTORS, vectors->GetName() );
    if (err < 0) { err = 0; } else { err = 1;}
  }

  vtkDataArray* normals = pd->GetNormals();
  if(normals && normals->GetNumberOfTuples() > 0)
  {
    err = H5Vtk::H5Lite::writeStringAttribute(parentId, groupName, H5_ACTIVE_NORMALS, normals->GetName() );
    if (err < 0) { err = 0; } else { err = 1;}
  }

  vtkDataArray* tcoords = pd->GetTCoords();
  if(tcoords && tcoords->GetNumberOfTuples() > 0)
  {
    err = H5Vtk::H5Lite::writeStringAttribute(parentId, groupName, H5_ACTIVE_TEXTURE_COORDINATES, tcoords->GetName() );
    if (err < 0) { err = 0; } else { err = 1;}
  }

  vtkDataArray* tensors = pd->GetTensors();
  if(tensors && tensors->GetNumberOfTuples() > 0)
  {
    err = H5Vtk::H5Lite::writeStringAttribute(parentId, groupName, H5_ACTIVE_TENSORS, tensors->GetName() );
    if (err < 0) { err = 0; } else { err = 1;}
  }

  vtkDataArray* globalIds = pd->GetGlobalIds();
  if(globalIds && globalIds->GetNumberOfTuples() > 0)
  {
    err = H5Vtk::H5Lite::writeStringAttribute(parentId, groupName, H5_ACTIVE_GLOBAL_IDS, globalIds->GetName() );
    if (err < 0) { err = 0; } else { err = 1;}
  }

  vtkAbstractArray* pedigreeIds = pd->GetPedigreeIds();
  if(pedigreeIds && pedigreeIds->GetNumberOfTuples() > 0)
  {
    err = H5Vtk::H5Lite::writeStringAttribute(parentId, groupName, H5_ACTIVE_PEDIGREE_IDS, pedigreeIds->GetName() );
    if (err < 0) { err = 0; } else { err = 1;}
  }

  err = H5Gclose(fp); if (err < 0) { err = 0; } else { err = 1; } // Because HDF5 returns 0 to indicate NO_ERROR, but we need to return 0 ON error
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

int vtkH5DataWriter::WriteCells(hid_t fp, vtkCellArray *cells, const char *label)
{
  // std::cout << "   vtkH5DataWriter::WriteCells()" << std::endl;
  if ( ! cells )
    {
    return 1;
    }

  int ncells=cells->GetNumberOfCells();
  int size=cells->GetNumberOfConnectivityEntries();

  if ( ncells < 1 )
    {
    return 1;
    }
  vtkIdType *tempArray = cells->GetPointer();
  vtkTypeInt32 rank =1;
  vtkTypeUInt64 dims[1] = {size};
  herr_t err = H5Vtk::H5Lite::writePointerDataset(fp, label, rank, dims, tempArray);
  if (err < 0)
  {
    // std::cout << "Error Writing Vertices." << std::endl;
  }
  err = H5Vtk::H5Lite::writeScalarAttribute(fp, label, "Number Of Cells", ncells);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

int vtkH5DataWriter::WritePoints(hid_t fp, vtkPoints *points)
{
  // std::cout << "   vtkH5DataWriter::WritePoints()" << std::endl;
  int numPts;

  if (points == NULL)
    {
    return 1;
    }

  numPts=points->GetNumberOfPoints();

  int err = this->WriteArray(fp, points->GetDataType(), points->GetData(), H5_POINTS, numPts, 3);
  if (err != 1)
  {
    // std::cout << "Error Writing Points Array" << std::endl;
  }
  //Write the attributes to the dataset
  vtkTypeInt32 numComp = 3;
  err = H5Vtk::H5Lite::writeScalarAttribute(fp, H5_POINTS, H5_NUMCOMPONENTS, numComp);
  if (err < 0)
  {
    // std::cout << "Error Writing NumComponents attribute to dataset." << std::endl;
  }
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::WriteFieldData(hid_t parentGroup, vtkFieldData *f)
{
  // std::cout << "    vtkH5DataWriter::WriteFieldData()" << std::endl;
  // char format[1024];
  int i, numArrays = f->GetNumberOfArrays(), actNumArrays = 0;
  int numComp, numTuples;
  int attributeIndices[vtkDataSetAttributes::NUM_ATTRIBUTES];
  vtkAbstractArray *array;

  for (i = 0; i < vtkDataSetAttributes::NUM_ATTRIBUTES; i++)
  {
    attributeIndices[i] = -1;
  }
  vtkDataSetAttributes* dsa = NULL;
  if ((dsa = vtkDataSetAttributes::SafeDownCast(f)))
  {
    dsa->GetAttributeIndices(attributeIndices);
  }

  for (i = 0; i < numArrays; i++)
  {
    if (!vtkH5DataWriter::vtkIsInTheList(i, attributeIndices, vtkDataSetAttributes::NUM_ATTRIBUTES))
    {
      actNumArrays++;
    }
  }
  if (actNumArrays < 1)
  {
    return 1; // Nothing to write
  }

  herr_t err = -1;
 // hid_t fp = H5Gcreate(parentGroup, H5_FIELD_DATA_GROUP_NAME, numArrays);
  H5G_CREATE_GROUP(fp, parentGroup, H5_FIELD_DATA_GROUP_NAME, 1, 0)
  err = H5Vtk::H5Lite::writeStringAttribute(parentGroup, H5_FIELD_DATA_GROUP_NAME, H5_NAME, H5_FIELD_DATA_DEFAULT);

  for (i = 0; i < numArrays; i++)
  {
    if (!vtkIsInTheList(i, attributeIndices, vtkDataSetAttributes::NUM_ATTRIBUTES))
    {
      array = f->GetAbstractArray(i);
      if (array != NULL)
      {
        numComp = array->GetNumberOfComponents();
        numTuples = array->GetNumberOfTuples();

        // Buffer size is size of array name times four because
        // in theory there could be array name consisting of only
        // weird symbols.
        char* buffer;
        if (!array->GetName() || strlen(array->GetName()) == 0)
        {
          buffer = strcpy(new char[strlen("unknown") + 1], "unknown");
        }
        else
        {
          buffer = new char[strlen(array->GetName()) * 4 + 1];
          this->EncodeString(buffer, array->GetName(), true);
        }
        // sprintf(format, "%s %d %d %s\n", buffer, numComp, numTuples, "%s");
        this->WriteArray(fp, array->GetDataType(), array, buffer, numTuples, numComp);
        delete[] buffer;
      }

    }
  }

  err = H5Gclose(fp); if (err < 0) { err = 0; } else { err = 1; } // Because HDF5 returns 0 to indicate NO_ERROR, but we need to return 0 ON error
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// Write out data to file specified.
int vtkH5DataWriter::WriteArray(hid_t fp, int dataType, vtkAbstractArray *data,
                                const char *dsetName, int num, int numComp)
{
  // std::cout << "    vtkH5DataWriter::WriteArray()" << std::endl;
  int i;

  //char* outputFormat = new char[10];
  switch (dataType)
    {
    case VTK_BIT:
      {
        {
        unsigned char *cptr= static_cast<vtkUnsignedCharArray*>(data)->GetPointer(0);
        vtkWriteDataArray(fp, reinterpret_cast<char *>(cptr), dsetName, (sizeof(unsigned char))*((num-1)/8+1), 1);
        }
      }
    break;

    case VTK_CHAR:
      {
       char *s=static_cast<vtkCharArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s, dsetName, num, numComp);
      }
    break;

    case VTK_UNSIGNED_CHAR:
      {

      unsigned char *s= static_cast<vtkUnsignedCharArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    case VTK_SHORT:
      {

      short *s=static_cast<vtkShortArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    case VTK_UNSIGNED_SHORT:
      {

      unsigned short *s= static_cast<vtkUnsignedShortArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    case VTK_INT:
      {
      int *s=static_cast<vtkIntArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    case VTK_UNSIGNED_INT:
      {
      unsigned int *s=static_cast<vtkUnsignedIntArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    case VTK_LONG:
      {
      long *s=static_cast<vtkLongArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    case VTK_UNSIGNED_LONG:
      {
      unsigned long *s= static_cast<vtkUnsignedLongArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

#if defined(VTK_TYPE_USE___INT64)
    case VTK___INT64:
      {

      __int64_t *s= static_cast<__int64_t*>(data->GetVoidPointer(0));
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    case VTK_UNSIGNED___INT64:
      {

      unsigned __int64_t *s=  static_cast<unsigned __int64_t*>(data->GetVoidPointer(0));
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;
#endif

#if defined(VTK_TYPE_USE_LONG_LONG)
    case VTK_LONG_LONG:
      {

      long long *s= static_cast<long long*>(data->GetVoidPointer(0));
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    case VTK_UNSIGNED_LONG_LONG:
      {

      unsigned long long *s=
        static_cast<unsigned long long*>(data->GetVoidPointer(0));
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;
#endif

    case VTK_FLOAT:
      {

      float *s=static_cast<vtkFloatArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    case VTK_DOUBLE:
      {

      double *s=static_cast<vtkDoubleArray *>(data)->GetPointer(0);
      vtkWriteDataArray(fp, s , dsetName, num, numComp);
      }
    break;

    //TODO: Write the correct type in all conditions for vtkIdType
    case VTK_ID_TYPE:
      {
      // currently writing vtkIdType as int.
      int size = data->GetNumberOfTuples();
      int *intArray = new int[size*numComp];
      vtkIdType *s=static_cast<vtkIdTypeArray *>(data)->GetPointer(0);
      for (i = 0; i < size*numComp; i++)
        {
        intArray[i] = s[i];
        }
      vtkWriteDataArray(fp, intArray, dsetName, num, numComp);
      delete [] intArray;
      }
    break;
    //TODO: Write a String data set
#if 0
    case VTK_STRING:
      {

        {
        vtkStdString s;
        for (j=0; j<num; j++)
          {
          for (i=0; i<numComp; i++)
            {
            idx = i + j*numComp;
            s = static_cast<vtkStringArray *>(data)->GetValue(idx);
            vtkTypeUInt64 length = s.length();
            if (length < (static_cast<vtkTypeUInt64>(1) << 6))
              {
              vtkTypeUInt8 len = (static_cast<vtkTypeUInt8>(3) << 6)
                | static_cast<vtkTypeUInt8>(length);
              fp->write(reinterpret_cast<char*>(&len), 1);
              }
            else if (length < (static_cast<vtkTypeUInt64>(1) << 14))
              {
              vtkTypeUInt16 len = (static_cast<vtkTypeUInt16>(2) << 14)
                | static_cast<vtkTypeUInt16>(length);
              vtkByteSwap::SwapWrite2BERange(&len, 1, fp);
              }
            else if (length < (static_cast<vtkTypeUInt64>(1) << 30))
              {
              vtkTypeUInt32 len = (static_cast<vtkTypeUInt32>(1) << 30)
                | static_cast<vtkTypeUInt32>(length);
              vtkByteSwap::SwapWrite4BERange(&len, 1, fp);
              }
            else
              {
              vtkByteSwap::SwapWrite8BERange(&length, 1, fp);
              }
            fp->write(s.c_str(), length);
            }
          }
        }
      *fp << "\n";
      }
    break;
#endif

#if 0
    case VTK_VARIANT:
      {
      sprintf (str, format, "variant"); *fp << str;
      vtkVariant *v=static_cast<vtkVariantArray *>(data)->GetPointer(0);
      for (j = 0; j < num*numComp; j++)
        {
        this->EncodeWriteString(fp, v[j].ToString().c_str(), false);
        }
      }
    break;
#endif
    default:
      {
      vtkErrorMacro(<<"Type currently not supported");
      return 0;
      }
    }
  //delete[] outputFormat;

  return 1;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::vtkIsInTheList(int index, int* list, int numElem)
{
  for(int i=0; i<numElem; i++)
    {
    if (index == list[i])
      {
      return 1;
      }
    }
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// Write out scalar data.
int vtkH5DataWriter::WriteScalarData(hid_t fp, vtkDataArray *scalars, int num)
{
  // std::cout << "     vtkH5DataWriter::WriteScalarData()" << std::endl;
  int size=0;
  const char *name;
  vtkLookupTable *lut;
  int dataType = scalars->GetDataType();
  int numComp = scalars->GetNumberOfComponents();

  if ( (lut=scalars->GetLookupTable()) == NULL ||
       (size = lut->GetNumberOfColors()) <= 0 )
    {
    name = H5_DEFAULT;
    }

  char* scalarsName;
  // Buffer size is size of array name times four because
  // in theory there could be array name consisting of only
  // weird symbols.
    if (scalars->GetName() && strlen(scalars->GetName()))
      {
      scalarsName = new char[ strlen(scalars->GetName()) * 4 + 1];
      this->EncodeString(scalarsName, scalars->GetName(), true);
      }
    else
      {
      scalarsName = new char[ strlen(H5_SCALARS) + 1];
      strcpy(scalarsName, H5_SCALARS);
      }


  if ( dataType != VTK_UNSIGNED_CHAR )
    {
//    char format[1024];
//    //*fp << "SCALARS ";
//
//
//    if (numComp == 1)
//      {
//      sprintf(format,"%s %%s\nLOOKUP_TABLE %s\n", scalarsName, name);
//      }
//    else
//      {
//      sprintf(format,"%s %%s %d\nLOOKUP_TABLE %s\n",
//              scalarsName, numComp, name);
//      }

    if (this->WriteArray(fp, scalars->GetDataType(), scalars, scalarsName, num, numComp) == 0)
      {
      return 0;
      }
    delete[] scalarsName;
    }

  else //color scalars
    {
    int nvs = scalars->GetNumberOfComponents();
    //unsigned char *data = static_cast<vtkUnsignedCharArray *>(scalars)->GetPointer(0);
   // *fp << "COLOR_SCALARS " << scalarsName << " " << nvs << "\n";

   // fp->write(reinterpret_cast<char *>(data), (sizeof(unsigned char))*(nvs*num));
    this->WriteArray(fp, scalars->GetDataType(), scalars, H5_COLOR_SCALARS, num, nvs);
    delete[] scalarsName;

    }

  //if lookup table, write it out
  if ( lut && size > 0 )
    {
   // *fp << "LOOKUP_TABLE " << this->LookupTableName << " " << size << "\n";

    //unsigned char *colors=lut->GetPointer(0);
    //fp->write(reinterpret_cast<char *>(colors), (sizeof(unsigned char)*4*size));
    int numComp = 4;
    this->WriteArray(fp, lut->GetTable()->GetDataType(), lut->GetTable(), H5_LOOKUP_TABLE, size, numComp);

    }

  return 1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::WriteVectorData(hid_t fp1, vtkDataArray *vectors, int num)
{
   // std::cout << "     vtkH5DataWriter::WriteVectorData()" << std::endl;
  //hid_t fp = H5Gcreate(fp1, "VECTORS", 1);
  //H5G_CREATE_GROUP(fp, fp1, H5_VECTORS, 1, 0)

  char* vectorsName;
  // Buffer size is size of array name times four because
  // in theory there could be array name consisting of only
  // weird symbols.

  if (vectors->GetName() && strlen(vectors->GetName()))
    {
    vectorsName = new char[ strlen(vectors->GetName()) * 4 + 1];
    this->EncodeString(vectorsName, vectors->GetName(), true);
    }
  else
    {
    vectorsName = new char[ strlen(H5_VECTORS_DEFAULT) + 1];
    strcpy(vectorsName, H5_VECTORS_DEFAULT);
    }


  herr_t err = this->WriteArray(fp1, vectors->GetDataType(), vectors, vectorsName, num, 3);
  delete[] vectorsName;
  //err = H5Gclose(fp); if (err < 0) { err = 0; } else { err = 1; } // Because HDF5 returns 0 to indicate NO_ERROR, but we need to return 0 ON error
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::WriteNormalData(hid_t fp1, vtkDataArray *normals, int num)
{
  char* normalsName;
  // Buffer size is size of array name times four because
  // in theory there could be array name consisting of only
  // weird symbols.
  if (normals->GetName() && strlen(normals->GetName()) )
    {
    normalsName = new char[ strlen(normals->GetName()) * 4 + 1];
    this->EncodeString(normalsName, normals->GetName(), true);
    }
  else
    {
    normalsName = new char[ strlen(H5_NORMALS_DEFAULT) + 1];
    strcpy(normalsName, H5_NORMALS_DEFAULT);
    }

  herr_t err =  this->WriteArray(fp1, normals->GetDataType(), normals, normalsName, num, 3);
  delete[] normalsName;
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::WriteTCoordData(hid_t fp1, vtkDataArray *tcoords, int num)
{
  // std::cout << "     vtkH5DataWriter::WriteTCoordData()" << std::endl;
 //hid_t fp = H5Gcreate(fp1, "TEXTURE_COORDINATES", 1);
 //H5G_CREATE_GROUP(fp, fp1, H5_TEXTURE_COORDINATES, 1, 0)

  int dim=tcoords->GetNumberOfComponents();
 // char format[1024];

  char* tcoordsName;
  // Buffer size is size of array name times four because
  // in theory there could be array name consisting of only
  // weird symbols.

    if (tcoords->GetName() && strlen(tcoords->GetName()))
      {
      tcoordsName = new char[ strlen(tcoords->GetName()) * 4 + 1];
      this->EncodeString(tcoordsName, tcoords->GetName(), true);
      }
    else
      {
      tcoordsName = new char[ strlen(H5_TEXTURE_COORDINATES_DEFAULT) + 1];
      strcpy(tcoordsName, H5_TEXTURE_COORDINATES_DEFAULT);
      }


 // *fp << "TEXTURE_COORDINATES ";
 // sprintf(format, "%s %d %s\n", tcoordsName, dim, "%s");

  herr_t err =  this->WriteArray(fp1, tcoords->GetDataType(), tcoords, tcoordsName, num, dim);
  delete[] tcoordsName;
  //err = H5Gclose(fp); if (err < 0) { err = 0; } else { err = 1; } // Because HDF5 returns 0 to indicate NO_ERROR, but we need to return 0 ON error
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::WriteTensorData(hid_t fp1, vtkDataArray *tensors, int num)
{
  // std::cout << "     vtkH5DataWriter::WriteTensorData()" << std::endl;
 //hid_t fp = H5Gcreate(fp1, "TENSORS", 1);
 //H5G_CREATE_GROUP(fp, fp1, H5_TENSORS, 1, 0)
  //char format[1024];

  char* tensorsName;
  // Buffer size is size of array name times four because
  // in theory there could be array name consisting of only
  // weird symbols.

    if (tensors->GetName() && strlen(tensors->GetName()))
      {
      tensorsName = new char[ strlen(tensors->GetName()) * 4 + 1];
      this->EncodeString(tensorsName, tensors->GetName(), true);
      }
    else
      {
      tensorsName = new char[ strlen(H5_TENSORS_DEFAULT) + 1];
      strcpy(tensorsName, H5_TENSORS_DEFAULT);
      }


//  *fp << "TENSORS ";
//  sprintf(format, "%s %s\n", tensorsName, "%s");


  herr_t err = this->WriteArray(fp1, tensors->GetDataType(), tensors, tensorsName, num, 9);
  delete[] tensorsName;
  //err = H5Gclose(fp); if (err < 0) { err = 0; } else { err = 1; } // Because HDF5 returns 0 to indicate NO_ERROR, but we need to return 0 ON error
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::WriteGlobalIdData(hid_t fp1, vtkDataArray *globalIds, int num)
{
  // std::cout << "     vtkH5DataWriter::WriteGlobalIdData()" << std::endl;
// hid_t fp = H5Gcreate(fp1, "GLOBAL_IDS", 1);
// H5G_CREATE_GROUP(fp, fp1, H5_GLOBAL_IDS, 1, 0)
  //*fp << "GLOBAL_IDS ";

  char* globalIdsName;
  // Buffer size is size of array name times four because
  // in theory there could be array name consisting of only
  // weird symbols.

    if (globalIds->GetName() && strlen(globalIds->GetName()))
      {
      globalIdsName = new char[ strlen(globalIds->GetName()) * 4 + 1];
      this->EncodeString(globalIdsName, globalIds->GetName(), true);
      }
    else
      {
      globalIdsName = new char[ strlen(H5_GLOBAL_IDS_DEFAULT) + 1];
      strcpy(globalIdsName, H5_GLOBAL_IDS_DEFAULT);
      }

//  sprintf(format, "%s %s\n", globalIdsName, "%s");


  herr_t err = this->WriteArray(fp1, globalIds->GetDataType(), globalIds, globalIdsName, num, 1);
  delete[] globalIdsName;
 // err = H5Gclose(fp); if (err < 0) { err = 0; } else { err = 1; } // Because HDF5 returns 0 to indicate NO_ERROR, but we need to return 0 ON error
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::WritePedigreeIdData(hid_t fp1, vtkAbstractArray *pedigreeIds, int num)
{
  // std::cout << "     vtkH5DataWriter::WritePedigreeIdData()" << std::endl;
// hid_t fp = H5Gcreate(fp1, "PEDIGREE_IDS", 1);
 //H5G_CREATE_GROUP(fp, fp1, H5_PEDIGREE_IDS, 1, 0)

  //*fp << "PEDIGREE_IDS ";

  char* pedigreeIdsName;
  // Buffer size is size of array name times four because
  // in theory there could be array name consisting of only
  // weird symbols.

    if (pedigreeIds->GetName() && strlen(pedigreeIds->GetName()))
      {
      pedigreeIdsName = new char[ strlen(pedigreeIds->GetName()) * 4 + 1];
      this->EncodeString(pedigreeIdsName, pedigreeIds->GetName(), true);
      }
    else
      {
      pedigreeIdsName = new char[ strlen(H5_PEDIGREE_IDS_DEFAULT) + 1];
      strcpy(pedigreeIdsName, H5_PEDIGREE_IDS_DEFAULT);
      }

  //sprintf(format, "%s %s\n", pedigreeIdsName, "%s");

  herr_t err = this->WriteArray(fp1, pedigreeIds->GetDataType(), pedigreeIds, pedigreeIdsName, num, 1);
  delete[] pedigreeIdsName;
  //err = H5Gclose(fp); if (err < 0) { err = 0; } else { err = 1; } // Because HDF5 returns 0 to indicate NO_ERROR, but we need to return 0 ON error
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int vtkH5DataWriter::writeObjectIndex(hid_t fileId, std::vector<std::string> &hdfPaths)
{
  herr_t err = 0;

  err = H5Vtk::H5Utilities::createGroupsFromPath(H5_VTK_OBJECT_INDEX_PATH, fileId);
  if (err < 0)
  {
    std::cout << "Error creating HDF Group " << H5_VTK_OBJECT_INDEX_PATH << std::endl;
  }

  hid_t gid = H5Gopen(fileId, H5_VTK_OBJECT_INDEX_PATH);
  if(gid < 0)
  {
    std::cout << "Error writing string attribute to HDF Group " << H5_VTK_OBJECT_INDEX_PATH << std::endl;
  }

  std::stringstream ss;
  std::vector<std::string>::size_type stop = hdfPaths.size();
  std::vector<std::string>::size_type p = 0;
  for (p = 0; p < stop; ++p)
  {
    ss.str("");
    ss << p;
    err = H5Vtk::H5Lite::writeStringDataset(gid, ss.str(), hdfPaths[p]);
    if (err < 0)
    {
      std::cout << "Error writing VTK Object Index" << std::endl;
    }
  }
  err = H5Gclose(gid);


  return err;
}
