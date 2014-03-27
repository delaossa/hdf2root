//  Alberto Martinez de la Ossa (FLA @ DESY)
//  July 27, 2011
//  Revised: Mar 26, 2014

#include <iostream>
#include <string>
#include <vector>

#include "H5Cpp.h"

#include "hdf2root.hh"

#include <TString.h>
#include <TFile.h>

using namespace std;
using namespace H5;


// Functions and methods

char map_h5class_to_root(H5T_class_t tClass) {

  if(tClass == H5T_STRING) {
    return 'C';
  }
  return 0;

}

char map_h5type_to_root(DataType type) {
  
  if(type == PredType::NATIVE_SHORT){
    return 'B';
  }
  if(type == PredType::NATIVE_USHORT){
    return 'b';
  }
  if(type == PredType::NATIVE_INT){
    if(type.getSize() == 16)
      return 'S';
    else
      return 'I';
  }
  if(type == PredType::NATIVE_UINT){
    if(type.getSize() == 16)
      return 's';
    else
      return 'I';
  }
  if(type == PredType::NATIVE_LONG){
    return 'I';
  }
  if(type == PredType::NATIVE_ULONG){
    return 'i';
  }
  if(type == PredType::NATIVE_FLOAT){
    return 'F';
  }
  if(type == PredType::NATIVE_DOUBLE){
    return 'D';
  }
  if(type == PredType::NATIVE_CHAR){
    return 'c';
  }
  if(type == PredType::NATIVE_SCHAR){
    return 'C';
  }

  return 0;

}

char map_h5type_to_root(DataType *type) {

  cout << " ESTE NO" << endl;
  
  H5T_class_t type_class = type->getClass();
  size_t type_size = type->getSize();

  if( type_class == H5T_INTEGER ) {
    *type = PredType::NATIVE_INT; 
    return 'I';
  }

  if( type_class == H5T_FLOAT ) {
    if(type_size == 4) {
      *type = PredType::NATIVE_FLOAT;
      return 'F';
    } else if(type_size == 8) {
      *type = PredType::NATIVE_DOUBLE; 
      return 'D';
    }
  }
  
  if(type_class == H5T_STRING) {
    return 'C';
  }
  
  return 0;
  
}

