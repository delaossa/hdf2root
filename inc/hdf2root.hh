//  Alberto Martinez de la Ossa (FLA @ DESY)
//  July 27, 2011
//  Revised: Mar 26, 2014

#ifndef HDF2ROOT
#define HDF2ROOT

#include "H5Cpp.h"
#include <TTree.h>

using namespace H5;

void help();

char map_h5class_to_root(H5T_class_t tClass);

char map_h5type_to_root(DataType type);

char map_h5type_to_root(DataType *type);

void GroupToTree(Group *group, TTree *dataTree, TTree *attrTree, Bool_t ver=kFALSE, Bool_t seq=kFALSE);

void GroupToTree(Group *group, TTree *dataTree, Bool_t seq=kFALSE, Bool_t ver=kFALSE);

#endif

