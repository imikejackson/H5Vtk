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

#ifndef __vtkH5DataReader_h
#define __vtkH5DataReader_h

//-- C++ includes
#include <string>

//-- VTK includes
#include <vtkAlgorithm.h>

//-- HDF5 includes
#include <hdf5.h>


#define VTK_BINARY 2

class vtkAbstractArray;
class vtkCharArray;
class vtkDataSet;
class vtkDataSetAttributes;
class vtkFieldData;
class vtkGraph;
class vtkPointSet;
class vtkRectilinearGrid;

class VTK_EXPORT vtkH5DataReader : public vtkAlgorithm
{
public:
  static vtkH5DataReader *New();
  vtkTypeRevisionMacro(vtkH5DataReader,vtkAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file name of vtk data file to read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  // Description:
  // Set the name of the scalar data to extract. If not specified, first
  // scalar data encountered is extracted.
  vtkSetStringMacro(ScalarsName);
  vtkGetStringMacro(ScalarsName);

  // Description:
  // Set the name of the vector data to extract. If not specified, first
  // vector data encountered is extracted.
  vtkSetStringMacro(VectorsName);
  vtkGetStringMacro(VectorsName);

  // Description:
  // Set the name of the tensor data to extract. If not specified, first
  // tensor data encountered is extracted.
  vtkSetStringMacro(TensorsName);
  vtkGetStringMacro(TensorsName);

  // Description:
  // Set the name of the normal data to extract. If not specified, first
  // normal data encountered is extracted.
  vtkSetStringMacro(NormalsName);
  vtkGetStringMacro(NormalsName);

  // Description:
  // Set the name of the texture coordinate data to extract. If not specified,
  // first texture coordinate data encountered is extracted.
  vtkSetStringMacro(TCoordsName);
  vtkGetStringMacro(TCoordsName);

  // Description:
  // Set the name of the lookup table data to extract. If not specified, uses
  // lookup table named by scalar. Otherwise, this specification supersedes.
  vtkSetStringMacro(LookupTableName);
  vtkGetStringMacro(LookupTableName);

  // Description:
  // Set the name of the field data to extract. If not specified, uses
  // first field data encountered in file.
  vtkSetStringMacro(FieldDataName);
  vtkGetStringMacro(FieldDataName);


//BTX
  // Description:
  // Read the cell data of a vtk data file. The number of cells (from the
  // dataset) must match the number of cells defined in cell attributes (unless
  // no geometry was defined).
  int ReadCellData(vtkDataSet *ds, hid_t parentId, hid_t gids);


  // Description:
  // Read the point data of a vtk data file. The number of points (from the
  // dataset) must match the number of points defined in point attributes
  // (unless no geometry was defined).
  int ReadPointData(vtkDataSet *ds, hid_t parentId, hid_t gids);


  // Description:
  // Read point coordinates. Return 0 if error.
  int ReadPoints(hid_t parentId, const std::string &dsetName, vtkPointSet* ps);

#if 0
  // Description:
  // Read the vertex data of a vtk data file. The number of vertices (from the
  // graph) must match the number of vertices defined in vertex attributes
  // (unless no geometry was defined).
  int ReadVertexData(vtkGraph *g, int numVertices);

  // Description:
  // Read the edge data of a vtk data file. The number of edges (from the
  // graph) must match the number of edges defined in edge attributes
  // (unless no geometry was defined).
  int ReadEdgeData(vtkGraph *g, int numEdges);

  // Description:
  // Read a bunch of "cells". Return 0 if error.
  int ReadCells(int size, int *data);

  // Description:
  // Read a piece of the cells (for streaming compliance)
  int ReadCells(int size, int *data, int skip1, int read2, int skip3);
#endif

  // Description:
  // Helper functions for reading data.
  vtkAbstractArray* ReadArray(hid_t parentId, const std::string &dsetName);

  /**
   * @brief
   * @param parentId The parent Id of gid
   * @param gid The hdf5 id for the group
   */
  vtkFieldData*  ReadFieldData(hid_t parentId, hid_t gid);

#if 0
  // Description:
  // Close the vtk file.
  void CloseVTKFile();


  // Description:
  // Read the meta information from the file.  This needs to be public to it
  // can be accessed by vtkDataSetReader.
  virtual int ReadMetaData(vtkInformation *) { return 1; }
#endif

//ETX

protected:
  vtkH5DataReader();
   ~vtkH5DataReader();

  char *FileName;
  int FileType;

  char *ScalarsName;
  char *VectorsName;
  char *TensorsName;
  char *TCoordsName;
  char *NormalsName;
  char *LookupTableName;
  char *FieldDataName;
  char *ScalarLut;

  int ReadFromInputString;
  char *InputString;
  int InputStringLength;
  int InputStringPos;

  void SetScalarLut(const char* lut);
  vtkGetStringMacro(ScalarLut);

  char *Header;

  //int ReadScalarData(vtkDataSetAttributes *a, int num);
//  int ReadVectorData(vtkDataSetAttributes *a, int num, hid_t parentId, hid_t gid);
//  int ReadNormalData(vtkDataSetAttributes *a, int num, hid_t parentId, hid_t gid);
 // int ReadTensorData(vtkDataSetAttributes *a, int num);
//  int ReadCoScalarData(vtkDataSetAttributes *a, int num, hid_t parentId, hid_t gid);
  //int ReadLutData(vtkDataSetAttributes *a);
//  int ReadTCoordsData(vtkDataSetAttributes *a, int num);
//  int ReadGlobalIds(vtkDataSetAttributes *a, int num);
//  int ReadPedigreeIds(vtkDataSetAttributes *a, int num);

  int ReadDataHelper(vtkDataSetAttributes *a, int num,
                                      hid_t gid,
                                      const char* name);

  int ReadDataSetArrays(vtkDataSet *ds, vtkDataSetAttributes *a, int num,
                                       hid_t parentId, hid_t gid, const char* groupName);

  // This supports getting additional information from vtk files
  int  NumberOfScalarsInFile;
  char **ScalarsNameInFile;
  int ScalarsNameAllocSize;
  int  NumberOfVectorsInFile;
  char **VectorsNameInFile;
  int VectorsNameAllocSize;
  int  NumberOfTensorsInFile;
  char **TensorsNameInFile;
  int TensorsNameAllocSize;
  int  NumberOfTCoordsInFile;
  char **TCoordsNameInFile;
  int TCoordsNameAllocSize;
  int  NumberOfNormalsInFile;
  char **NormalsNameInFile;
  int NormalsNameAllocSize;
  int  NumberOfFieldDataInFile;
  char **FieldDataNameInFile;
  int FieldDataNameAllocSize;
  vtkTimeStamp CharacteristicsTime;

  int ReadAllScalars;
  int ReadAllVectors;
  int ReadAllNormals;
  int ReadAllTensors;
  int ReadAllColorScalars;
  int ReadAllTCoords;
  int ReadAllFields;

  vtkCharArray* InputArray;

  // Description:
  // Decode a string. This method is the inverse of
  // vtkWriter::EncodeString.  Returns the length of the
  // result string.
  int DecodeString(char *resname, const char* name);

  virtual int ProcessRequest(vtkInformation *, vtkInformationVector **,
                             vtkInformationVector *);

  virtual int RequestData(vtkInformation *, vtkInformationVector **,
                          vtkInformationVector *)
    { return 1; }
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **,
                                  vtkInformationVector *)
    { return 1; }
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
                                 vtkInformationVector *)
    { return 1; }

private:
  vtkH5DataReader(const vtkH5DataReader&);  // Not implemented.
  void operator=(const vtkH5DataReader&);  // Not implemented.
};

#endif