void GroupToTree(Group *group, TTree *dataTree, TTree *attrTree, Bool_t ver, Bool_t seq) {

  // EXTRACT DATASETS

  // Objects and vectors for Data readout in the current group
  vector<DataSet*>    dataSets;
  vector<TString>     dataNames;
  vector<TString>     dataDesc;
  vector<hsize_t*>    dims;
  vector<hsize_t*>    offset;
  vector<hsize_t*>    count;
  vector<DataSpace*>  mem_out;
  vector<char*>       data_out;

  UInt_t iN = 0;
  for(UInt_t i=0;i<group->getNumObjs();i++) {
    
    if(H5G_DATASET != group->getObjTypeByIdx(i))
      continue;

    dataNames.push_back(group->getObjnameByIdx(i));
    if(ver)
      cout << Form(" DataSet %2i : %5s",iN,dataNames[iN].Data());

    dataSets.push_back(new DataSet(group->openDataSet(dataNames[iN].Data()))) ;
    DataSpace dataSpace = dataSets[iN]->getSpace();

    const DataType type = dataSets[iN]->getDataType();
    char rType = map_h5type_to_root(type);
    if(ver)
      cout << Form(" , type = %c",rType);
    if(rType==0) {
      if(ver)
	cout << " --> Skipping: Non-supported type! " << endl;
      dataNames.pop_back();
      dataSets.pop_back();
      continue;
    }
    
    // rank gives the dimensionality of the object
    Int_t rank = dataSpace.getSimpleExtentNdims();
    if(ver)
      cout << Form(" , rank = %1i",rank);
    
    // dims will store the number of elements in each dimension
    dims.push_back(new hsize_t[rank]);
    int ndims = dataSpace.getSimpleExtentDims(dims[iN], NULL);
    if(ver)
      cout << Form(", dim = %2i (",ndims);
    for(Int_t d=0;d<rank;d++) {
      if(ver)
	cout << Form("%2i",(UInt_t)dims[iN][d]);
      if(d<rank-1) 
	if(ver) cout << " x " ;
    }
    if(ver)
      cout << ")";

    size_t type_size = type.getSize();
    if(ver)
      cout << ", size = " << type_size;

    // Prepare the description of the TTree branch of this dataset.
    // ------------------------------------------------------------
    // ACHTUNG!: There is a difference on how the data will stored in TTree:
    // Sequentially: First dimension of arrays serves as "event index"
    // of the TTree looping.
    // Non-Seq.: First dimension is treated as any other and TTree won't loop.

    dataNames[iN].ReplaceAll("|","");
    dataNames[iN].ReplaceAll(" ","_");
    TString desc = dataNames[iN].Data();
    Int_t ifirst = 0;
    if(seq) ifirst = 1;
    for(Int_t i=ifirst;i<rank;i++) {
      char dim[64];
      sprintf(dim,"[%i]",(Int_t)dims[iN][i]);
      desc += dim;
    }
    desc += "/";
    desc += rType;
    dataDesc.push_back(desc);

    if(ver) 
      cout << Form(", branch = %8s",desc.Data()) << endl;
    
    // Arrays to read out the data
    // ------------------------------------------------
    offset.push_back(new hsize_t[rank]);
    count.push_back(new hsize_t[rank]);
    hsize_t *dims_out  = new hsize_t[rank];

    Int_t n_out = 1;
    for(Int_t idim=0;idim<rank;idim++) {
      offset[iN][idim] = 0;
      if(seq && (idim == 0) ) {
	dims_out[idim] = 1;
	count[iN][idim] = 1;
      } else {
	dims_out[idim] = dims[iN][idim];
	count[iN][idim] = dims[iN][idim];
      }
      n_out *= dims_out[idim];
    }

    data_out.push_back(new char[type_size*n_out]);
    mem_out.push_back(new DataSpace(rank,dims_out));
    // ------------------------------------------------
    iN++;
  }  
   
  // Define the branches
  for(UInt_t i=0;i<iN;i++) {
    dataTree->Branch(dataNames[i].Data(),data_out[i],dataDesc[i].Data());
  }
  
  // Actually read and fill data
  UInt_t Nseq = 1; 
  if(seq) Nseq = dims[0][0];
  for(UInt_t k=0;k<Nseq;k++) {
    for(UInt_t i=0;i<iN;i++) {
      DataSpace dataSpace = dataSets[i]->getSpace();
      dataSpace.selectHyperslab(H5S_SELECT_SET,count[i],offset[i]);
      const DataType type = dataSets[i]->getDataType();
      dataSets[i]->read(data_out[i],type,*(mem_out[i]),dataSpace);
      if(seq) // increase first dimension offset
	offset[i][0] += count[i][0];
    }
    dataTree->Fill();
  }

  // EXTRACT ATTRIBUTES

  // Objects and vectors for Attributes readout in the current group
  vector<Attribute*>   attr;
  vector<TString>      attrNames;
  vector<TString>      attrDesc;
  vector<hsize_t*>     attrDims;
  vector<char*>        attr_data_out;

  // First gather the attributes of the group:
  UInt_t iattN = 0;
  for(UInt_t i=0;i<(UInt_t)group->getNumAttrs();i++) {
    attr.push_back(new Attribute(group->openAttribute(i)));
    attrNames.push_back(attr[iattN]->getName());
    if(ver)
      cout << Form(" Attibute %2i : %18s",iattN,attrNames[iattN].Data());
    DataSpace attrSpace = attr[iattN]->getSpace();

    const DataType type = attr[iattN]->getDataType();
    char rType = map_h5type_to_root(type);
    char rClass = map_h5class_to_root(type.getClass());
    if(rType==0) {
      if(rClass!=0) {
	rType = rClass;
	if(ver)
	  cout << Form(" , class = %c",rClass);
      }	else {
	if(ver)
	  cout << " --> Skipping: Non-supported type! " << endl;
	attrNames.pop_back();
	attr.pop_back();
	continue;
      }
    } else {
      if(ver)
	cout << Form(" , type  = %c",rType);
    }
    
    // rank gives the dimensionality of the object
    Int_t rank = attrSpace.getSimpleExtentNdims();
    if(ver)
      cout << Form(" , rank = %1i",rank);
    
    // attrDims will store the number of elements in each dimension
    attrDims.push_back(new hsize_t[rank]);
    int ndims = attrSpace.getSimpleExtentDims(attrDims[iattN],NULL);
    if(ver)
      cout << Form(", dim = %2i (",ndims);
    for(Int_t d=0;d<rank;d++) {
      if(ver)
	cout << Form("%2i",(UInt_t)attrDims[iattN][d]);
      if(d<rank-1) cout << " x " ;
    }
    if(ver)
      cout << ")";

    size_t type_size = type.getSize();
    if(ver)
      cout << ", size = " << type_size ;

    attrNames[iattN].ReplaceAll("|","");
    attrNames[iattN].ReplaceAll(" ","_");
    TString desc = attrNames[iattN].Data();
    for(Int_t d=0;d<rank;d++) {
      char dim[64] = "\0";
      if(rank==1 && attrDims[iattN][d]==1) break;
      sprintf(dim,"[%i]",(Int_t)attrDims[iattN][d]);
      desc += dim;
    }
    desc += "/";
    desc += rType;
    attrDesc.push_back(desc);

    if(ver)
      cout << Form(", branch = %8s",attrDesc[iattN].Data()) << endl;
    
    // Defines the output buffer
    // ------------------------------------------------
    Int_t n_out = 1;
    for(Int_t idim=0;idim<rank;idim++) 
      n_out *= attrDims[iattN][idim];

    attr_data_out.push_back(new char[type_size*n_out]);
    // ------------------------------------------------ 
    iattN++;
  }

  // Now Attributes of the DataSets in group
  for(UInt_t iD=0;iD<iN;iD++) {
    for(UInt_t i=0;i<(UInt_t)dataSets[iD]->getNumAttrs();i++) {
      attr.push_back(new Attribute(dataSets[iD]->openAttribute(i)));
      TString attName = dataNames[iD];
      attName += "_";
      attName += attr[iattN]->getName();
      attrNames.push_back(attName);
      if(ver)
	cout << Form(" Attibute %2i : %18s",iattN,attrNames[iattN].Data());
      DataSpace attrSpace = attr[iattN]->getSpace();
      
      const DataType type = attr[iattN]->getDataType();
      char rType = map_h5type_to_root(type);
      char rClass = map_h5class_to_root(type.getClass());
      if(rType==0) {
	if(rClass!=0) {
	  rType = rClass;
	  if(ver)
	    cout << Form(" , class = %c",rClass);
	} else {
	  if(ver)
	    cout << " --> Skipping: Non-supported type! " << endl;
	  attrNames.pop_back();
	  attr.pop_back();
	  continue;
	}
      } else {
	if(ver)
	  cout << Form(" , type  = %c",rType);
      }
      
      // rank gives the dimensionality of the object
      Int_t rank = attrSpace.getSimpleExtentNdims();
      if(ver)
	cout << Form(" , rank = %1i",rank);
      
      // attrDims will store the number of elements in each dimension
      attrDims.push_back(new hsize_t[rank]);
      int ndims = attrSpace.getSimpleExtentDims(attrDims[iattN],NULL);
      if(ver)
	cout << Form(", dim = %2i (",ndims);
      for(Int_t d=0;d<rank;d++) {
	if(ver)
	  cout << Form("%2i",(UInt_t)attrDims[iattN][d]);
	if(d<rank-1) 
	  if(ver) cout << " x " ;
      }
      if(ver)
	cout << ")";
      
      size_t type_size = type.getSize();
      if(ver)
	cout << ", size = " << type_size;
      
      attrNames[iattN].ReplaceAll("|","");
      attrNames[iattN].ReplaceAll(" ","_");
      TString desc = attrNames[iattN].Data();
      for(Int_t d=0;d<rank;d++) {
	char dim[64] = "\0";
	if(rank==1 && attrDims[iattN][d]==1) break;
	sprintf(dim,"[%i]",(Int_t)attrDims[iattN][d]);
	desc += dim;
      }
      desc += "/";
      desc += rType;
      attrDesc.push_back(desc);
      
      if(ver)
	cout << Form(", branch = %8s",attrDesc[iattN].Data()) << endl;
      
      // Defines the output buffer
      // ------------------------------------------------
      Int_t n_out = 1;
      for(Int_t idim=0;idim<rank;idim++)
	n_out *= attrDims[iattN][idim];
            
      attr_data_out.push_back(new char[type_size*n_out]);
      // ------------------------------------------------ 
      iattN++;
    }
  }
  
  // Define the branches for attributes
  for(UInt_t i=0;i<attr.size();i++) {
    attrTree->Branch(attrNames[i].Data(),attr_data_out[i],attrDesc[i].Data());
  }
  
  // Actually read and fill attributes
  for(UInt_t i=0;i<attr.size();i++) {
    const DataType type = attr[i]->getDataType();
    attr[i]->read(type,attr_data_out[i]);
    // if(attrDesc[i].Contains("/C")) {
    //   TString str((char*)attr_data_out[i]);
    //   cout << str.Data() << endl;
    // }
  }
  attrTree->Fill();
}




