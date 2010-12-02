///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) ,
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
//  This code was written under United States Air Force Contract number
//                           FA8650-04-C-5229
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _VTKH5DATAWRITER_H_
#define _VTKH5DATAWRITER_H_


#include <vtkWriter.h>

//-- Hdf5 includes
#include <hdf5.h>

//-- Our Constants
#include "VTKH5Constants.h"
#include "HDF5/H5Lite.h"


#ifndef VTK_EXPORT
#define VTK_EXPORT
#endif

class vtkPolyData;
class vtkFieldData;
class vtkAbstractArray;
class vtkDataSet;
class vtkPoints;
class vtkCellArray;
class vtkDataArray;
class vtkDataSetAttributes;

/**
* @class vtkH5DataWriter vtkH5DataWriter.h HDF5/vtkH5DataWriter.h
* @brief
* @author Mike Jackson for IMTS.us
* @date April 2008
* @version $Revision: 1.2 $
*/
class VTK_EXPORT vtkH5DataWriter : public vtkWriter
{

public:
  static vtkH5DataWriter* New();
  vtkTypeRevisionMacro( vtkH5DataWriter, vtkWriter );
  void PrintSelf( ostream&, vtkIndent );

  int WriteDatasetArrays(hid_t parentId, vtkDataSet* ds,
                                          vtkDataSetAttributes* pd,
                                          int numTuples, const char* groupName);

  int WritePoints(hid_t fp, vtkPoints *points);
  //TODO:: Implement WriteCoordinates
 // int WriteCoordinates(hid_t fp, vtkDataArray *coords, int axes);
  int WriteCells(hid_t fp, vtkCellArray *cells, const char *label);
//  int WriteCellData1(hid_t fp, vtkDataSet *ds);
//  int WritePointData1(hid_t fp, vtkDataSet *ds);

#if 0
  int WriteEdgeData(ostream *fp, vtkGraph *g);
  int WriteVertexData(ostream *fp, vtkGraph *g);
  int WriteRowData(ostream *fp, vtkTable *g);
#endif

  int WriteFieldData(hid_t parentGroup, vtkFieldData *f);
  //int WriteDataSetData1(hid_t parentGroup, vtkDataSet *ds);


protected:
  vtkH5DataWriter();
  ~vtkH5DataWriter();

  virtual void WriteData();

  int WriteArray(hid_t fp, int dataType, vtkAbstractArray *data,
                 const char *dsetName, int num, int numComp);
  int WriteScalarData(hid_t fp, vtkDataArray *s, int num);
  int WriteVectorData(hid_t fp, vtkDataArray *v, int num);
  int WriteNormalData(hid_t fp, vtkDataArray *n, int num);
  int WriteTCoordData(hid_t fp, vtkDataArray *tc, int num);
  int WriteTensorData(hid_t fp, vtkDataArray *t, int num);
  int WriteGlobalIdData(hid_t fp, vtkDataArray *g, int num);
  int WritePedigreeIdData(hid_t fp, vtkAbstractArray *p, int num);

  int vtkIsInTheList(int index, int* list, int numElem);

  //BTX
  // Template to handle writing data in ascii or binary
  // We could change the format into C++ io standard ...
  template <class T>
  void vtkWriteDataArray(hid_t fp, T *data, const char *dsetName,
                         int num, int numComp)
  {
    // std::cout << "      vtkH5DataWriter::vtkWriteDataArray<T>()" << std::endl;
    vtkTypeInt32 rank = 1;
    vtkTypeUInt64 dims[1] = { (vtkTypeUInt64)num * (vtkTypeUInt64)numComp};
    std::string name (dsetName);
    herr_t err = H5Vtk::H5Lite::writePointerDataset(fp, name, rank, dims, data);
    if (err < 0)
    {
      std::cout << "Error writing array with name: " << std::string (dsetName) << std::endl;
    }
    err = H5Vtk::H5Lite::writeScalarAttribute(fp, name, std::string(H5_NUMCOMPONENTS), numComp);
  }
  //ETX

private:
  vtkH5DataWriter(const vtkH5DataWriter&);  // Not implemented.
  void operator=(const vtkH5DataWriter&);  // Not implemented.

};


#endif /* _VTKH5DATAWRITER_H_ */
