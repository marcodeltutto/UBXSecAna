#!/bin/bash

mkdir -p Build/
mkdir -p Files/

export UBXSecAnaRoot=`pwd`/Root/
export UBXSecAnaFluxFile=`pwd`/Flux/numode_bnb_470m_r200.root
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:`pwd`/Build/Libraries/Source/
