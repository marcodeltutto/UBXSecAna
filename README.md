# UBXSecAna

## Build and Install
```
git clone git@github.com:marcodeltutto/UBXSecAna.git
cd UBXSecAna
source setup.sh
scp -r mdeltutt@uboonegpvm02.fnal.gov:/uboone/data/users/mdeltutt/ubxsec_static/mcc8.7_test/*test1.root Files/
cd Build
cmake ..
make install
```

## Run
Scripts to run are in `UBXSecAna/Mac/*`.
```
cd UBXSecAna/Mac
source run_maker.sh
source analyse.sh
```
