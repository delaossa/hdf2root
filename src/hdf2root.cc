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


void help() {
  printf("\n Usage: hdf2root [--seq] [--ver] [--help, -h] <inFile.h5> <outFile.root(=inFile.root)>\n");
  printf("\n  %15s  %s ","--seq","Enables sequential mode.");
  printf("\n  %15s  %s ","--ver","Enables print out.");
  printf("\n  %15s  %s ","--help, -h","Enables print out.");
  printf("\n\n");
}

int main (int argc,char *argv[]) {
  if(argc<=1) {
    help();
    return 0;
  }

  TString inFilename = "\0";
  TString outFilename = "\0";
  Bool_t SEQ = kFALSE;
  Bool_t VER = kFALSE;

  for(int l=1;l<argc;l++){
    TString arg = argv[l];
   
    if(arg.Contains("--help") || arg.Contains("-h")) {
      help();
      return 0;
    } else if(arg.EndsWith(".h5") || arg.EndsWith(".hdf5")) {
      inFilename = arg;
    } else if(arg.EndsWith(".root")) {
      outFilename = arg;
    } else if(arg.Contains("--seq")) {
      SEQ = kTRUE;
    } else if(arg.Contains("--ver")) {
      VER = kTRUE;
    } 
  }
  
  cout << "Converting " << inFilename.Data() << " ... " << endl;
  
  if(inFilename=="\0")
    return 0;
  
  if(outFilename=="\0") {
    outFilename = inFilename;
    outFilename.ReplaceAll("|","");
    outFilename.ReplaceAll(" ","_");
  
    if(outFilename.Contains(".h5"))
      outFilename.ReplaceAll(".h5",".root");
    else if(outFilename.Contains(".hdf5"))
      outFilename.ReplaceAll(".hdf5",".root");
    
  }
  
  // Open input HDF5 file
  H5File h5 = H5File(inFilename,H5F_ACC_RDONLY);
  
  // 2 TTrees will be created for every group in the HDF file:
  // One for DataSets and
  // one for Attributes.
  vector<TTree*>   dataTrees;
  vector<Group*>   groups;
  vector<TString>  groupNames;
  vector<TTree*>   attrTrees;

  // Open root group
  UInt_t gIdx = 0;
  groups.push_back(new Group(h5.openGroup("/")));
  groupNames.push_back("root");
  dataTrees.push_back(new TTree("root","DataSets in root group"));
  attrTrees.push_back(new TTree("root_att","Attributes of root group"));
  gIdx++;
  
  // Recursively check if there are other groups in root group
  for(UInt_t ig=0;ig<gIdx;ig++) {
    for(UInt_t i=0;i<groups[ig]->getNumObjs();i++) {
      if(H5G_GROUP == groups[ig]->getObjTypeByIdx(i)) {
	groupNames.push_back(groups[ig]->getObjnameByIdx(i));
	groups.push_back(new Group(groups[ig]->openGroup(groupNames[gIdx])));
	// Creates a TTree to store the DataSets in the group
	char tName[24];
	char tDesc[128];
	strcpy(tName,groupNames[gIdx].Data());
	sprintf(tDesc,"DataSets in %s group",groupNames[gIdx].Data());
	dataTrees.push_back(new TTree(tName,tDesc));
	// Creates a TTree to store the Attributes of the group
	strcpy(tName,groupNames[gIdx].Data());
	strcat(tName,"_att");
	sprintf(tDesc,"Attributes of %s group",groupNames[gIdx].Data());
	attrTrees.push_back(new TTree(tName,tDesc));
	gIdx++;
      }
    }
  }
  
  // Here the DataSets and the Attributes are
  // read from the current group and stored in TTree's
  for(UInt_t i=0;i<dataTrees.size();i++) {
    if(VER)
      cout << "Group " << i << " : " << groupNames[i] << endl;
    GroupToTree(groups[i],dataTrees[i],attrTrees[i],VER,SEQ);
  }
  
  // Writting TTrees in output file
  TFile * rfile = new TFile(outFilename,"RECREATE","Dump of HDF5 file");
  for(UInt_t i=0;i<dataTrees.size();i++) 
    dataTrees[i]->Write();
  for(UInt_t i=0;i<attrTrees.size();i++) 
    attrTrees[i]->Write();
  
  rfile->Write();
}

