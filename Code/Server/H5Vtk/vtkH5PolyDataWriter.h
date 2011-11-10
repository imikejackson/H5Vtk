///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
//  This code was written under United States Air Force Contract number
//                           FA8650-04-C-5229
//
///////////////////////////////////////////////////////////////////////////////
#ifndef VTKH5POLYDATAWRITER_H_
#define VTKH5POLYDATAWRITER_H_


#include "vtkH5DataWriter.h"

//-- Hdf5 includes
#include <hdf5.h>

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

/**
* @class vtkH5PolyDataWriter vtkH5PolyDataWriter.h UtilityFilters/vtkH5PolyDataWriter.h
* @brief This class writes a vtkPolyData object to an HDF5 based file
* @author Mike Jackson for IMTS.us
* @date April 2008
* @version $Revision: 1.2 $
*/
class VTK_EXPORT vtkH5PolyDataWriter : public vtkH5DataWriter
{
public:
  static vtkH5PolyDataWriter* New();
  vtkTypeRevisionMacro( vtkH5PolyDataWriter, vtkH5DataWriter );
  void PrintSelf( ostream&, vtkIndent );

  // Description:
  // Specify file name of vtk polygon data file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  vtkSetStringMacro(HDFPath);
  vtkGetStringMacro(HDFPath);

  vtkSetMacro(AppendData, vtkTypeInt32);
  vtkGetMacro(AppendData, vtkTypeInt32);
  vtkBooleanMacro(AppendData, vtkTypeInt32);

  // Description:
  // Get the input to this writer.
  vtkPolyData* GetInput();
  vtkPolyData* GetInput(int port);

  //BTX
  int writeVtkObjectIndex(std::vector<std::string> &paths);
  //ETX

protected:
	vtkH5PolyDataWriter();
	~vtkH5PolyDataWriter();

  virtual void WriteData();

  /**
  * @brief Standard vtk 5.x pipeline method. This method is used to set what type
  * of outputs this filter produces.
  * @param port The port to get output information for
  * @param information The vtkInformation pointer
  * @return 1 on success, 0 on error
  */
  virtual int FillInputPortInformation(int port, vtkInformation* information);

private:

  char* FileName;

  char* HDFPath;

  vtkTypeInt32 AppendData;


  vtkH5PolyDataWriter(const vtkH5PolyDataWriter&);  // Not implemented.
  void operator=(const vtkH5PolyDataWriter&);  // Not implemented.


};

#endif /*VTKH5POLYDATAWRITER_H_*/
