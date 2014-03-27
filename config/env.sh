#!/bin/bash

# MAC flag
export PLATFORM=macosx

# Setting up Root environment....
export ROOTSYS=/usr/local/root
export DYLD_LIBRARY_PATH=$ROOTSYS/lib:$DYLD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH

# HDF5: C++ interface
export HDF5CPP=/usr/local/hdf5-c++
export DYLD_LIBRARY_PATH=$HDF5CPP/lib:$DYLD_LIBRARY_PATH