void GroupToTree(Group *group, TTree *dataTree, Bool_t seq, Bool_t ver) {

  // EXTRACT DATASETS

  // Objects and vectors for Data readout in the current group
  vector<DataSet*>    dataSets;
  vector<TString>     dataNames;
  vector<TString>     dataDesc;
  vector<hsize_t*>    dims;
  vector<hsize_t*>    offset;
  vector<hsize_t*>    count;
  vector<DataSpace*>  mem_out;
  vector<char*>       data_out;

  UInt_t iN = 0;
  for(UInt_t i=0;i<group->getNumObjs();i++) {
    
    if(H5G_DATASET != group->getObjTypeByIdx(i))
      continue;

    dataNames.push_back(group->getObjnameByIdx(i));
    if(ver)
      cout << Form(" DataSet %2i : %5s",iN,dataNames[iN].Data());

    dataSets.push_back(new DataSet(group->openDataSet(dataNames[iN].Data()))) ;
    DataSpace dataSpace = dataSets[iN]->getSpace();

    DataType type = dataSets[iN]->getDataType();
 
    char rType = map_h5type_to_root(&type);
    char rClass = map_h5class_to_root(type.getClass());
    if(rType==0) {
      if(rClass!=0) {
	rType = rClass;
	if(ver)
	  cout << Form(" , class = %c",rClass);
      }	else {
	if(ver)
	  cout << " --> Skipping: Non-supported type! " << endl;
	dataNames.pop_back();
	dataSets.pop_back();
	continue;
      }
    } else {
      if(ver)
	cout << Form(" , type  = %c",rType);
    }
    
    // rank gives the dimensionality of the object
    Int_t rank = dataSpace.getSimpleExtentNdims();
    if(ver)
      cout << Form(" , rank = %1i",rank);
    
    // dims will store the number of elements in each dimension
    dims.push_back(new hsize_t[rank]);
    int ndims = dataSpace.getSimpleExtentDims(dims[iN], NULL);
    if(ver)
      cout << Form(", dim = %2i (",ndims);
    for(Int_t d=0;d<rank;d++) {
      if(ver)
	cout << Form("%2i",(UInt_t)dims[iN][d]);
      if(d<rank-1) 
	if(ver) cout << " x " ;
    }
    if(ver)
      cout << ")";

    size_t type_size = type.getSize();
    if(ver)
      cout << ", size = " << type_size;

    // Prepare the description of the TTree branch of this dataset.
    // ------------------------------------------------------------
    // ACHTUNG!: There is a difference on how the data will stored in TTree:
    // Sequentially: First dimension of arrays serves as "event index"
    // of the TTree looping.
    // Non-Seq.: First dimension is treated as any other and TTree won't loop.

    dataNames[iN].ReplaceAll("|","");
    dataNames[iN].ReplaceAll(" ","_");
    TString desc = dataNames[iN].Data();
    Int_t ifirst = 0;
    if(seq) ifirst = 1;
    for(Int_t i=ifirst;i<rank;i++) {
      char dim[64];
      sprintf(dim,"[%i]",(Int_t)dims[iN][i]);
      desc += dim;
    }
    desc += "/";
    desc += rType;
    dataDesc.push_back(desc);

    if(ver) 
      cout << Form(", branch = %8s",desc.Data()) << endl;
    
    // Arrays to read out the data
    // ------------------------------------------------
    offset.push_back(new hsize_t[rank]);
    count.push_back(new hsize_t[rank]);
    hsize_t *dims_out  = new hsize_t[rank];

    Int_t n_out = 1;
    for(Int_t idim=0;idim<rank;idim++) {
      offset[iN][idim] = 0;
      if(seq && (idim == 0) ) {
	dims_out[idim] = 1;
	count[iN][idim] = 1;
      } else {
	dims_out[idim] = dims[iN][idim];
	count[iN][idim] = dims[iN][idim];
      }
      n_out *= dims_out[idim];
    }

    data_out.push_back(new char[type_size*n_out]);
    mem_out.push_back(new DataSpace(rank,dims_out));
    // ------------------------------------------------
    iN++;
  }  
   
  // Define the branches
  for(UInt_t i=0;i<iN;i++) {
    dataTree->Branch(dataNames[i].Data(),data_out[i],dataDesc[i].Data());
  }

  // Actually read and fill data
  UInt_t Nseq = 1; 
  if(seq) Nseq = dims[0][0];
  for(UInt_t k=0;k<Nseq;k++) {
    for(UInt_t i=0;i<iN;i++) {
      DataSpace dataSpace = dataSets[i]->getSpace();
      dataSpace.selectHyperslab(H5S_SELECT_SET,count[i],offset[i]);
      DataType type = dataSets[i]->getDataType();
      //char rType = map_h5type_to_root(&type);
      dataSets[i]->read(data_out[i],type,*(mem_out[i]),dataSpace);
      if(seq) // increase first dimension offset
	offset[i][0] += count[i][0];
    }
    dataTree->Fill();
  }

}
