///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, mjackson
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
//  This code was written under United States Air Force Contract number
//                           FA8650-04-C-5229
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _VTKH5POLYDATAREADER_H_
#define _VTKH5POLYDATAREADER_H_

//-- HDF5 includes
#include <hdf5.h>

//-- Superclass
#include "vtkH5DataReader.h"

class vtkInformation;
class vtkInformationVector;
class vtkPolyData;
class vtkCellArray;


/**
* @class vtkH5PolyDataReader vtkH5PolyDataReader.h HDF5/vtkH5PolyDataReader.h
* @brief The class reads vtkPolyData Objects from an HDF5 file. It borrows heavily
* from the vtkPolyDataReader class from the vtk package.
* @author Mike Jackson
* @date April 2008
* @version $Revision: 1.5 $
*/
class VTK_EXPORT vtkH5PolyDataReader : public vtkH5DataReader
{
public:
  static vtkH5PolyDataReader *New();
  vtkTypeRevisionMacro(vtkH5PolyDataReader,vtkH5DataReader);

  //BTX
  /**
   * @brief Prints information about this class. Good for debugging
   * @param os Output stream to write to
   * @param indent The amount of indentation
   */
  void PrintSelf(std::ostream& os, vtkIndent indent);
//ETX

  // Description:
  // Specify file name of vtk polygon data file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  vtkSetStringMacro(HDFPath);
  vtkGetStringMacro(HDFPath);


  // Description:
  // Get the output of this reader.
#if 1
  vtkPolyData *GetOutput();
  vtkPolyData *GetOutput(int idx);
  void SetOutput(vtkPolyData *output);
#endif

  vtkSetMacro(HDFError, int);
  vtkGetMacro(HDFError, int);

  //BTX
   /**
    * @brief Loads the vtkPolyData object from the HDF5 file using the given hdf path
    * @param fileId The HDF5 fileId
    * @param hdfpath The internal hdf5 path to the data set
    * @return NULL pointer if error, otherwise valid vtkPolyData object
    */
   virtual vtkPolyData* loadPolyData(hid_t fileId, const std::string &hdfpath);
   //ETX

protected:
  vtkH5PolyDataReader();
 ~vtkH5PolyDataReader();

 /**
 * @brief Standard vtk 5.x pipeline method. This method is used to set what type
 * of outputs this filter produces.
 * @param port The port to get output information for
 * @param information The vtkInformation pointer
 * @return 1 on success, 0 on error
 */
 virtual int FillOutputPortInformation(int port, vtkInformation* information);


 /**
  * @brief Standard Method to over ride in the VTK 5.x pipeline
  * @param vtkNotUsed vtkInformation Object
  * @param vtkNotUsed vtkInformationVector for the inputs
  * @param outputVector vtkInformationVector for the outputs
  * @return 1 on success, 0 on error
  */
 virtual int RequestData( vtkInformation *vtkNotUsed(request),
                  vtkInformationVector **vtkNotUsed(inputVector),
                  vtkInformationVector *outputVector);


//BTX
  /**
   * @brief
   * @param output
   * @param rootId
   * @param verts
   * @param dsetname
   */
  virtual int readCells(vtkPolyData* output, hid_t rootId, vtkCellArray* verts, const std::string &dsetname);
  //ETX

private:

  char* FileName;
  char* HDFPath;
  int HDFError;


  vtkH5PolyDataReader(const vtkH5PolyDataReader&);  // Not implemented.
  void operator=(const vtkH5PolyDataReader&);  // Not implemented.
};

#endif


