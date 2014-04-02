#!/bin/bash

# Setting up Root environment....
export ROOTSYS=/Users/delaossa/local/root
export DYLD_LIBRARY_PATH=$ROOTSYS/lib:$DYLD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH

# HDF5: C++ interface
export HDF5CPP=/usr/local/opt/hdf5
export DYLD_LIBRARY_PATH=$HDF5CPP/lib:$DYLD_LIBRARY_PATH

# HDF2ROOT environment:
export HDF2ROOT=/Users/delaossa/plasma/software/hdf2root
export DYLD_LIBRARY_PATH=$HDF2ROOT/lib:$DYLD_LIBRARY_PATH
export PATH=$HDF2ROOT/bin:$PATH

