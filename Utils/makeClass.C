void makeClass() {

  TFile *f = new TFile("../Files/ubxsec_output_mc_bnbcosmic_mcc8.7_test6.root");
  TTree *v = (TTree*)f->Get("UBXSec/tree");
  v->MakeClass("UBXSecEvent");
}
