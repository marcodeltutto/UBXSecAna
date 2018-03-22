# UBXSecAna

This code take 3 input ROOT files that are produced by the UBXSec code in https://github.com/marcodeltutto/UBXSec. First, a `Maker` is run, which runs the event selection on all of them and produces a light ROOT file with histograms of relevant variables. Then, the `Analyse` is run which takes all these produced histograms and combines them to make the data/MC distributions. It also performs the cross section measurment and handles systematics.

## Build and Install
On uboonegpvm:
```
source /grid/fermiapp/products/uboone/setup_uboone.sh
setup uboonecode v06_26_01_12 -q e10:prof

git clone git@github.com:marcodeltutto/UBXSecAna.git
cd UBXSecAna
git checkout linux

bash setup.sh
cd Build
cmake -DCMAKE_CXX_COMPILER=/cvmfs/larsoft.opensciencegrid.org/products/gcc/v4_9_3a/Linux64bit+2.6-2.12/bin/g++ ..
make install
```

## Run
Scripts to run are in `UBXSecAna/Mac/*`.
```
cd UBXSecAna/Mac
source run_maker.sh  # Runs the Makes on the three input files downloaded above
source analyse.sh    # Runs the Analyse on the three lighter files produced by the Makes
```

## How to change the selection in order to make validation plots
To add or remove a cut, open Main/Maker.cxx. A list of cuts is around line 1460. Line 1460 is commented out and show an example of an additional cut. If uncommented, the selection will select only contained tracks.
Then recompile in the Build directory with just `make install`, and run again the Maker and the Analyse.
Variables are documented here: https://marcodeltutto.github.io/UBXSec/documentation/html/classUBXSecEvent.html
