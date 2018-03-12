#include "UBXSecAnaConfig.h" // From the build dir (cmake generated)

#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <fstream>
#include <math.h>


#include <TRandom1.h>
#include <TSystem.h>
#include <TBrowser.h>
#include <TApplication.h>
#include <TChain.h>
#include "TThread.h"
#include "THStack.h"
#include "TLegend.h"
#include "TEfficiency.h"
#include "TGraphAsymmErrors.h"
#include "TGraph.h"
#include <TString.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TH1D.h>

#include "AnaTree.h"
#include "Spectrum.hpp"
#include "Spectrum2D.hpp"
#include "PlottingTools.h"

const bool _breakdownPlots = true;
const bool _makePlots = false;

double _beamSpillStarts = 3.2;  // us
double _beamSpillEnds   = 4.8;  // us
double _flashShift      = 0.;//4.06; //us
double _gainCalib       = 198; // e-/ADC

const double _pe_cut = 10;

const double targetPOT = 4.95e19;

using namespace std;

//____________________________________________________________________________________________________
void DrawProgressBar(double progress, double barWidth) {
  
  std::cout << "[";
  int pos = barWidth * progress;
  for (int i = 0; i < barWidth; ++i) {
    if (i < pos) std::cout << "=";
    else if (i == pos) std::cout << ">";
    else std::cout << " ";
  }
  std::cout << "] " << int(progress * 100.0) << " %\r";
  std::cout.flush();
}

//____________________________________________________________________________________________________
void DrawPOT2(double pot, double target = 6.6e20)
{
  //std::string str = "Simulated POT:" + std::to_string(pot);
  
  std::stringstream sstm;
  sstm << "Simulated POT: " << pot;
  std::string str = sstm.str();
  
  TLatex* pot_latex = new TLatex(.10, .96, str.c_str());
  pot_latex->SetTextColor(kGray+2);
  pot_latex->SetNDC();
  pot_latex->SetTextSize(1/30.);
  pot_latex->SetTextAlign(10); //left adjusted
  pot_latex->Draw();
  
  
  std::stringstream sstm2;
  sstm2 << "Scaled to POT: " << target;
  str = sstm2.str();
  
  TLatex* pot_latex_2 = new TLatex(.10, .92, str.c_str());
  pot_latex_2->SetTextFont(62);
  pot_latex_2->SetTextColor(kGray+2);
  pot_latex_2->SetNDC();
  pot_latex_2->SetTextSize(1/30.);
  pot_latex_2->SetTextAlign(10);//left adjusted
  pot_latex_2->Draw();
  
}

//____________________________________________________________________________________________________
void ActivateBranches(AnaTree *at) {
  
  //t->fChain->SetBranchStatus("",1);
}


//____________________________________________________________________________________________________
double eff_uncertainty(int _n, int _N) {

  double n = (double) _n;
  double N = (double) _N;

  double unc = 1/std::sqrt(N) * std::sqrt((n/N)*(1-n/N));

  return unc;

} 



//____________________________________________________________________________________________________
//____________________________________________________________________________________________________
//____________________________________________________________________________________________________
int main(int argc, char* argv[]) {
  
  clock_t begin = clock();

  fprintf(stdout, "%s Version %d.%d\n", argv[0], UBXSecAna_VERSION_MAJOR,
            UBXSecAna_VERSION_MINOR);
  
  std::string filen     = "ubxsec_output.root";
  bool evalPOT = false;
  int maxEntries = -1;
  
  std::ofstream _csvfile;
  _csvfile.open ("dqdx_trklen.csv", std::ofstream::out | std::ofstream::trunc);
  _csvfile << "dqdx,trklen,y" << std::endl;
  
  //*************************
  //* Getting input parameters
  //*************************
  
  int c;
  //int digit_optind = 0;
  
  while (1) {
    //int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
      {"filename",    required_argument, 0,  'f' },
      {"maxentries",  required_argument, 0,  'e' },
      {"flashstart",  required_argument, 0,  'a' },
      {"flashend",    required_argument, 0,  'b' },
      {"flashshift",  required_argument, 0,  's' },
      {"gaincalib",   required_argument, 0,  'g' },
      {0,             0,                 0,   0  }
    };
    
    c = getopt_long(argc, argv, "f:e:a:b:s:g:p",
                    long_options, &option_index);
    if (c == -1)
      break;
    
    switch (c) {
      case 0:
        std::cout << "Option " << long_options[option_index].name;
        if (optarg)
          std::cout << " with arg " << optarg << std::endl;
        break;
        
      case 'f':
        std::cout << "Input file: " << optarg << std::endl;
        filen = optarg;
        break;
        
      case 'e':
        std::cout << "Run over entries: " << optarg << std::endl;
        maxEntries = atof(optarg);
        break;
        
      case 'a':
        std::cout << "Beam spill start time: " << optarg << std::endl;
        _beamSpillStarts = atof(optarg);
        break;
        
      case 'b':
        std::cout << "Beam spill end time: " << optarg << std::endl;
        _beamSpillEnds = atof(optarg);
        break;
        
      case 's':
        std::cout << "Flash shift: " << optarg << std::endl;
        _flashShift = atof(optarg);
        break;
        
      case 'g':
        std::cout << "Gain Calibration: " << optarg << std::endl;
        _gainCalib = atof(optarg);
        break;

      case 'p':
        std::cout << "Calculating POT." << std::endl;
        evalPOT = true;
        break;
        
      case '?':
        break;
        
      default:
        printf("?? getopt returned character code 0%o ??\n", c);
    }
  }
  
  if (optind < argc) {
    printf("non-option ARGV-elements: ");
    while (optind < argc)
      printf("%s ", argv[optind++]);
    printf("\n");
  }
  
  std::cout << "_beamSpillStarts is " << _beamSpillStarts << std::endl;
  std::cout << "_beamSpillEnds is   " << _beamSpillEnds << std::endl;

  
  

  //*************************
  //* Starting ROOT application
  //*************************

  std::string env = std::getenv("UBXSecAnaRoot");
  
  //TApplication* rootapp = new TApplication("ROOT Application",&argc, argv);
  gROOT->SetBatch(kTRUE);
  gROOT->ProcessLine("gErrorIgnoreLevel = 2001;"); // 1001: INFO, 2001: WARNINGS, 3001: ERRORS
  gROOT->ProcessLine(".x ~/rootlogon.C");

  std::string library = ".L " + env + "loader_C.so";
  gROOT->ProcessLine(library.c_str());
  //gROOT->ProcessLine(".L ../../Root/loader_C.so");
  
  std::cout << "Opening output file ubxsecana_output.root." << std::endl;
  TFile *file_out = new TFile("ubxsecana_output.root","RECREATE");
  if ( file_out->IsOpen() )
    std::cout << "File opened successfully" << std::endl;
  
  string pattern = filen;
  
  
  
  //*************************
  //* Getting POTs
  //*************************
  
  double totalPOT = 0.;
  
  if (maxEntries > 0) evalPOT = false;
  
  if (evalPOT) {
    
    cout << " ----- " << endl;
    cout << "| Calculating POT" << endl;
    cout << "| " << endl;
    
    TChain *cpot;
    cpot = new TChain("UBXSec/pottree");
    cpot->Add(pattern.c_str());
    cout << "| Number of entries in the pot tree: " << cpot->GetEntries() << endl;
    Double_t pot;
    cpot->SetBranchAddress("pot", &pot);
    for (int potEntry = 0; potEntry < cpot->GetEntries(); potEntry++) {
      cpot->GetEntry(potEntry);
      totalPOT += pot;
    } // end loop entries
    cout << "| Total POT: " << totalPOT << endl;
    cout << " ----- " << endl << endl;
  } // end if evalPOT
  else
    totalPOT = -1.;
  
  double pot_scaling = 1.;
  if (evalPOT) pot_scaling = targetPOT/totalPOT;
  
  
  TChain *chain_ubxsec;
  chain_ubxsec = new TChain("UBXSec/tree");
  chain_ubxsec->Add(pattern.c_str());
  
  cout << "Using file: " << pattern << endl;
  
  int Nfiles = chain_ubxsec->GetNtrees();
  cout << "Number of files: " << Nfiles << endl;
  
  int evts = chain_ubxsec -> GetEntries();
  cout << "Number of events used is: " << evts << endl;
  
  AnaTree * t = new AnaTree(chain_ubxsec);
  ActivateBranches(t);
  

  
  int nsignal = 0;

  int nsignal_qe = 0;
  int nsignal_res = 0;
  int nsignal_dis = 0;
  int nsignal_coh = 0;
  int nsignal_mec = 0;
  
  int signal_sel = 0;
  int bkg_anumu_sel = 0;
  int bkg_nue_sel = 0;
  int bkg_nc_sel = 0;
  int bkg_outfv_sel = 0;
  int bkg_cosmic_sel = 0;
  int bkg_cosmic_top_sel = 0;

  int signal_sel_qe = 0;
  int signal_sel_res = 0;
  int signal_sel_dis = 0;
  int signal_sel_coh = 0;
  int signal_sel_mec = 0;
  
  int nEvtsWFlashInBeamSpill = 0;
  int nNumuCC = 0;
  
  double nue_cc_fv = 0;
  double nue_cc_selected = 0;
  double nue_cc_selected_total = 0;
  double nue_cc_selected_total_energy_range = 0;
  double nue_selected_total_energy_range = 0;
  double n_nue_electron = 0;
  double n_nue_proton = 0;
  double n_nue_pion = 0;

  int nSignalWMuonReco = 0;
  int nSignalMuonRecoVtxOk = 0;
  
  int nSignalFlashMatched = 0;
  
  int n_slc_nu_origin = 0;
  int n_slc_acpt_tag_nu = 0;

  std::map<std::string, double> selected_events_percut;
  selected_events_percut["initial"] = 0.;
  selected_events_percut["beamflash"] = 0.;
  selected_events_percut["flash_match"] = 0.;
  selected_events_percut["flash_match_deltax"] = 0.;
  selected_events_percut["flash_match_deltaz"] = 0.;
  selected_events_percut["fiducial_volume"] = 0.;
  selected_events_percut["quality"] = 0.;
  selected_events_percut["mcs_length_quality"] = 0.;
  selected_events_percut["mip_consistency"] = 0.;

  std::map<std::string, double> selected_signal_events_percut;
  selected_signal_events_percut["initial"] = 0.;
  selected_signal_events_percut["beamflash"] = 0.;
  selected_signal_events_percut["flash_match"] = 0.;
  selected_signal_events_percut["flash_match_deltax"] = 0.;
  selected_signal_events_percut["flash_match_deltaz"] = 0.;
  selected_signal_events_percut["fiducial_volume"] = 0.;
  selected_signal_events_percut["quality"] = 0.;
  selected_signal_events_percut["mcs_length_quality"] = 0.;
  selected_signal_events_percut["mip_consistency"] = 0.;

  
  TTree* _shower_tree = new TTree("shower_tree", "shower_tree");
  double _s_nupdg, _s_track_pdg, _s_tpcobj_origin, _s_shower_length, _s_shower_phi, _s_shower_theta, _s_shower_openangle, _s_shower_startx, _s_shower_starty, _s_shower_startz, _s_flash_z;
  _shower_tree->Branch("s_nupdg", &_s_nupdg, "s_nupdg/D");
  _shower_tree->Branch("s_track_pdg", &_s_track_pdg, "s_track_pdg/D");
  _shower_tree->Branch("s_tpcobj_origin", &_s_tpcobj_origin, "s_tpcobj_origin/D");
  _shower_tree->Branch("s_shower_length", &_s_shower_length, "s_shower_length/D");
  _shower_tree->Branch("s_shower_phi", &_s_shower_phi, "s_shower_phi/D");
  _shower_tree->Branch("s_shower_theta", &_s_shower_theta, "s_shower_theta/D");
  _shower_tree->Branch("s_shower_openangle", &_s_shower_openangle, "s_shower_openangle/D");
  _shower_tree->Branch("s_shower_startx", &_s_shower_startx, "s_shower_startx/D");
  _shower_tree->Branch("s_shower_starty", &_s_shower_starty, "s_shower_starty/D");
  _shower_tree->Branch("s_shower_startz", &_s_shower_startz, "s_shower_startz/D");
  _shower_tree->Branch("s_flash_z", &_s_flash_z, "s_flash_z/D");

  TTree* _mom_tree = new TTree("mom_tree", "mom_tree");
  double _mom_tree_true, _mom_tree_mcs;
  bool _mom_tree_contained, _mom_tree_selected;
  double _mom_tree_angle_true, _mom_tree_angle_reco;
  _mom_tree->Branch("mom_tree_true", &_mom_tree_true, "mom_tree_true/D");
  _mom_tree->Branch("mom_tree_mcs", &_mom_tree_mcs, "mom_tree_mcs/D");
  _mom_tree->Branch("mom_tree_contained", &_mom_tree_contained, "mom_tree_contained/O");
  _mom_tree->Branch("mom_tree_selected", &_mom_tree_selected, "mom_tree_selected/O");
  _mom_tree->Branch("mom_tree_angle_true", &_mom_tree_angle_true, "mom_tree_angle_true/D");
  _mom_tree->Branch("mom_tree_angle_reco", &_mom_tree_angle_reco, "mom_tree_angle_reco/D");


  double bins_mumom[7] = {0.00, 0.18, 0.30, 0.45, 0.77, 1.28, 2.50};

  TH2D * h_true_reco_mom= new TH2D("h_true_reco_mom", ";Muon Momentum (Truth) [GeV]; Muon Momentum (MCS) [GeV]", 6, bins_mumom, 6, bins_mumom);

  double bins_mucostheta[10] = {-1.00, -0.50, 0.00, 0.27, 0.45, 0.62, 0.76, 0.86, 0.94, 1.00};

  TH2D * h_true_reco_costheta= new TH2D("h_true_reco_costheta", ";Muon cos(#theta) (Truth) [GeV]; Muon cos(#theta) (MCS) [GeV]", 9, bins_mucostheta, 9, bins_mucostheta);

  int n_bins_mumom_temp = 4;
  double bins_mumom_temp[5] = {0.00, 0.25, 0.50, 1.0, 2.50};
  int n_bins_mucostheta_temp = 6;
  double bins_mucostheta_temp[7] = {-1.00, -0.50, 0.00, 0.25, 0.50, 0.75, 1.00};
  
  TH1D* h_eff_num = new TH1D("h_eff_num", "h_eff_num", 15, 0, 3);
  TH1D* h_eff_den = new TH1D("h_eff_den", "h_eff_den", 15, 0, 3);
  TEfficiency* pEff = new TEfficiency("eff",";Neutrino Energy (truth) [GeV];Efficiency",6, 0, 4);
  TH1D* h_eff_mumom_num = new TH1D("h_eff_mumom_num", "h_eff_mumom_num", 6, bins_mumom);
  TH1D* h_eff_mumom_den = new TH1D("h_eff_mumom_den", "h_eff_mumom_den", 6, bins_mumom);
  TH1D* h_eff_muangle_num = new TH1D("h_eff_muangle_num", "h_eff_muangle_num", 9, bins_mucostheta);
  TH1D* h_eff_muangle_den = new TH1D("h_eff_muangle_den", "h_eff_muangle_den", 9, bins_mucostheta);
  TH2D* h_eff_muangle_mumom_num = new TH2D("h_eff_muangle_mumom_num", "h_eff_muangle_mumom_num", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  TH2D* h_eff_muangle_mumom_den = new TH2D("h_eff_muangle_mumom_den", "h_eff_muangle_mumom_den", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);

  TH1D* h_eff_mult_num = new TH1D("h_eff_mult_num", "h_eff_mult_num", 20, 0, 20);
  TH1D* h_eff_mult_den = new TH1D("h_eff_mult_den", "h_eff_mult_den", 20, 0, 20);
  TH1D* h_eff_mult_ch_num = new TH1D("h_eff_mult_ch_num", "h_eff_mult_ch_num", 10, 0, 15);
  TH1D* h_eff_mult_ch_den = new TH1D("h_eff_mult_ch_den", "h_eff_mult_ch_den", 10, 0, 15);
  TH1D* h_eff_muphi_num = new TH1D("h_eff_muphi_num", "h_eff_muphi_num", 15, -3.1415, 3.1415);
  TH1D* h_eff_muphi_den = new TH1D("h_eff_muphi_den", "h_eff_muphi_den", 15, -3.1415, 3.1415);

  TH1D* h_eff_qe_num = new TH1D("h_eff_qe_num", "h_eff_qe_num", 15, 0, 3);
  TH1D* h_eff_qe_den = new TH1D("h_eff_qe_den", "h_eff_qe_den", 15, 0, 3);
  TH1D* h_eff_res_num = new TH1D("h_eff_res_num", "h_eff_res_num", 15, 0, 3);
  TH1D* h_eff_res_den = new TH1D("h_eff_res_den", "h_eff_res_den", 15, 0, 3);
  TH1D* h_eff_dis_num = new TH1D("h_eff_dis_num", "h_eff_dis_num", 15, 0, 3);
  TH1D* h_eff_dis_den = new TH1D("h_eff_dis_den", "h_eff_dis_den", 15, 0, 3);
  TH1D* h_eff_coh_num = new TH1D("h_eff_coh_num", "h_eff_coh_num", 15, 0, 3);
  TH1D* h_eff_coh_den = new TH1D("h_eff_coh_den", "h_eff_coh_den", 15, 0, 3);
  TH1D* h_eff_mec_num = new TH1D("h_eff_mec_num", "h_eff_mec_num", 15, 0, 3);
  TH1D* h_eff_mec_den = new TH1D("h_eff_mec_den", "h_eff_mec_den", 15, 0, 3);

  TH1D* h_truth_xsec_mumom = new TH1D("h_truth_xsec_mumom", "h_truth_xsec_mumom", 6, bins_mumom);
  TH1D* h_truth_xsec_muangle = new TH1D("h_truth_xsec_muangle", "h_truth_xsec_muangle", 9, bins_mucostheta);

  TH1D* h_nue_selected_energy = new TH1D("h_nue_selected_energy", ";True Neutrino Energy [GeV];#nu_{e} Selected Events", 100, 0, 1.5);


  TH1D* h_true_nu_eng_beforesel = new TH1D("h_true_nu_eng_beforesel", ";True Neutrino Energy [GeV];Events", 200, 0, 3);
  TH1D* h_true_nu_eng_afterflash = new TH1D("h_true_nu_eng_afterflash", ";True Neutrino Energy [GeV];Events", 200, 0, 3);
  TH1D* h_true_nu_eng_aftersel = new TH1D("h_true_nu_eng_aftersel", ";True Neutrino Energy [GeV];Events", 200, 0, 3);

  
  
  TH1D* h_chi2 = new TH1D("h_chi2", "h_chi2", 50, 0, 50);
  TH1D* h_flsTime = new TH1D("h_flsTime", ";Flash time w.r.t. trigger [#mus];Flashes", 125, 0, 25);
  TH1D* h_flsTime_wcut = new TH1D("h_flsTime_wcut", ";Flash time w.r.t. trigger [#mus];Flashes (> 50PE)", 125, 0, 25);
  TH1D* h_flsTime_wcut_2 = new TH1D("h_flsTime_wcut_2", "(2);Flash time w.r.t. trigger [#mus];Flashes (> 50PE)", 125, 0, 25);
  TH1D* h_flsTime_wcut_3 = new TH1D("h_flsTime_wcut_3", "(3);Flash time w.r.t. trigger [#mus];Flashes (> 50PE)", 125, 0, 25);
  TH1D* h_flsTime_wcut_4 = new TH1D("h_flsTime_wcut_4", "(4);Flash time w.r.t. trigger [#mus];Flashes (> 50PE)", 125, 0, 25);
  TH1D* h_flsTime_wcut_5 = new TH1D("h_flsTime_wcut_5", "(5);Flash time w.r.t. trigger [#mus];Flashes (> 50PE)", 125, 0, 25);
  TH1D* h_flsTime_wcut_6 = new TH1D("h_flsTime_wcut_6", "(6);Flash time w.r.t. trigger [#mus];Flashes (> 50PE)", 125, 0, 25);
  TH1D* h_flsTime_wcut_7 = new TH1D("h_flsTime_wcut_7", "(7);Flash time w.r.t. trigger [#mus];Flashes (> 50PE)", 125, 0, 25);
  TH1D* h_flsTime_wcut_8 = new TH1D("h_flsTime_wcut_8", "(8);Flash time w.r.t. trigger [#mus];Flashes (> 50PE)", 125, 0, 25);
  h_flsTime->Sumw2(); h_flsTime_wcut->Sumw2(); h_flsTime_wcut_2->Sumw2(); h_flsTime_wcut_3->Sumw2(); h_flsTime_wcut_4->Sumw2(); h_flsTime_wcut_5->Sumw2(); h_flsTime_wcut_6->Sumw2(); h_flsTime_wcut_7->Sumw2(); h_flsTime_wcut_8->Sumw2();

  TH1D* h_flsPe_wcut = new TH1D("h_flsPe_wcut", ";Flash PE;Flashes (> 50PE)", 700, 0, 50000);
  TH2D* h_flsTime_flsPe_wcut = new TH2D("h_flsTime_flsPe_wcut", "Flashes (> 50PE);Flash time w.r.t. trigger [#mus];Flash PE", 125, 0, 25, 700, 0, 50000);
  
  TH1D* h_deltax = new TH1D("h_deltax", "(4);QLL X - TPC X [cm];;", 500, -200,200);
  TH2D* h_deltax_2d = new TH2D("h_deltax_2d", "(4);QLL X [cm];TPC X [cm]", 70, -100,350, 70, -100,350);
  TH1D* h_deltaz_4 = new TH1D("h_deltaz_4", "(4);Delta z [cm];", 100, -200,200);
  TH1D* h_deltaz_6 = new TH1D("h_deltaz_6", "(6);Delta z [cm];", 100, -200,200);

  TH1D* h_nslices = new TH1D("h_nslices", ";Number of slices per event;Entries per bin", 15, 0, 15);
  TH1D* h_vtx_resolution = new TH1D("h_nslh_vtx_resolutionices", ";Vertex resolution (2D) [cm];Entries per bin", 300, 0, 500);
  
  TH2D* h_frac_diff = new TH2D("h_frac_diff", ";PMT ID; Fractional difference", 32, 0, 32, 80, -2, 2);
  TH2D* h_frac_diff_others = new TH2D("h_frac_diff", ";PMT ID; Fractional difference", 32, 0, 32, 80, -2, 2);
  double hypo_spec_x[32], hypo_spec_y[32];
  double meas_spec_x[32], meas_spec_y[32];
  double numc_spec_x[32], numc_spec_y[32];
  
  TH1D* h_xdiff = new TH1D("h_xdiff", "h_xdiff", 1000, -100,100);
  TH1D* h_xdiff_others = new TH1D("h_xdiff_others", "h_xdiff_others", 1000, -100,100);
  TH1D* h_zdiff = new TH1D("h_zdiff", "h_zdiff", 1000, 0,1000);
  TH1D* h_zdiff_others = new TH1D("h_zdiff_others", "h_zdiff_others", 1000, 0,1000);
  // Before selection
  std::map<std::string,TH1D*> hmap_xdiff_b;
  hmap_xdiff_b["total"] = new TH1D("h_xdiff_total_b", ";QLL x - TPC x [cm];", 80, -200,200);//  100, 0,22
  hmap_xdiff_b["signal"] = new TH1D("h_xdiff_signal_b", ";QLL x - TPC x [cm];", 80, -200,200);
  hmap_xdiff_b["background"] = new TH1D("h_xdiff_background_b", ";QLL x - TPC x [cm];", 80, -200,200);
  std::map<std::string,TH1D*> hmap_zdiff_b;
  hmap_zdiff_b["total"] = new TH1D("h_zdiff_total_b", ";Hypo z - Flash z [cm];", 160, -400,400);
  hmap_zdiff_b["signal"] = new TH1D("h_zdiff_signal_b", ";Hypo z - Flash z [cm];", 160, -400,400);
  hmap_zdiff_b["background"] = new TH1D("h_zdiff_background_b", ";Hypo z - Flash z [cm];", 160, -400,400);
  // After selection
  std::map<std::string,TH1D*> hmap_xdiff;
  hmap_xdiff["total"] = new TH1D("h_xdiff_total", ";QLL x - TPC x [cm];", 80, -200,200);
  hmap_xdiff["signal"] = new TH1D("h_xdiff_signal", ";QLL x - TPC x [cm];", 80, -200,200);
  hmap_xdiff["background"] = new TH1D("h_xdiff_background", ";QLL x - TPC x [cm];", 80, -200,200);
  std::map<std::string,TH1D*> hmap_zdiff;
  hmap_zdiff["total"] = new TH1D("h_zdiff_total", ";Hypo z - Flash z [cm];", 160, -400,400);
  hmap_zdiff["signal"] = new TH1D("h_zdiff_signal", ";Hypo z - Flash z [cm];", 160, -400,400);
  hmap_zdiff["background"] = new TH1D("h_zdiff_background", ";Hypo z - Flash z [cm];", 160, -400,400);
  std::map<std::string,TH1D*> hmap_pediff;
  hmap_pediff["total"] = new TH1D("h_pediff_total", ";Hypo PE - Flash PE [p.e.];", 160, -400,400);
  hmap_pediff["signal"] = new TH1D("h_pediff_signal", ";Hypo PE - Flash PE [p.e.];", 160, -400,400);
  hmap_pediff["background"] = new TH1D("h_pediff_background", ";Hypo PE - Flash PE [p.e.];", 160, -600,600);

  std::map<std::string,TH1D*> hmap_vtxcheck_angle;
  hmap_vtxcheck_angle["total"] = new TH1D("h_vtxcheck_angle_total", ";Angle [rad];Entries per bin", 80, 0, 4);
  hmap_vtxcheck_angle["signal"] = new TH1D("h_vtxcheck_angle_signal", ";Angle [rad];Entries per bin", 80, 0, 4);
  hmap_vtxcheck_angle["background"] = new TH1D("h_vtxcheck_angle_background", ";Angle [rad];Entries per bin", 80, 0, 4);
  
  TH1D* h_vtxcheck_angle_good = new TH1D("h_vtxcheck_angle_good", ";Angle [rad];Entries per bin", 100, 0, 4);
  TH1D* h_vtxcheck_angle_bad  = new TH1D("h_vtxcheck_angle_bad",  ";Angle [rad];Entries per bin",  100, 0, 4);
  
  std::map<std::string,TH1D*> hmap_residuals_std;
  hmap_residuals_std["total"] = new TH1D("h_residuals_std_total", ";#sigma_{r_{i}};Entries per bin", 40, 0, 10);
  hmap_residuals_std["signal"] = new TH1D("h_residuals_std_signal", ";Angle [rad];Entries per bin", 40, 0, 10);
  hmap_residuals_std["background"] = new TH1D("h_residuals_std_background", ";Angle [rad];Entries per bin", 40, 0, 10);
  std::map<std::string,TH1D*> hmap_residuals_mean;
  hmap_residuals_mean["total"] = new TH1D("h_residuals_mean_total", ";<r_{i}>;Entries per bin", 40, -5, 5);
  hmap_residuals_mean["signal"] = new TH1D("h_residuals_mean_signal", ";<r_{i}>;Entries per bin", 40, -5, 5);
  hmap_residuals_mean["background"] = new TH1D("h_residuals_mean_background", ";<r_{i}>;Entries per bin", 40, -5, 5);
  std::map<std::string,TH1D*> hmap_perc_used_hits;
  hmap_perc_used_hits["total"] = new TH1D("h_perc_used_hits_total", ";Fraction of used hits in cluster;Entries per bin", 30, 0, 1);
  hmap_perc_used_hits["signal"] = new TH1D("h_perc_used_hits_signal", ";Fraction of used hits in cluster;Entries per bin", 30, 0, 1);
  hmap_perc_used_hits["background"] = new TH1D("h_perc_used_hits_background", ";Fraction of used hits in cluster;Entries per bin", 30, 0, 1);

  std::map<std::string,TH1D*> hmap_mom_mcs_length;
  hmap_mom_mcs_length["total"] = new TH1D("h_mom_mcs_length_total", ";(MCS - Length) Reconstructed Momentum [GeV];Entries per bin", 20, -0.5, 1.5);
  hmap_mom_mcs_length["signal"] = new TH1D("h_mom_mcs_length_signal", ";(MCS - Length) Reconstructed Momentum [GeV];Entries per bin", 20, -0.5, 1.5);
  hmap_mom_mcs_length["background"] = new TH1D("h_mom_mcs_length_background", ";(MCS - Length) Reconstructed Momentum [GeV];Entries per bin", 20, -0.5, 1.5);

  TH1D* h_muon_track_eff  = new TH1D("h_muon_track_eff",  ";Muon track efficiency;Entries per bin",  100, 0, 1);
  TH1D* h_muon_track_pur  = new TH1D("h_muon_track_pur",  ";Muon track purity;Entries per bin",  100, 0, 1);
  
  TH1D* h_mueff_num = new TH1D("h_mueff_num", "h_mueff_num", 30, 0, 2);
  TH1D* h_mueff_2_num = new TH1D("h_mueff_2_num", "h_mueff_2_num", 30, 0, 2);
  TH1D* h_mueff_den = new TH1D("h_mueff_den", "h_mueff_den", 30, 0, 2);
  TH1D* h_mueff_angle_num = new TH1D("h_mueff_angle_num", "h_mueff_num", 15, -1, 1);
  TH1D* h_mueff_angle_den = new TH1D("h_mueff_angle_den", "h_mueff_den", 15, -1, 1);

  TH2D* h_mu_eff_mom = new TH2D("h_mu_eff_mom", ";True Muon Momentum [GeV]; Efficiency", 50, 0, 2, 20, 0, 1);
  TH2D* h_mu_pur_mom = new TH2D("h_mu_pur_mom", ";True Muon Momentum [GeV]; Purity", 50, 0, 2, 20, 0, 1);
  TH2D* h_mu_eff_mom_sel = new TH2D("h_mu_eff_mom_sel", "After Selection;True Muon Momentum [GeV]; Efficiency", 50, 0, 2, 20, 0, 1);
  
  TH2D* h_mumom_nue = new TH2D("h_mumom_nue", ";True Neutrino Energy [GeV]; True Muon Momentum [GeV]", 50, 0, 2, 50, 0, 4);
  
  TH1D* h_acpt_tagged  = new TH1D("h_acpt_tagged",  ";Tagged TPC Objects;Entries per bin",  10, 0, 10);
  
  TH1D* h_slice_origin = new TH1D("h_slice_origin",  ";;",  3, -0.5, 2.5);
  
  TH1D* h_slice_npfp = new TH1D("h_slice_npfp",  ";npfp;",  10, 0, 10);
  TH1D* h_slice_npfp_others = new TH1D("h_slice_npfp_others",  ";npfp;",  10, 0, 10);
  
  TH1D* h_slice_ntrack = new TH1D("h_slice_ntrack",  ";npfp;",  10, 0, 10);
  TH1D* h_slice_ntrack_others = new TH1D("h_slice_ntrack_others",  ";npfp;",  10, 0, 10);
  
  TH1D* h_fm_score = new TH1D("h_fm_score",  ";fm score;",  500, 0, 10);
  TH1D* h_fm_score_others = new TH1D("h_fm_score_other",  ";fm score;",  500, 0, 10);
  TH2D* h_fm_score_pe = new TH2D("h_fm_score_pe",  ";fm score;Reco PE",  500, 0, 10, 500, 0, 2000);
  
  TH1D* h_n_slc_flsmatch = new TH1D("h_n_slc_flsmatch",  ";n slices flash matched per event;",  10, 0, 10);

  std::map<std::string,TH2D*> hmap_trktheta_trkmom;
  hmap_trktheta_trkmom["total"] = new TH2D("h_trktheta_trkmom_total", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["signal"] = new TH2D("h_trktheta_trkmom_signal", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["cosmic"] = new TH2D("h_trktheta_trkmom_cosmic", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["outfv"] = new TH2D("h_trktheta_trkmom_outfv", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["nc"] = new TH2D("h_trktheta_trkmom_nc", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["anumu"] = new TH2D("h_trktheta_trkmom_anumu", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["nue"] = new TH2D("h_trktheta_trkmom_nue", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["cosmic_stopmu"] = new TH2D("h_trktheta_trkmom_cosmic_stopmu", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["cosmic_nostopmu"] = new TH2D("h_trktheta_trkmom_cosmic_nostopmu", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["outfv_stopmu"] = new TH2D("h_trktheta_trkmom_outfv_stopmu", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["outfv_nostopmu"] = new TH2D("h_trktheta_trkmom_outfv_nostopmu", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["nc_proton"] = new TH2D("h_trktheta_trkmom_nc_proton", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["nc_pion"] = new TH2D("h_trktheta_trkmom_nc_pion", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["nc_other"] = new TH2D("h_trktheta_trkmom_nc_other", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["signal_stopmu"] = new TH2D("h_trktheta_trkmom_signal_stopmu", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  hmap_trktheta_trkmom["signal_nostopmu"] = new TH2D("h_trktheta_trkmom_signal_nostopmu", "; Track cos(#theta);", n_bins_mucostheta_temp, bins_mucostheta_temp, n_bins_mumom_temp, bins_mumom_temp);
  
  std::map<std::string,TH1D*> hmap_trklen;
  hmap_trklen["total"] = new TH1D("h_trklen_total", "; Track length;", 30, 0, 700);
  hmap_trklen["signal"] = new TH1D("h_trklen_signal", "; Track length;", 30, 0, 700);
  hmap_trklen["cosmic"] = new TH1D("h_trklen_cosmic", "; Track length;", 30, 0, 700);
  hmap_trklen["cosmic_stopmu"] = new TH1D("h_trklen_cosmic_stopmu", "; Track length;", 30, 0, 700);
  hmap_trklen["cosmic_nostopmu"] = new TH1D("h_trklen_cosmic_nostopmu", "; Track length;", 30, 0, 700);
  hmap_trklen["outfv"] = new TH1D("h_trklen_outfv", "; Track length;", 30, 0, 700);
  hmap_trklen["outfv_stopmu"] = new TH1D("h_trklen_outfv_stopmu", "; Track length;", 30, 0, 700);
  hmap_trklen["outfv_nostopmu"] = new TH1D("h_trklen_outfv_nostopmu", "; Track length;", 30, 0, 700);
  hmap_trklen["nc"] = new TH1D("h_trklen_nc", "; Track length;", 30, 0, 700);
  hmap_trklen["nc_proton"] = new TH1D("h_trklen_nc_proton", "; Track length;", 30, 0, 700);
  hmap_trklen["nc_pion"] = new TH1D("h_trklen_nc_pion", "; Track length;", 30, 0, 700);
  hmap_trklen["nc_other"] = new TH1D("h_trklen_nc_other", "; Track length;", 30, 0, 700);
  hmap_trklen["anumu"] = new TH1D("h_trklen_anumu", "; Track length;", 30, 0, 700);
  hmap_trklen["nue"] = new TH1D("h_trklen_nue", "; Track length;", 30, 0, 700);
  hmap_trklen["signal_stopmu"] = new TH1D("h_trklen_signal_stopmu", "; Track length;", 30, 0, 700);
  hmap_trklen["signal_nostopmu"] = new TH1D("h_trklen_signal_nostopmu", "; Track length;", 30, 0, 700);
  
  std::map<std::string,TH1D*> hmap_trkmom;
  hmap_trkmom["total"] = new TH1D("h_trkmom_total", "; Track length;", 6, bins_mumom); // 20, 0, 2.5
  hmap_trkmom["signal"] = new TH1D("h_trkmom_signal", "; Track length;", 6, bins_mumom);
  hmap_trkmom["cosmic"] = new TH1D("h_trkmom_cosmic", "; Track length;", 6, bins_mumom);
  hmap_trkmom["cosmic_stopmu"] = new TH1D("h_trkmom_cosmic_stopmu", "; Track length;", 6, bins_mumom);
  hmap_trkmom["cosmic_nostopmu"] = new TH1D("h_trkmom_cosmic_nostopmu", "; Track length;", 6, bins_mumom);
  hmap_trkmom["outfv"] = new TH1D("h_trkmom_outfv", "; Track length;", 6, bins_mumom);
  hmap_trkmom["outfv_stopmu"] = new TH1D("h_trkmom_outfv_stopmu", "; Track length;", 6, bins_mumom);
  hmap_trkmom["outfv_nostopmu"] = new TH1D("h_trkmom_outfv_nostopmu", "; Track length;", 6, bins_mumom);
  hmap_trkmom["nc"] = new TH1D("h_trkmom_nc", "; Track length;", 6, bins_mumom);
  hmap_trkmom["nc_proton"] = new TH1D("h_trkmom_nc_proton", "; Track length;", 6, bins_mumom);
  hmap_trkmom["nc_pion"] = new TH1D("h_trkmom_nc_pion", "; Track length;", 6, bins_mumom);
  hmap_trkmom["nc_other"] = new TH1D("h_trkmom_nc_other", "; Track length;", 6, bins_mumom);
  hmap_trkmom["anumu"] = new TH1D("h_trkmom_anumu", "; Track length;", 6, bins_mumom);
  hmap_trkmom["nue"] = new TH1D("h_trkmom_nue", "; Track length;", 6, bins_mumom);
  hmap_trkmom["signal_stopmu"] = new TH1D("h_trkmom_signal_stopmu", "; Track length;", 6, bins_mumom);
  hmap_trkmom["signal_nostopmu"] = new TH1D("h_trkmom_signal_nostopmu", "; Track length;", 6, bins_mumom);
  
  std::map<std::string,TH1D*> hmap_trkphi;
  hmap_trkphi["total"] = new TH1D("h_trkphi_total", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["signal"] = new TH1D("h_trkphi_signal", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["cosmic"] = new TH1D("h_trkphi_cosmic", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["outfv"] = new TH1D("h_trkphi_outfv", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["nc"] = new TH1D("h_trkphi_nc", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["anumu"] = new TH1D("h_trkphi_anumu", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["nue"] = new TH1D("h_trkphi_nue", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["cosmic_stopmu"] = new TH1D("h_trkphi_cosmic_stopmu", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["cosmic_nostopmu"] = new TH1D("h_trkphi_cosmic_nostopmu", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["outfv_stopmu"] = new TH1D("h_trkphi_outfv_stopmu", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["outfv_nostopmu"] = new TH1D("h_trkphi_outfv_nostopmu", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["nc_proton"] = new TH1D("h_trkphi_nc_proton", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["nc_pion"] = new TH1D("h_trkphi_nc_pion", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["nc_other"] = new TH1D("h_trkphi_nc_other", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["signal_stopmu"] = new TH1D("h_trkphi_signal_stopmu", "; Track #phi;", 20, -3.15, 3.15);
  hmap_trkphi["signal_nostopmu"] = new TH1D("h_trkphi_signal_nostopmu", "; Track #phi;", 20, -3.15, 3.15);
  
  std::map<std::string,TH1D*> hmap_trktheta;
  hmap_trktheta["total"] = new TH1D("h_trktheta_total", "; Track cos(#theta);", 9, bins_mucostheta); // 30, -1, 1
  hmap_trktheta["signal"] = new TH1D("h_trktheta_signal", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["cosmic"] = new TH1D("h_trktheta_cosmic", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["outfv"] = new TH1D("h_trktheta_outfv", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["nc"] = new TH1D("h_trktheta_nc", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["anumu"] = new TH1D("h_trktheta_anumu", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["nue"] = new TH1D("h_trktheta_nue", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["cosmic_stopmu"] = new TH1D("h_trktheta_cosmic_stopmu", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["cosmic_nostopmu"] = new TH1D("h_trktheta_cosmic_nostopmu", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["outfv_stopmu"] = new TH1D("h_trktheta_outfv_stopmu", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["outfv_nostopmu"] = new TH1D("h_trktheta_outfv_nostopmu", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["nc_proton"] = new TH1D("h_trktheta_nc_proton", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["nc_pion"] = new TH1D("h_trktheta_nc_pion", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["nc_other"] = new TH1D("h_trktheta_nc_other", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["signal_stopmu"] = new TH1D("h_trktheta_signal_stopmu", "; Track cos(#theta);", 9, bins_mucostheta);
  hmap_trktheta["signal_nostopmu"] = new TH1D("h_trktheta_signal_nostopmu", "; Track cos(#theta);", 9, bins_mucostheta);

  std::map<std::string,TH1D*> hmap_multpfp;
  hmap_multpfp["total"] = new TH1D("h_multpfp_total", "; PFP Multiplicity", 10, 0, 10);
  hmap_multpfp["signal"] = new TH1D("h_multpfp_signal", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["cosmic"] = new TH1D("h_multpfp_cosmic", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["outfv"] = new TH1D("h_multpfp_outfv", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["nc"] = new TH1D("h_multpfp_nc", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["anumu"] = new TH1D("h_multpfp_anumu", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["nue"] = new TH1D("h_multpfp_nue", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["cosmic_stopmu"] = new TH1D("h_multpfp_cosmic_stopmu", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["cosmic_nostopmu"] = new TH1D("h_multpfp_cosmic_nostopmu", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["outfv_stopmu"] = new TH1D("h_multpfp_outfv_stopmu", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["outfv_nostopmu"] = new TH1D("h_multpfp_outfv_nostopmu", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["nc_proton"] = new TH1D("h_multpfp_nc_proton", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["nc_pion"] = new TH1D("h_multpfp_nc_pion", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["nc_other"] = new TH1D("h_multpfp_nc_other", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["signal_stopmu"] = new TH1D("h_multpfp_signal_stopmu", "; PFP Multiplicity;", 10, 0, 10);
  hmap_multpfp["signal_nostopmu"] = new TH1D("h_multpfp_signal_nostopmu", "; PFP Multiplicity;", 10, 0, 10);

  std::map<std::string,TH1D*> hmap_multtracktol;
  hmap_multtracktol["total"] = new TH1D("h_multtracktol_total", "; Track Multiplicity (5 cm)", 10, 0, 10);
  hmap_multtracktol["signal"] = new TH1D("h_multtracktol_signal", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["cosmic"] = new TH1D("h_multtracktol_cosmic", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["outfv"] = new TH1D("h_multtracktol_outfv", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["nc"] = new TH1D("h_multtracktol_nc", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["anumu"] = new TH1D("h_multtracktol_anumu", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["nue"] = new TH1D("h_multtracktol_nue", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["cosmic_stopmu"] = new TH1D("h_multtracktol_cosmic_stopmu", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["cosmic_nostopmu"] = new TH1D("h_multtracktol_cosmic_nostopmu", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["outfv_stopmu"] = new TH1D("h_multtracktol_outfv_stopmu", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["outfv_nostopmu"] = new TH1D("h_multtracktol_outfv_nostopmu", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["nc_proton"] = new TH1D("h_multtracktol_nc_proton", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["nc_pion"] = new TH1D("h_multtracktol_nc_pion", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["nc_other"] = new TH1D("h_multtracktol_nc_other", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["signal_stopmu"] = new TH1D("h_multtracktol_signal_stopmu", "; Track Multiplicity (5 cm);", 10, 0, 10);
  hmap_multtracktol["signal_nostopmu"] = new TH1D("h_multtracktol_signal_nostopmu", "; Track Multiplicity (5 cm);", 10, 0, 10);
  
  std::map<std::string,TH1D*> hmap_dqdx_trunc;
  hmap_dqdx_trunc["total"] = new TH1D("h_dqdx_trunc_total", ";<dQ/dx>_{trunc};", 40, 0, 200000);//40, 0, 800);
  hmap_dqdx_trunc["muon"] = new TH1D("h_dqdx_trunc_muon", ";<dQ/dx>_{trunc};", 40, 0, 200000);
  hmap_dqdx_trunc["proton"] = new TH1D("h_dqdx_trunc_proton", ";<dQ/dx>_{trunc};", 40, 0, 200000);
  hmap_dqdx_trunc["pion"] = new TH1D("h_dqdx_trunc_pions", ";<dQ/dx>_{trunc};", 40, 0, 200000);
  hmap_dqdx_trunc["photon"] = new TH1D("h_dqdx_trunc_photon", ";<dQ/dx>_{trunc};", 40, 0, 200000);
  hmap_dqdx_trunc["electron"] = new TH1D("h_dqdx_trunc_electron", ";<dQ/dx>_{trunc};", 40, 0, 200000);
  hmap_dqdx_trunc["else"] = new TH1D("h_dqdx_trunc_else", ";<dQ/dx>_{trunc};", 40, 0, 200000);
  
  TH2D *h_dqdx_trunc_length = new TH2D("h_dqdx_trunc_length", ";Candidate Track <dQ/dx>_{trunc};Track Length [cm]", 40, 0, 200000,40, 0, 700);
  TH2D *h_dqdx_trunc_length_muon = new TH2D("h_dqdx_trunc_length_muon", ";Candidate Track <dQ/dx>_{trunc};Track Length [cm]", 40, 0, 200000,40, 0, 700);
  TH2D *h_dqdx_trunc_length_proton = new TH2D("h_dqdx_trunc_length_proton", ";Candidate Track <dQ/dx>_{trunc};Track Length [cm]", 40, 0, 200000,40, 0, 700);

  std::map<std::string,TH1D*> hmap_vtxx;
  hmap_vtxx["total"] = new TH1D("h_vtxx_total", ";Candidate Neutrino Vertex X [cm];", 40, 0, 275);
  hmap_vtxx["signal"] = new TH1D("h_vtxx_signal", ";Candidate Neutrino Vertex X [cm];", 40, 0,275);
  hmap_vtxx["background"] = new TH1D("h_vtxx_background", ";Candidate Neutrino Vertex X [cm];", 40, 0,275);
  
  std::map<std::string,TH1D*> hmap_vtxx_b;
  hmap_vtxx_b["total"] = new TH1D("h_vtxx_total_b", ";Candidate Neutrino Vertex X [cm];", 40, 0, 275);
  hmap_vtxx_b["signal"] = new TH1D("h_vtxx_signal_b", ";Candidate Neutrino Vertex X [cm];", 40, 0,275);
  hmap_vtxx_b["background"] = new TH1D("h_vtxx_background_b", ";Candidate Neutrino Vertex X [cm];", 40, 0,275);
  
  std::map<std::string,TH1D*> hmap_vtxy;
  hmap_vtxy["total"] = new TH1D("h_vtxy_total", ";Candidate Neutrino Vertex Y [cm];", 40, -125,125);
  hmap_vtxy["signal"] = new TH1D("h_vtxy_signal", ";Candidate Neutrino Vertex Y [cm];", 40, -125,125);
  hmap_vtxy["background"] = new TH1D("h_vtxy_background", ";Candidate Neutrino Vertex Y [cm];", 40, -125,125);
  
  std::map<std::string,TH1D*> hmap_vtxy_b;
  hmap_vtxy_b["total"] = new TH1D("h_vtxy_total_b", ";Candidate Neutrino Vertex Y [cm];", 40, -125,125);
  hmap_vtxy_b["signal"] = new TH1D("h_vtxy_signal_b", ";Candidate Neutrino Vertex Y [cm];", 40, -125,125);
  hmap_vtxy_b["background"] = new TH1D("h_vtxy_background_b", ";Candidate Neutrino Vertex Y [cm];", 40, -125,125);
  
  std::map<std::string,TH1D*> hmap_vtxz;
  hmap_vtxz["total"] = new TH1D("h_vtxz_total", ";Candidate Neutrino Vertex Z [cm];", 50, 0,1050);
  hmap_vtxz["signal"] = new TH1D("h_vtxz_signal", ";Candidate Neutrino Vertex Z [cm];", 50, 0,1050);
  hmap_vtxz["background"] = new TH1D("h_vtxz_background", ";Candidate Neutrino Vertex Z [cm];", 50, 0,1050);
  
  std::map<std::string,TH1D*> hmap_vtxz_b;
  hmap_vtxz_b["total"] = new TH1D("h_vtxz_total_b", ";Candidate Neutrino Vertex Z [cm];", 50, 0,1050);
  hmap_vtxz_b["signal"] = new TH1D("h_vtxz_signal_b", ";Candidate Neutrino Vertex Z [cm];", 50, 0,1050);
  hmap_vtxz_b["background"] = new TH1D("h_vtxz_background_b", ";Candidate Neutrino Vertex Z [cm];", 50, 0,1050);
  
  TH2D * h_vtx_xz = new TH2D("h_vtx_xz", ";X;Z", 40, 0, 275, 50, 0,1050);
  TH2D * h_vtx_xy = new TH2D("h_vtx_xy", ";X;Y", 40, 0, 275, 40, -125,125);

  std::map<std::string,TH1D*> hmap_flsmatch_score;
  hmap_flsmatch_score["total"] = new TH1D("h_flsmatch_score_total", ";1/(-log(L));", 100, 0, 1.5);
  hmap_flsmatch_score["signal"] = new TH1D("h_flsmatch_score_signal", ";1/(-log(L));", 100, 0, 1.5);
  hmap_flsmatch_score["background"] = new TH1D("h_flsmatch_score_background", ";1/(-log(L));", 100, 0, 1.5);

  std::map<std::string,TH1D*> hmap_ntpcobj;
  hmap_ntpcobj["total"] = new TH1D("h_ntpcobj_total", ";1/(-log(L));", 10, 0, 10);
  hmap_ntpcobj["signal"] = new TH1D("h_ntpcobj_signal", ";1/(-log(L));", 10, 0, 10);
  hmap_ntpcobj["background"] = new TH1D("h_ntpcobj_background", ";1/(-log(L));", 10, 0, 10);
  
  TH1D* h_pot = new TH1D("h_pot", "First bin contains number of POT (not valid on data)", 1, 0, 1);
  TH1D* h_nevts = new TH1D("h_nevts", "First bin contains number of events", 1, 0, 1);

  
  TH1D * h_deltall_cosmic_stop = new TH1D("h_deltall_cosmic_stop", "Cosmic stopping Muons;MCS Delta LL;", 400, -30, 30);
  TH2D * h_deltall_length_cosmic_stop = new TH2D("h_deltall_length_cosmic_stop", "Cosmic stopping Muons;MCS Delta LL;Track Length [cm]", 70, -30, 30, 70, 0, 700);
  TH1D * h_deltall_cosmic_nostop = new TH1D("h_deltall_cosmic_nostop", "Cosmic non-stopping Muons;MCS Delta LL;", 400, -30, 30);
  TH2D * h_deltall_length_cosmic_nostop = new TH2D("h_deltall_length_cosmic_nostop", "Cosmic non-stopping Muons;MCS Delta LL;Track Length [cm]", 70, -30, 30, 70, 0, 700);
  TH1D * h_deltall_nu = new TH1D("h_deltall_nu", "Neutrino origin;MCS Delta LL;", 400, -30, 30);
  TH2D * h_deltall_length_nu = new TH2D("h_deltall_length_nu", "Neutrino origin;MCS Delta LL;Track Length [cm]", 70, -30, 30, 70, 0, 700);

  
  
  int barWidth = 70;
  
  if(maxEntries > 0.) evts = maxEntries;
  
  int total_events = 0;
  
  for(int i = 0; i < evts; i++) {
    
    DrawProgressBar((double)i/(double)evts, barWidth);
    
    chain_ubxsec->GetEntry(i);
    
    
    //if (t->run > 5804 && t->run < 5886)
    //  continue;
    
    total_events ++;
    
    //cout << "***** Event " << i << endl;
    //cout << "***** Event Number " << t->event << endl;
    
    
    // ************************
    //
    // Preliminary
    //
    // ************************
    
    
    
    

    
    // This variable will store if this is a signal event or not
    bool isSignal = false;
    
    if (t->nupdg == 14 && t->ccnc == 0 && t->fv == 1 && (t->tvtx_z[0] < 675 || t->tvtx_z[0] > 775)){

      nsignal++;
      isSignal = true;

      if (t->mode == 0) nsignal_qe++;
      if (t->mode == 1) nsignal_res++;
      if (t->mode == 2) nsignal_dis++;
      if (t->mode == 3) nsignal_coh++;
      if (t->mode == 10) nsignal_mec++;
      
    }
    
    
    // Check if it's a nue event
    bool isNueCCFV = false;
    if (t->nupdg == 12 && t->ccnc == 0 && t->fv == 1) {
      isNueCCFV = true;
      nue_cc_fv+=1;//t->bnb_weight;
    }
    bool isNue = false;
    if (t->nupdg == 12) {
      isNue = true;
    }

    
    
    // Construct the denominator for the efficiency plots
    if (isSignal) {
      
      h_eff_den->Fill(t->nu_e);
      h_eff_mumom_den->Fill(t->true_muon_mom);
      h_eff_muangle_den->Fill(t->lep_costheta);
      h_eff_muangle_mumom_den->Fill(t->lep_costheta, t->true_muon_mom);
      h_eff_muphi_den->Fill(t->lep_phi);
      h_eff_mult_den->Fill(t->genie_mult);
      h_eff_mult_ch_den->Fill(t->genie_mult_ch);

      if (t->mode == 0) h_eff_qe_den->Fill(t->nu_e);
      if (t->mode == 1) h_eff_res_den->Fill(t->nu_e);
      if (t->mode == 2) h_eff_dis_den->Fill(t->nu_e);
      if (t->mode == 3) h_eff_coh_den->Fill(t->nu_e);
      if (t->mode == 10) h_eff_mec_den->Fill(t->nu_e);

      h_truth_xsec_mumom->Fill(t->true_muon_mom);
      h_truth_xsec_muangle->Fill(t->lep_costheta);

      h_mueff_den->Fill(t->true_muon_mom);
      h_mueff_angle_den->Fill(t->lep_costheta);

      h_true_nu_eng_beforesel->Fill(t->nu_e);
      
      if (t->muon_is_reco){
        h_mumom_nue->Fill(t->nu_e, t->true_muon_mom);
        nSignalWMuonReco++;
        h_mueff_num->Fill(t->true_muon_mom);
        h_mueff_angle_num->Fill(t->lep_costheta);
        for (auto origin : t->slc_origin){
          if (origin == 0 || origin == 2) {
            h_mueff_2_num->Fill(t->true_muon_mom);
            break;
          }
        }
        if (t->vtx_resolution > -1 && t->vtx_resolution < 10) nSignalMuonRecoVtxOk++;
        
        h_muon_track_eff->Fill(t->muon_reco_eff);
        h_muon_track_pur->Fill(t->muon_reco_pur);
        
        h_mu_eff_mom->Fill(t->true_muon_mom, t->muon_reco_eff);
        h_mu_pur_mom->Fill(t->true_muon_mom, t->muon_reco_pur);
      }
      else{
        //std::cout << "This is a signal event but the muon was not reconstructed. Event: " << event << std::endl;
      }
    }
    if(t->nupdg == 14 && t->ccnc == 0){
      nNumuCC++;
    }
    
    
    
    
    
    
    
    // VTX before selection
    if(t->slc_nuvtx_x.size()>0) {
      hmap_vtxx_b["total"]->Fill(t->slc_nuvtx_x.at(0));
      hmap_vtxy_b["total"]->Fill(t->slc_nuvtx_y.at(0));
      hmap_vtxz_b["total"]->Fill(t->slc_nuvtx_z.at(0));
      h_vtx_xz->Fill(t->slc_nuvtx_x.at(0), t->slc_nuvtx_z.at(0));
      h_vtx_xy->Fill(t->slc_nuvtx_x.at(0), t->slc_nuvtx_y.at(0));
    }
    
    // Number of TPCObjects
    hmap_ntpcobj["total"]->Fill(t->nslices);
    if (isSignal) hmap_ntpcobj["signal"]->Fill(t->nslices);
    else hmap_ntpcobj["background"]->Fill(t->nslices);
    
    // Vertex Resolution plot
    if (isSignal) h_vtx_resolution->Fill(t->vtx_resolution);
    
    
    
    //int n_acpt_tagged_per_event = 0;
    for (int slc = 0; slc < t->nslices; slc ++) {
      
      if (t->slc_origin.at(slc) == 0 || t->slc_origin.at(slc) == 2) {
        h_slice_npfp->Fill(t->slc_npfp.at(slc));
        h_slice_ntrack->Fill(t->slc_ntrack.at(slc));
      } else {
        h_slice_npfp_others->Fill(t->slc_npfp.at(slc));
        h_slice_ntrack_others->Fill(t->slc_ntrack.at(slc));
      }
      
      /*if (isSignal) {
        if (t->slc_origin.at(slc) == 0 || t->slc_origin.at(slc) == 2) {
          h_fm_score->Fill(t->slc_flsmatch_score.at(slc));
          h_fm_score_pe->Fill(t->slc_flsmatch_score.at(slc), t->beamfls_pe.at(flashInBeamSpill));
        } else {
          h_fm_score_others->Fill(t->slc_flsmatch_score.at(slc));
        }
        
      }*/
      
      if (t->slc_origin.at(slc) == 0) h_slice_origin->Fill(2);
      if (t->slc_origin.at(slc) == 1) h_slice_origin->Fill(0);
      if (t->slc_origin.at(slc) == 2) h_slice_origin->Fill(1);
      
      //if (t->slc_acpt_outoftime.at(slc) == 1) n_acpt_tagged_per_event++;
      
      if ((t->slc_origin.at(slc) == 0 || t->slc_origin.at(slc) == 2) && t->fv == 1) {
        n_slc_nu_origin ++;
        //if (t->slc_acpt_outoftime.at(slc) == 1) {
        //n_slc_acpt_tag_nu ++;
        //std::cout << "A neutrino was acpt tagged in event " << t->event << std::endl;
        //}
      }
    }
    //h_acpt_tagged->Fill(n_acpt_tagged_per_event);
    
    
    
    
    
    
    
    
    
    
    
    
    
    // ************************
    //
    //  Selection
    //
    // ************************

    if (isSignal) selected_signal_events_percut["initial"]++;
    selected_events_percut["initial"]++;

    
    
    //
    // Optical
    //

    if (t->nbeamfls == 0) continue;
    

    int flashInBeamSpill = -1;
    double old_pe = -1;
    
    for (int fls = 0; fls < t->nbeamfls; fls ++){
      h_flsTime->Fill(t->beamfls_time.at(fls) - _flashShift);
      if(t->beamfls_pe.at(fls) > _pe_cut) {
        h_flsTime_wcut->Fill(t->beamfls_time.at(fls) - _flashShift);
      }
      if (t->beamfls_time.at(fls) > _beamSpillStarts && t->beamfls_time.at(fls) < _beamSpillEnds) {
        
        //flashInBeamSpill = fls;
        if (t->beamfls_pe.at(fls) >= _pe_cut) {
          if (t->beamfls_pe.at(fls) > old_pe) {
            flashInBeamSpill = fls;
            old_pe = t->beamfls_pe.at(fls);
          }
          nEvtsWFlashInBeamSpill++;
        }
      }
    }
    
    
    if (flashInBeamSpill == -1) continue;
    h_flsPe_wcut->Fill(t->beamfls_pe.at(flashInBeamSpill));
    
    h_flsTime_flsPe_wcut->Fill(t->beamfls_time.at(flashInBeamSpill) - _flashShift, t->beamfls_pe.at(flashInBeamSpill));

    h_flsTime_wcut_2->Fill(t->beamfls_time.at(flashInBeamSpill) - _flashShift);
    

    
    
    if (isSignal) h_true_nu_eng_afterflash->Fill(t->nu_e);

    if (isSignal) selected_signal_events_percut["beamflash"]++;
    selected_events_percut["beamflash"]++;
    
    
    

    
  
    // Slice loop
    for (int slc = 0; slc < t->nslices; slc ++) {
      
      
      bool nu_origin = (t->slc_origin.at(slc) == 0 || t->slc_origin.at(slc) == 2);
      
      // PMTs
      
      if (flashInBeamSpill > -1 && t->slc_flsmatch_score.at(slc) > -1
          && t->slc_flsmatch_qllx.at(slc)!= -9999 && t->slc_flsmatch_tpcx.at(slc)!=-9999) {
        hmap_xdiff_b["total"]->Fill(t->slc_flsmatch_qllx.at(slc) - t->slc_flsmatch_tpcx.at(slc));
        hmap_zdiff_b["total"]->Fill(t->slc_flsmatch_hypoz.at(slc) - t->beamfls_z.at(flashInBeamSpill));
        if ( isSignal && nu_origin) {
          hmap_xdiff_b["signal"]->Fill(t->slc_flsmatch_qllx.at(slc) - t->slc_flsmatch_tpcx.at(slc));
          hmap_zdiff_b["signal"]->Fill(t->slc_flsmatch_hypoz.at(slc) - t->beamfls_z.at(flashInBeamSpill));
        } else {
          hmap_xdiff_b["background"]->Fill(t->slc_flsmatch_qllx.at(slc) - t->slc_flsmatch_tpcx.at(slc));
          hmap_zdiff_b["background"]->Fill(t->slc_flsmatch_hypoz.at(slc) - t->beamfls_z.at(flashInBeamSpill));
        }
      }
      
      //   nu
      if ( isSignal && nu_origin && flashInBeamSpill > -1 && t->slc_flsmatch_score.at(slc) > -1){
        for (int pmt = 0; pmt < 32; pmt++) {
          if ((t->slc_flshypo_spec.at(slc))[pmt] < 5 || (t->beamfls_spec.at(flashInBeamSpill))[pmt] < 5) continue;
          double mean = ((t->slc_flshypo_spec.at(slc))[pmt] + (t->beamfls_spec.at(flashInBeamSpill))[pmt]) / 2.;
          h_frac_diff->Fill(pmt, ( (t->slc_flshypo_spec.at(slc))[pmt] - (t->beamfls_spec.at(flashInBeamSpill))[pmt] ) / (mean) );
        }
        if (t->slc_flsmatch_qllx.at(slc)!= -9999 && t->slc_flsmatch_tpcx.at(slc)!=-9999){
          h_xdiff->Fill(t->slc_flsmatch_qllx.at(slc) - t->slc_flsmatch_tpcx.at(slc));
          h_zdiff->Fill(t->slc_flsmatch_hypoz.at(slc) - t->beamfls_z.at(flashInBeamSpill));
        }
      }
      //   others
      if (t->slc_origin.at(slc) == 1 && flashInBeamSpill > -1 && t->slc_flsmatch_score.at(slc) > -1){
        for (int pmt = 0; pmt < 32; pmt++) {
          if ((t->slc_flshypo_spec.at(slc))[pmt] < 5 || (t->beamfls_spec.at(flashInBeamSpill))[pmt] < 5) continue;
          double mean = ((t->slc_flshypo_spec.at(slc))[pmt] + (t->beamfls_spec.at(flashInBeamSpill))[pmt]) / 2.;
          h_frac_diff_others->Fill(pmt, ( (t->slc_flshypo_spec.at(slc))[pmt] - (t->beamfls_spec.at(flashInBeamSpill))[pmt] ) / (mean) );
        }
        if (t->slc_flsmatch_qllx.at(slc)!= -9999 && t->slc_flsmatch_tpcx.at(slc)!=-9999){
          h_xdiff_others->Fill(t->slc_flsmatch_qllx.at(slc) - t->slc_flsmatch_tpcx.at(slc));
          h_zdiff_others->Fill(t->slc_flsmatch_hypoz.at(slc) - t->beamfls_z.at(flashInBeamSpill));
        }
      }
      //  spec
      if (t->event == -1/*150801 4990051 3099969*/) {
        if (flashInBeamSpill > -1 && t->slc_flsmatch_score.at(slc) > -1){
          int instance = 0;
          std::cout << "tpcx " << t->slc_flsmatch_tpcx.at(instance) << std::endl;
          std::cout << "qllx " << t->slc_flsmatch_qllx.at(instance) << std::endl;
          std::cout << "SCORE IS " << t->slc_flsmatch_score.at(instance) << std::endl;
          for (int pmt = 0; pmt < 32; pmt++) {
            hypo_spec_x[pmt] = pmt;
            hypo_spec_y[pmt] = (t->slc_flshypo_spec.at(instance))[pmt]; ;//(t->slc_flshypo_spec.at(3))[pmt];
            meas_spec_x[pmt] = pmt;
            meas_spec_y[pmt] = (t->beamfls_spec.at(flashInBeamSpill))[pmt];
            numc_spec_x[pmt] = pmt;
            //numc_spec_y[pmt] = t->numc_flash_spec.at(pmt);
          }
        }
      }
      
      // CheckVertex
      hmap_vtxcheck_angle["total"]->Fill(t->slc_vtxcheck_angle.at(slc));
      if (isSignal && nu_origin) {
        h_vtxcheck_angle_good->Fill(t->slc_vtxcheck_angle.at(slc));
        hmap_vtxcheck_angle["signal"]->Fill(t->slc_vtxcheck_angle.at(slc));
      } else {
        h_vtxcheck_angle_bad->Fill(t->slc_vtxcheck_angle.at(slc));
        hmap_vtxcheck_angle["background"]->Fill(t->slc_vtxcheck_angle.at(slc));
      }

      // Residuals std, mean
      hmap_residuals_std["total"]->Fill(t->slc_muoncandidate_residuals_std.at(slc));
      hmap_residuals_mean["total"]->Fill(t->slc_muoncandidate_residuals_mean.at(slc));
      hmap_perc_used_hits["total"]->Fill(t->slc_muoncandidate_perc_used_hits_in_cluster.at(slc));
      if(t->slc_muoncandidate_contained.at(slc)) {
        hmap_mom_mcs_length["total"]->Fill(t->slc_muoncandidate_mom_mcs.at(slc) - t->slc_muoncandidate_mom_range.at(slc));
      }
      if (isSignal && nu_origin) {
        hmap_residuals_std["signal"]->Fill(t->slc_muoncandidate_residuals_std.at(slc));
        hmap_residuals_mean["signal"]->Fill(t->slc_muoncandidate_residuals_mean.at(slc));
        hmap_perc_used_hits["signal"]->Fill(t->slc_muoncandidate_perc_used_hits_in_cluster.at(slc));
        if(t->slc_muoncandidate_contained.at(slc)) {
          hmap_mom_mcs_length["signal"]->Fill(t->slc_muoncandidate_mom_mcs.at(slc) - t->slc_muoncandidate_mom_range.at(slc));
        }
      } else {
        hmap_residuals_std["background"]->Fill(t->slc_muoncandidate_residuals_std.at(slc));
        hmap_residuals_mean["background"]->Fill(t->slc_muoncandidate_residuals_mean.at(slc));
        hmap_perc_used_hits["background"]->Fill(t->slc_muoncandidate_perc_used_hits_in_cluster.at(slc));
        if(t->slc_muoncandidate_contained.at(slc))  {
          hmap_mom_mcs_length["background"]->Fill(t->slc_muoncandidate_mom_mcs.at(slc) - t->slc_muoncandidate_mom_range.at(slc));
        }
      }
      
      // Track chi2
      h_chi2->Fill(t->slc_kalman_chi2.at(slc)/(double)t->slc_kalman_ndof.at(slc));
      
      // Number of TPCObjects per event
      h_nslices->Fill(t->nslices);
      
      
      
      
    } // slice loop
    
    
    
      
    int n_slc_flsmatch = 0;
    
    // Find slice with maximum score
    double score_max = -1;
    int scl_ll_max = -1;
    for (int slc = 0; slc < t->nslices; slc ++){
      
      if (t->slc_flsmatch_score.at(slc) > 0.00000001) {
        n_slc_flsmatch++;
      }
      
      if(t->slc_flsmatch_score.at(slc) > score_max){
        scl_ll_max = slc;
        score_max = t->slc_flsmatch_score.at(slc);
      }
    }
    
    h_n_slc_flsmatch->Fill(n_slc_flsmatch);
    
    
    if (scl_ll_max == -1) continue;
    
    
    bool nu_origin = (t->slc_origin.at(scl_ll_max) == 0 || t->slc_origin.at(scl_ll_max) == 2);
    
    
    h_flsTime_wcut_3->Fill(t->beamfls_time.at(flashInBeamSpill) - _flashShift);
    
    if (isSignal && nu_origin) nSignalFlashMatched ++;

    if (score_max <= 3e-4) continue;
    if (std::isinf(score_max)) continue;


    if (isSignal && nu_origin) selected_signal_events_percut["flash_match"]++;
    selected_events_percut["flash_match"]++;
    
    
    h_flsTime_wcut_4->Fill(t->beamfls_time.at(flashInBeamSpill) - _flashShift);
    h_deltax->Fill(t->slc_flsmatch_qllx.at(scl_ll_max) - t->slc_flsmatch_tpcx.at(scl_ll_max));
    h_deltax_2d->Fill(t->slc_flsmatch_qllx.at(scl_ll_max), t->slc_flsmatch_tpcx.at(scl_ll_max));
    h_deltaz_4->Fill(t->slc_flsmatch_hypoz.at(scl_ll_max) - t->beamfls_z.at(flashInBeamSpill));
    

    
    if(t->slc_flsmatch_qllx.at(scl_ll_max) - t->slc_flsmatch_tpcx.at(scl_ll_max) > 50) continue;
    
    h_flsTime_wcut_5->Fill(t->beamfls_time.at(flashInBeamSpill) - _flashShift);

    if(t->slc_flsmatch_qllx.at(scl_ll_max) - t->slc_flsmatch_tpcx.at(scl_ll_max) < -100) continue;

    if (isSignal && nu_origin) selected_signal_events_percut["flash_match_deltax"]++;
    selected_events_percut["flash_match_deltax"]++;
    
    h_flsTime_wcut_6->Fill(t->beamfls_time.at(flashInBeamSpill) - _flashShift);
    h_deltaz_6->Fill(t->slc_flsmatch_hypoz.at(scl_ll_max) - t->beamfls_z.at(flashInBeamSpill));
    
    if(t->slc_flsmatch_hypoz.at(scl_ll_max) - t->beamfls_z.at(flashInBeamSpill) > 75) continue;
    
    h_flsTime_wcut_7->Fill(t->beamfls_time.at(flashInBeamSpill) - _flashShift);

    if(t->slc_flsmatch_hypoz.at(scl_ll_max) - t->beamfls_z.at(flashInBeamSpill) < -75) continue;

    if (isSignal && nu_origin) selected_signal_events_percut["flash_match_deltaz"]++;
    selected_events_percut["flash_match_deltaz"]++;
    
    h_flsTime_wcut_8->Fill(t->beamfls_time.at(flashInBeamSpill) - _flashShift);

    
    
    double dqdx_calib = t->slc_muoncandidate_dqdx_trunc.at(scl_ll_max) * _gainCalib;
  

    if(t->slc_vtxcheck_angle.at(scl_ll_max) > 2.9) continue;
    
    //if(t->slc_vtxcheck_angle.at(scl_ll_max) < 0.05 && t->slc_vtxcheck_angle.at(scl_ll_max) !=-9999 ) continue;
    
    if(t->slc_ntrack.at(scl_ll_max) == 0) continue;
    
    if(!t->slc_passed_min_track_quality.at(scl_ll_max)) continue;
    
    if(!t->slc_passed_min_vertex_quality.at(scl_ll_max)) continue;

    if (isSignal && nu_origin) selected_signal_events_percut["quality"]++;
    selected_events_percut["quality"]++;
    
    //if (!t->slc_muoncandidate_contained.at(scl_ll_max)) continue;
    
    if(t->slc_muoncandidate_contained.at(scl_ll_max) && (t->slc_muoncandidate_mom_mcs.at(scl_ll_max) - t->slc_muoncandidate_mom_range.at(scl_ll_max) > 0.2)) continue;
    
    if (isSignal && nu_origin) selected_signal_events_percut["mcs_length_quality"]++;
    selected_events_percut["mcs_length_quality"]++;

    //if(dqdx_calib > 70000) continue;
    //if (t->slc_muoncandidate_length.at(scl_ll_max) > 100 && dqdx_calib > 70000) continue;
    //else if (t->slc_muoncandidate_length.at(scl_ll_max) <= 100 && dqdx_calib > 100000) continue;
    
    // mcc8.3 std::vector<double> svm_x = {92200, 91700, 91200, 90700, 90250, 89800, 89350, 88900, 88500, 88100, 87700, 87300, 86900, 86550, 86200, 85850, 85500, 85150, 84800, 84500, 84200, 83850, 83550, 83250, 82950, 82650, 82400, 82100, 81850, 81550, 81300, 81050, 80800, 80550, 80300, 80050, 79800, 79600, 79350, 79150, 78900, 78700, 78450, 78250, 78050, 77850, 77650, 77450, 77250, 77050, 76850, 76650, 76500, 76300, 76100, 75950, 75750, 75600, 75400, 75250, 75100, 74900, 74750, 74600, 74450, 74300, 74150, 74000, 73850, 73700, 73550, 73400, 73250, 73100, 73000, 72850, 72700, 72600, 72450, 72300, 72200, 72050, 71950, 71800, 71700, 71600, 71450, 71350, 71250, 71100, 71000, 70900, 70800, 70700, 70550, 70450, 70350, 70250, 70150, 70050, 69950, 69850, 69750, 69650, 69550, 69500, 69400, 69300, 69200, 69100, 69050, 68950, 68850, 68800, 68700, 68600, 68550, 68450, 68350, 68300, 68200, 68150, 68050, 68000, 67900, 67850, 67750, 67700, 67650, 67550, 67500, 67400, 67350, 67300, 67250, 67150, 67100, 67050, 67000, 66900, 66850, 66800, 66750, 66700, 66600, 66550, 66500, 66450, 66400, 66350, 66300, 66250, 66200, 66150, 66100, 66050, 66000, 65950, 65900, 65850, 65800, 65750, 65700, 65650, 65650, 65600, 65550, 65500, 65450, 65400, 65400, 65350, 65300, 65250, 65250, 65200, 65150, 65100, 65100, 65050, 65000, 65000, 64950, 64900, 64900, 64850, 64800, 64800, 64750, 64750, 64700, 64650, 64650, 64600, 64600, 64550, 64550, 64500, 64500, 64450, 64450, 64400, 64400, 64350, 64350, 64300, 64300, 64300, 64250, 64250, 64200, 64200, 64200, 64150, 64150, 64150, 64100, 64100, 64100, 64050, 64050, 64050, 64000, 64000, 64000, 63950, 63950, 63950, 63950, 63900, 63900, 63900, 63900, 63900, 63850, 63850, 63850, 63850, 63850, 63800, 63800, 63800, 63800, 63800, 63800, 63800, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63700, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63750, 63800, 63800, 63800, 63800, 63800, 63800, 63800, 63850, 63850, 63850, 63850, 63850, 63850, 63900, 63900, 63900, 63900, 63900, 63950, 63950, 63950, 63950, 63950, 64000, 64000, 64000, 64000, 64050, 64050, 64050, 64050, 64100, 64100, 64100, 64150, 64150, 64150, 64150, 64200, 64200, 64200, 64250, 64250, 64250, 64300, 64300, 64300, 64350, 64350, 64350, 64400, 64400, 64400, 64450, 64450, 64450, 64500, 64500, 64550, 64550, 64550, 64600, 64600, 64600, 64650, 64650, 64700, 64700, 64750, 64750, 64750, 64800, 64800, 64850, 64850, 64900, 64900, 64900, 64950, 64950, 65000, 65000, 65050, 65050, 65100, 65100, 65150, 65150, 65200, 65200, 65250, 65250, 65300, 65300, 65350, 65350, 65400, 65400, 65450, 65450, 65500, 65500, 65550, 65550, 65600, 65650, 65650, 65700, 65700, 65750, 65750, 65800, 65800, 65850, 65900, 65900, 65950, 65950, 66000, 66000, 66050, 66100, 66100, 66150, 66150, 66200, 66250, 66250, 66300, 66350, 66350, 66400, 66400, 66450, 66500, 66500, 66550, 66600, 66600, 66650, 66700, 66700, 66750, 66800, 66800, 66850, 66850, 66900, 66950, 67000, 67000, 67050, 67100, 67100, 67150, 67200, 67200, 67250, 67300, 67300, 67350, 67400, 67400, 67450, 67500, 67550, 67550, 67600, 67650, 67700, 67700, 67750, 67800, 67800, 67850, 67900, 67950, 67950, 68000, 68050, 68100, 68100, 68150, 68200, 68250, 68250, 68300, 68350, 68400, 68400, 68450, 68500, 68550, 68600, 68600, 68650, 68700, 68750, 68800, 68800, 68850, 68900, 68950, 69000, 69000, 69050, 69100, 69150, 69200, 69200, 69250, 69300, 69350, 69400, 69400, 69450, 69500, 69550, 69600, 69650, 69650, 69700, 69750, 69800, 69850, 69900, 69950, 69950, 70000, 70050, 70100, 70150, 70200, 70250, 70250, 70300, 70350, 70400, 70450, 70500, 70550, 70550, 70600, 70650, 70700, 70750, 70800, 70850, 70900, 70950, 70950, 71000, 71050, 71100, 71150, 71200, 71250, 71300, 71350, 71400, 71450, 71450, 71500, 71550, 71600, 71650, 71700, 71750, 71800, 71850, 71900, 71950, 72000, 72050, 72050, 72100, 72150, 72200, 72250, 72300, 72350, 72400, 72450, 72500, 72550, 72600, 72650, 72700, 72750, 72800, 72850, 72900, 72950, 73000, 73050, 73100, 73150, 73150, 73200, 73250, 73300, 73350, 73400, 73450, 73500, 73550, 73600, 73650, 73700, 73750, 73800, 73850, 73900, 73950, 74000, 74050, 74100, 74150, 74200, 74250, 74300, 74350, 74400, 74450, 74500, 74550, 74600, 74650, 74700, 74750, 74850, 74900, 74950, 75000, 75050, 75100, 75150, 75200, 75250, 75300, 75350, 75400, 75450, 75500, 75550, 75600, 75650, 75700, 75750, 75800, 75850, 75900, 75950, 76000, 76050, 76150, 76200, 76250, 76300, 76350, 76400, 76450, 76500, 76550, 76600, 76650, 76700, 76750, 76800, 76850, 76950, 77000, 77050, 77100, 77150, 77200, 77250, 77300, 77350, 77400, 77450, 77500, 77600, 77650, 77700, 77750, 77800, 77850, 77900, 77950, 78000, 78050, 78150, 78200, 78250, 78300, 78350, 78400, 78450, 78500, 78550, 78650, 78700, 78750, 78800, 78850, 78900, 78950, 79000, 79100, 79150, 79200, 79250, 79300, 79350, 79400, 79450, 79550, 79600, 79650, 79700, 79750, 79800, 79850, 79950, 80000, 80050, 80100, 80150, 80200, 80250, 80350, 80400, 80450, 80500, 80550, 80600, 80650, 80750, 80800, 80850, 80900, 80950, 81000, 81100, 81150, 81200, 81250, 81300, 81350, 81450, 81500, 81550, 81600, 81650, 81700, 81800, 81850, 81900, 81950, 82000, 82050, 82150, 82200, 82250, 82300, 82350, 82450, 82500, 82550, 82600, 82650, 82700, 82800, 82850, 82900, 82950, 83000, 83100, 83150, 83200, 83250, 83300, 83400, 83450, 83500, 83550, 83600, 83700, 83750, 83800, 83850, 83900, 84000, 84050, 84100, 84150, 84250, 84300, 84350, 84400, 84450, 84550, 84600, 84650, 84700, 84800, 84850, 84900, 84950, 85000, 85100, 85150, 85200, 85250, 85350, 85400, 85450, 85500, 85550, 85650, 85700, 85750, 85800, 85900, 85950, 86000, 86050, 86150, 86200, 86250, 86300, 86400, 86450, 86500, 86550, 86650, 86700, 86750, 86800, 86900, 86950, 87000, 87050, 87150, 87200, 87250, 87300, 87400, 87450, 87500, 87550, 87650, 87700, 87750, 87800, 87900, 87950, 88000, 88050, 88150, 88200, 88250, 88300, 88400, 88450, 88500, 88600, 88650, 88700, 88750, 88850, 88900, 88950, 89000, 89100, 89150, 89200, 89300, 89350, 89400, 89450, 89550, 89600, 89650, 89750, 89800, 89850, 89900, 90000, 90050, 90100, 90200, 90250, 90300, 90350, 90450, 90500, 90550, 90650, 90700, 90750, 90850, 90900, 90950, 91000, 91100, 91150, 91200, 91300, 91350, 91400, 91500, 91550, 91600, 91650, 91750, 91800, 91850, 91950, 92000, 92050, 92150, 92200, 92250, 92350, 92400, 92450, 92550, 92600, 92650, 92700, 92800, 92850, 92900, 93000, 93050, 93100, 93200, 93250, 93300, 93400, 93450, 93500, 93600, 93650, 93700, 93800, 93850, 93900, 94000, 94050, 94100, 94200, 94250, 94300, 94400, 94450, 94500, 94600, 94650, 94700, 94800, 94850, 94900, 95000, 95050, 95100, 95200, 95250, 95300, 95400, 95450, 95500, 95600, 95650, 95700, 95800, 95850, 95900, 96000, 96050, 96100, 96200, 96250, 96350, 96400, 96450, 96550, 96600, 96650, 96750, 96800, 96850, 96950, 97000, 97050, 97150, 97200, 97250, 97350, 97400, 97500, 97550, 97600, 97700, 97750, 97800, 97900, 97950, 98000, 98100, 98150, 98250};
    std::vector<double> svm_x = {86300, 86050, 85850, 85600, 85400, 85150, 84950, 84700, 84500, 84300, 84100, 83850, 83650, 83450, 83250, 83050, 82850, 82650, 82450, 82250, 82050, 81900, 81700, 81500, 81300, 81150, 80950, 80750, 80600, 80400, 80250, 80050, 79900, 79750, 79550, 79400, 79250, 79050, 78900, 78750, 78600, 78400, 78250, 78100, 77950, 77800, 77650, 77500, 77350, 77200, 77050, 76900, 76750, 76650, 76500, 76350, 76200, 76050, 75950, 75800, 75650, 75550, 75400, 75300, 75150, 75000, 74900, 74750, 74650, 74500, 74400, 74250, 74150, 74050, 73900, 73800, 73700, 73550, 73450, 73350, 73250, 73100, 73000, 72900, 72800, 72700, 72550, 72450, 72350, 72250, 72150, 72050, 71950, 71850, 71750, 71650, 71550, 71450, 71350, 71250, 71150, 71100, 71000, 70900, 70800, 70700, 70600, 70550, 70450, 70350, 70250, 70200, 70100, 70000, 69950, 69850, 69750, 69700, 69600, 69550, 69450, 69350, 69300, 69200, 69150, 69050, 69000, 68900, 68850, 68750, 68700, 68600, 68550, 68500, 68400, 68350, 68250, 68200, 68150, 68050, 68000, 67950, 67850, 67800, 67750, 67700, 67600, 67550, 67500, 67450, 67350, 67300, 67250, 67200, 67150, 67100, 67000, 66950, 66900, 66850, 66800, 66750, 66700, 66650, 66600, 66550, 66500, 66450, 66400, 66350, 66300, 66250, 66200, 66150, 66100, 66050, 66000, 65950, 65900, 65850, 65800, 65750, 65750, 65700, 65650, 65600, 65550, 65500, 65450, 65450, 65400, 65350, 65300, 65250, 65250, 65200, 65150, 65100, 65100, 65050, 65000, 65000, 64950, 64900, 64850, 64850, 64800, 64750, 64750, 64700, 64700, 64650, 64600, 64600, 64550, 64500, 64500, 64450, 64450, 64400, 64400, 64350, 64350, 64300, 64250, 64250, 64200, 64200, 64150, 64150, 64100, 64100, 64100, 64050, 64050, 64000, 64000, 63950, 63950, 63900, 63900, 63900, 63850, 63850, 63800, 63800, 63800, 63750, 63750, 63750, 63700, 63700, 63700, 63650, 63650, 63650, 63600, 63600, 63600, 63550, 63550, 63550, 63500, 63500, 63500, 63500, 63450, 63450, 63450, 63450, 63400, 63400, 63400, 63400, 63400, 63350, 63350, 63350, 63350, 63350, 63350, 63300, 63300, 63300, 63300, 63300, 63300, 63250, 63250, 63250, 63250, 63250, 63250, 63250, 63250, 63250, 63250, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63200, 63250, 63250, 63250, 63250, 63250, 63250, 63250, 63250, 63250, 63250, 63300, 63300, 63300, 63300, 63300, 63300, 63300, 63350, 63350, 63350, 63350, 63350, 63350, 63400, 63400, 63400, 63400, 63400, 63400, 63450, 63450, 63450, 63450, 63500, 63500, 63500, 63500, 63500, 63550, 63550, 63550, 63550, 63600, 63600, 63600, 63600, 63650, 63650, 63650, 63650, 63700, 63700, 63700, 63750, 63750, 63750, 63800, 63800, 63800, 63800, 63850, 63850, 63850, 63900, 63900, 63900, 63950, 63950, 63950, 64000, 64000, 64000, 64050, 64050, 64100, 64100, 64100, 64150, 64150, 64150, 64200, 64200, 64250, 64250, 64250, 64300, 64300, 64350, 64350, 64350, 64400, 64400, 64450, 64450, 64450, 64500, 64500, 64550, 64550, 64600, 64600, 64650, 64650, 64650, 64700, 64700, 64750, 64750, 64800, 64800, 64850, 64850, 64900, 64900, 64950, 64950, 65000, 65000, 65050, 65050, 65100, 65100, 65150, 65150, 65200, 65200, 65250, 65250, 65300, 65300, 65350, 65350, 65400, 65400, 65450, 65500, 65500, 65550, 65550, 65600, 65600, 65650, 65650, 65700, 65750, 65750, 65800, 65800, 65850, 65850, 65900, 65950, 65950, 66000, 66000, 66050, 66100, 66100, 66150, 66150, 66200, 66250, 66250, 66300, 66350, 66350, 66400, 66400, 66450, 66500, 66500, 66550, 66600, 66600, 66650, 66700, 66700, 66750, 66750, 66800, 66850, 66850, 66900, 66950, 66950, 67000, 67050, 67050, 67100, 67150, 67150, 67200, 67250, 67300, 67300, 67350, 67400, 67400, 67450, 67500, 67500, 67550, 67600, 67650, 67650, 67700, 67750, 67750, 67800, 67850, 67900, 67900, 67950, 68000, 68050, 68050, 68100, 68150, 68150, 68200, 68250, 68300, 68300, 68350, 68400, 68450, 68450, 68500, 68550, 68600, 68650, 68650, 68700, 68750, 68800, 68800, 68850, 68900, 68950, 69000, 69000, 69050, 69100, 69150, 69150, 69200, 69250, 69300, 69350, 69350, 69400, 69450, 69500, 69550, 69600, 69600, 69650, 69700, 69750, 69800, 69800, 69850, 69900, 69950, 70000, 70050, 70050, 70100, 70150, 70200, 70250, 70300, 70300, 70350, 70400, 70450, 70500, 70550, 70600, 70600, 70650, 70700, 70750, 70800, 70850, 70900, 70900, 70950, 71000, 71050, 71100, 71150, 71200, 71250, 71300, 71300, 71350, 71400, 71450, 71500, 71550, 71600, 71650, 71700, 71700, 71750, 71800, 71850, 71900, 71950, 72000, 72050, 72100, 72150, 72200, 72200, 72250, 72300, 72350, 72400, 72450, 72500, 72550, 72600, 72650, 72700, 72750, 72800, 72850, 72850, 72900, 72950, 73000, 73050, 73100, 73150, 73200, 73250, 73300, 73350, 73400, 73450, 73500, 73550, 73600, 73650, 73700, 73750, 73800, 73850, 73900, 73900, 73950, 74000, 74050, 74100, 74150, 74200, 74250, 74300, 74350, 74400, 74450, 74500, 74550, 74600, 74650, 74700, 74750, 74800, 74850, 74900, 74950, 75000, 75050, 75100, 75150, 75200, 75250, 75300, 75350, 75400, 75450, 75500, 75550, 75600, 75650, 75700, 75750, 75800, 75850, 75900, 76000, 76050, 76100, 76150, 76200, 76250, 76300, 76350, 76400, 76450, 76500, 76550, 76600, 76650, 76700, 76750, 76800, 76850, 76900, 76950, 77000, 77050, 77100, 77200, 77250, 77300, 77350, 77400, 77450, 77500, 77550, 77600, 77650, 77700, 77750, 77800, 77850, 77900, 78000, 78050, 78100, 78150, 78200, 78250, 78300, 78350, 78400, 78450, 78500, 78550, 78600, 78700, 78750, 78800, 78850, 78900, 78950, 79000, 79050, 79100, 79150, 79250, 79300, 79350, 79400, 79450, 79500, 79550, 79600, 79650, 79750, 79800, 79850, 79900, 79950, 80000, 80050, 80100, 80150, 80250, 80300, 80350, 80400, 80450, 80500, 80550, 80600, 80700, 80750, 80800, 80850, 80900, 80950, 81000, 81100, 81150, 81200, 81250, 81300, 81350, 81400, 81450, 81550, 81600, 81650, 81700, 81750, 81800, 81900, 81950, 82000, 82050, 82100, 82150, 82200, 82300, 82350, 82400, 82450, 82500, 82550, 82650, 82700, 82750, 82800, 82850, 82900, 83000, 83050, 83100, 83150, 83200, 83250, 83350, 83400, 83450, 83500, 83550, 83650, 83700, 83750, 83800, 83850, 83900, 84000, 84050, 84100, 84150, 84200, 84300, 84350, 84400, 84450, 84500, 84600, 84650, 84700, 84750, 84800, 84900, 84950, 85000, 85050, 85100, 85200, 85250, 85300, 85350, 85400, 85500, 85550, 85600, 85650, 85700, 85800, 85850, 85900, 85950, 86000, 86100, 86150, 86200, 86250, 86350, 86400, 86450, 86500, 86550, 86650, 86700, 86750, 86800, 86900, 86950, 87000, 87050, 87150, 87200, 87250, 87300, 87350, 87450, 87500, 87550, 87600, 87700, 87750, 87800, 87850, 87950, 88000, 88050, 88100, 88200, 88250, 88300, 88350, 88450, 88500, 88550, 88600, 88700, 88750, 88800, 88850, 88950, 89000, 89050, 89100, 89200, 89250, 89300, 89350, 89450, 89500, 89550, 89600, 89700, 89750, 89800, 89900, 89950, 90000, 90050, 90150, 90200, 90250, 90300, 90400, 90450, 90500, 90600, 90650, 90700, 90750, 90850, 90900, 90950, 91000, 91100, 91150, 91200, 91300, 91350, 91400, 91450, 91550, 91600, 91650, 91750, 91800, 91850, 91950, 92000, 92050, 92100, 92200, 92250};

    double l = std::round(t->slc_muoncandidate_length.at(scl_ll_max));
    double dqdx_cut = 200000;
    if (l >= 0 && l < 1000) {
      dqdx_cut = svm_x.at(l);
    }
    /*if (l >= 0 && l<=30) {
      dqdx_cut = dqdx_cut+50000;
    }*/
    //if (t->lep_costheta < -0.5 && dqdx_calib > 100000 && t->ccnc == 0)
      //std::cout << "low costheta, dqdx_calib: " << dqdx_calib << ", length: " << t->slc_muoncandidate_length.at(scl_ll_max)  << ", event: " << t->event << ", reco theta: " << t->slc_longesttrack_theta.at(scl_ll_max)  << std::endl;
    
    if (dqdx_calib > dqdx_cut) continue;

    if (isSignal && nu_origin) selected_signal_events_percut["mip_consistency"]++;
    selected_events_percut["mip_consistency"]++;

    if(t->slc_nuvtx_fv.at(scl_ll_max) == 0) continue;
    if(t->slc_nuvtx_z.at(scl_ll_max) > 675 && t->slc_nuvtx_z.at(scl_ll_max) < 775) continue;
    //if(t->slc_nuvtx_z.at(scl_ll_max) < 300) continue;

    if (isSignal && nu_origin) selected_signal_events_percut["fiducial_volume"]++;
    selected_events_percut["fiducial_volume"]++;
    
    //if (t->slc_consistency_score.at(scl_ll_max) > 0.65 && t->slc_consistency_score.at(scl_ll_max) < 0.75) continue;

    //if(t->slc_geocosmictag.at(scl_ll_max)) continue;
    
    //if(t->slc_crosses_top_boundary.at(scl_ll_max) && t->slc_muoncandidate_mcs_delta_ll.at(scl_ll_max) < -5) continue;
    
    
    //if(t->slc_ntrack.at(scl_ll_max) == 1 && t->slc_crosses_top_boundary.at(scl_ll_max) == 1) continue;
    
    //if(t->slc_longesttrack_length.at(scl_ll_max) < 25.) continue;
    
    //if(!t->slc_iscontained.at(scl_ll_max)) continue;
    
    //if(t->slc_crosses_top_boundary.at(scl_ll_max) == 1) continue;

    
    if (t->slc_muoncandidate_residuals_std.at(scl_ll_max) > 2.5) continue;
    //if (t->slc_muoncandidate_residuals_std.at(scl_ll_max) < 0.1) continue;
    if (std::abs(t->slc_muoncandidate_residuals_mean.at(scl_ll_max)) > 0.7) continue;
    if (t->slc_muoncandidate_perc_used_hits_in_cluster.at(scl_ll_max) < 0.7) continue;
    //if (t->slc_muoncandidate_maxscatteringangle.at(scl_ll_max) > 20) continue;
    //if (dqdx_calib < 0) continue;

    /*
    
    // Find longest shower in the event
    //bool found = false;
    double s_max_length = -1, s_max_length_index = -1;
    for (size_t s = 0; s < t->nslices; s++) {
      if (s == scl_ll_max) continue;
      if (t->slc_longestshower_length.at(s) == -9999) continue;
        
      //if (t->slc_longestshower_theta.at(s) > 0.5 
      //    && std::abs(t->slc_longestshower_startz.at(s) - t->beamfls_z.at(flashInBeamSpill)) < 100)
      //  found = true;

      if (t->slc_longestshower_theta.at(s) > s_max_length) {
        s_max_length = t->slc_longestshower_theta.at(s);
        s_max_length_index = s;
      }      
    }

    //if (found) continue;

    // Check all shower cuts
    bool shower_exists = false;
    bool shower_theta_cut = false;
    bool shower_phi_cut = false;
    bool shower_flash_cut = false;
    if (s_max_length_index > -1) {
      shower_exists = true;
      shower_theta_cut = t->slc_longestshower_theta.at(s_max_length_index) > 0;
      shower_phi_cut = t->slc_longestshower_phi.at(s_max_length_index) > 2
                       || t->slc_longestshower_phi.at(s_max_length_index) < -2
                       || (t->slc_longestshower_phi.at(s_max_length_index) > -1.1415 && t->slc_longestshower_phi.at(s_max_length_index) < 1.1415);
      shower_flash_cut = std::abs(t->slc_longestshower_startz.at(s_max_length_index) - t->beamfls_z.at(flashInBeamSpill)) < 200;
    }

    // If all shower cuts are satisfied, there is probably a nue in this event, reject it
    //if (shower_exists && shower_flash_cut && shower_theta_cut && shower_phi_cut) continue;
    //if (shower_exists && shower_theta_cut && shower_flash_cut) continue;
    */
    
    
    //
    // EVENT IS SELECTED
    //


    //if (!t->is_selected) std::cout << "event is selected, but not in filter, event " << t->event << std::endl;
    
    //std::cout << ">>>>>>>>>>>>>>>>> Event is selected, " << t->run << ", " << t->subrun << ", " << t->event << ", slice " << scl_ll_max << std::endl;
    
    hmap_trklen["total"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
    hmap_trkmom["total"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
    hmap_trkphi["total"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
    hmap_trktheta["total"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
    hmap_multpfp["total"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
    hmap_multtracktol["total"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
    hmap_trktheta_trkmom["total"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));

    if (isSignal) h_true_nu_eng_aftersel->Fill(t->nu_e);

    if (isSignal) {
      _mom_tree_true = t->true_muon_mom;
      _mom_tree_mcs = t->slc_muoncandidate_mom_mcs.at(scl_ll_max);
      _mom_tree_contained = t->slc_muoncandidate_contained.at(scl_ll_max);
      _mom_tree_selected = true;

      _mom_tree_angle_true = t->lep_costheta;
      _mom_tree_angle_reco = t->slc_muoncandidate_theta.at(scl_ll_max);
      _mom_tree->Fill();

      h_true_reco_mom->Fill(_mom_tree_true, _mom_tree_mcs);
      h_true_reco_costheta->Fill(_mom_tree_angle_true, _mom_tree_angle_reco);
    }
    
    int true_pdg = t->slc_muoncandidate_truepdg.at(scl_ll_max);
    //int true_origin = t->slc_muoncandidate_trueorigin.at(scl_ll_max);
    
    //
    // Fill dQ/ds histograms
    //
    hmap_dqdx_trunc["total"]->Fill(dqdx_calib);
    h_dqdx_trunc_length->Fill(dqdx_calib, t->slc_muoncandidate_length.at(scl_ll_max));
    if (true_pdg == 13 || true_pdg == -13) {
      hmap_dqdx_trunc["muon"]->Fill(dqdx_calib);
      h_dqdx_trunc_length_muon->Fill(dqdx_calib, t->slc_muoncandidate_length.at(scl_ll_max));
      if (dqdx_calib >= 0. && dqdx_calib <=200000. && t->slc_muoncandidate_length.at(scl_ll_max) < 2000) {
        _csvfile << dqdx_calib << ","
                 << t->slc_muoncandidate_length.at(scl_ll_max) << "," << "1" << std::endl;
      }
    } else if (true_pdg == 211 || true_pdg == -211) {
      hmap_dqdx_trunc["pion"]->Fill(dqdx_calib);
    } else if (true_pdg == 2212) {
      hmap_dqdx_trunc["proton"]->Fill(dqdx_calib);
      h_dqdx_trunc_length_proton->Fill(dqdx_calib, t->slc_muoncandidate_length.at(scl_ll_max));
      if (dqdx_calib >= 0. && dqdx_calib <=200000. && t->slc_muoncandidate_length.at(scl_ll_max) < 2000) {
        _csvfile << dqdx_calib << ","
                 << t->slc_muoncandidate_length.at(scl_ll_max) << "," << "0" << std::endl;
      }
    } else if (true_pdg == 22) {
      hmap_dqdx_trunc["photon"]->Fill(dqdx_calib);
    } else if (true_pdg == 11 || true_pdg == -11) {
      hmap_dqdx_trunc["electron"]->Fill(dqdx_calib);
    } else {
      hmap_dqdx_trunc["else"]->Fill(dqdx_calib);
    }
    
    
    double hypo_pe = 0;
    for (int pmt = 0; pmt < 32; pmt++) {
      hypo_pe += (t->slc_flshypo_spec.at(scl_ll_max))[pmt];
    }
    hmap_xdiff["total"]->Fill(t->slc_flsmatch_qllx.at(scl_ll_max) - t->slc_flsmatch_tpcx.at(scl_ll_max));
    hmap_zdiff["total"]->Fill(t->slc_flsmatch_hypoz.at(scl_ll_max) - t->beamfls_z.at(flashInBeamSpill));
    hmap_pediff["total"]->Fill(hypo_pe - t->beamfls_pe.at(flashInBeamSpill));
    
    if (t->event == 150801)
      std::cout << "hypo_pe: " << hypo_pe << ", reco_pe: " << t->beamfls_pe.at(flashInBeamSpill) << std::endl;
    
    hmap_vtxx["total"]->Fill(t->slc_nuvtx_x.at(scl_ll_max));
    hmap_vtxy["total"]->Fill(t->slc_nuvtx_y.at(scl_ll_max));
    hmap_vtxz["total"]->Fill(t->slc_nuvtx_z.at(scl_ll_max));
    
    hmap_flsmatch_score["total"]->Fill(t->slc_flsmatch_score.at(scl_ll_max));
    
    // SIGNAL
    if ( isSignal && nu_origin) {
      hmap_xdiff["signal"]->Fill(t->slc_flsmatch_qllx.at(scl_ll_max) - t->slc_flsmatch_tpcx.at(scl_ll_max));
      hmap_zdiff["signal"]->Fill(t->slc_flsmatch_hypoz.at(scl_ll_max) - t->beamfls_z.at(flashInBeamSpill));
      hmap_pediff["signal"]->Fill(hypo_pe - t->beamfls_pe.at(flashInBeamSpill)/2);
      
      hmap_vtxx["signal"]->Fill(t->slc_nuvtx_x.at(scl_ll_max));
      hmap_vtxy["signal"]->Fill(t->slc_nuvtx_y.at(scl_ll_max));
      hmap_vtxz["signal"]->Fill(t->slc_nuvtx_z.at(scl_ll_max));
      
      hmap_flsmatch_score["signal"]->Fill(t->slc_flsmatch_score.at(scl_ll_max));
    }
    // BACKGROUND
    else {
      hmap_xdiff["background"]->Fill(t->slc_flsmatch_qllx.at(scl_ll_max) - t->slc_flsmatch_tpcx.at(scl_ll_max));
      hmap_zdiff["background"]->Fill(t->slc_flsmatch_hypoz.at(scl_ll_max) - t->beamfls_z.at(flashInBeamSpill));
      hmap_pediff["background"]->Fill(hypo_pe - t->beamfls_pe.at(flashInBeamSpill)/2);

      hmap_vtxx["background"]->Fill(t->slc_nuvtx_x.at(scl_ll_max));
      hmap_vtxy["background"]->Fill(t->slc_nuvtx_y.at(scl_ll_max));
      hmap_vtxz["background"]->Fill(t->slc_nuvtx_z.at(scl_ll_max));
      
      hmap_flsmatch_score["background"]->Fill(t->slc_flsmatch_score.at(scl_ll_max)/2);
    }
    
    if (isNueCCFV) {
      nue_cc_selected_total+=1;//t->bnb_weight;
      if (t->nu_e >= 0.05 && t->nu_e <= 1.5){
        nue_cc_selected_total_energy_range+=1;//t->bnb_weight;
        //std::cout << "Selected nue event, run " << t->run << ", " << t->subrun << ", " << t->event << ", index " << scl_ll_max << std::endl;
      }
    }
    if (isNue) {
      if (t->nu_e >= 0.05 && t->nu_e <= 1.5 && t->ccnc==0 
        && t->tvtx_x[0] > 0. && t->tvtx_x[0] < 256.35
        && t->tvtx_y[0] > -116.5 && t->tvtx_y[0] < 116.5
        && t->tvtx_z[0] > 0. && t->tvtx_y[0] < 1036.8){
        nue_selected_total_energy_range+=1;//t->bnb_weight;
        //std::cout << "event " << t->event << ", weight: " << t->bnb_weight << ", truepdg: " << true_pdg 
        //          << ", dqds: " << dqdx_calib 
                  //<< ", dqds(1): " << t->slc_muoncandidate_dqdx_v_trunc.at(scl_ll_max) * _gainCalib 
                  //<< ", dqds(0): " << t->slc_muoncandidate_dqdx_u_trunc.at(scl_ll_max) * _gainCalib 
        //          << ", length: " << t->slc_longesttrack_length.at(scl_ll_max) << std::endl;
        h_nue_selected_energy->Fill(t->nu_e, 1/*t->bnb_weight*/); 
        if (std::abs(true_pdg) == 11) n_nue_electron+=1;//t->bnb_weight;
        if (std::abs(true_pdg) == 2212) n_nue_proton+=1;//t->bnb_weight;
        if (std::abs(true_pdg) == 211) n_nue_pion+=1;//t->bnb_weight;
        //std::cout << "Selected nue event, run " << t->run << ", " << t->subrun << ", " << t->event << ", index " << scl_ll_max << std::endl;
      }
    }

    // Look at showers
    /*
    _s_nupdg = t->nupdg;
    _s_track_pdg = true_pdg;
    _s_tpcobj_origin = t->slc_origin.at(scl_ll_max);
    _s_flash_z = t->beamfls_z.at(flashInBeamSpill);

    double max_length = 1e9, max_length_index = -1;
    for (size_t s = 0; s < t->nslices; s++) {
      if (s == scl_ll_max) continue;
      if (t->slc_longestshower_length.at(s) == -9999) continue;
      if (std::abs(_s_flash_z - t->slc_longestshower_startz.at(s)) < max_length) {
        max_length = std::abs(_s_flash_z - t->slc_longestshower_startz.at(s));
        max_length_index = s;
      }
    }

    if (max_length_index > -1) {
      _s_shower_length = t->slc_longestshower_length.at(max_length_index);
      _s_shower_phi = t->slc_longestshower_phi.at(max_length_index);
      _s_shower_theta = t->slc_longestshower_theta.at(max_length_index);
      _s_shower_openangle = t->slc_longestshower_openangle.at(max_length_index);
      _s_shower_startx = t->slc_longestshower_startx.at(max_length_index);
      _s_shower_starty = t->slc_longestshower_starty.at(max_length_index);
      _s_shower_startz = t->slc_longestshower_startz.at(max_length_index);
    } else {
      _s_shower_length = -1;
      _s_shower_phi = -1;
      _s_shower_theta = -1;
      _s_shower_openangle = -1;
      _s_shower_startx = -1;
      _s_shower_starty = -1;
      _s_shower_startz = -1;
    }

    _shower_tree->Fill();
    */
    
    
    //if (nu_origin && isSignal && true_pdg!=13 && t->lep_costheta < -0.5)
      //std::cout << ">>>>> here we select rigth backward candidate, but not the muon, event " << t->event << std::endl;
    
    //
    // SIGNAL
    //
    if(nu_origin && isSignal /*&& true_pdg==13 && true_origin == 0*/) {
      
      //std::cout << "Is signal and is selected. event: " << t->event << std::endl;
      
      signal_sel ++;
      h_eff_num->Fill(t->nu_e);
      h_eff_mumom_num->Fill(t->true_muon_mom);
      h_eff_muangle_num->Fill(t->lep_costheta);
      h_eff_muangle_mumom_num->Fill(t->lep_costheta, t->true_muon_mom);
      h_eff_muphi_num->Fill(t->lep_phi);
      h_eff_mult_num->Fill(t->genie_mult);
      h_eff_mult_ch_num->Fill(t->genie_mult_ch);
      h_mu_eff_mom_sel->Fill(t->true_muon_mom, t->muon_reco_eff);

      if (t->mode == 0) {
        h_eff_qe_num->Fill(t->nu_e);
        signal_sel_qe++;
      }
      if (t->mode == 1) {
        h_eff_res_num->Fill(t->nu_e);
        signal_sel_res++;
      }
      if (t->mode == 2) {
        h_eff_dis_num->Fill(t->nu_e);
        signal_sel_dis++;
      }
      if (t->mode == 3) {
        h_eff_coh_num->Fill(t->nu_e);
        signal_sel_coh++;
      }
      if (t->mode == 10) {
        h_eff_mec_num->Fill(t->nu_e);
        signal_sel_mec++;
      }

      pEff->Fill(true, t->nu_e);
      hmap_trklen["signal"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
      hmap_trkmom["signal"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      hmap_trkphi["signal"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
      hmap_trktheta["signal"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
      hmap_multpfp["signal"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
      hmap_multtracktol["signal"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
      hmap_trktheta_trkmom["signal"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      
      if (t->slc_origin_extra.at(scl_ll_max) == 0) {
        hmap_trklen["signal_stopmu"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
        hmap_trkmom["signal_stopmu"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
        hmap_trkphi["signal_stopmu"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
        hmap_trktheta["signal_stopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
        hmap_multpfp["signal_stopmu"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
        hmap_multtracktol["signal_stopmu"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
        hmap_trktheta_trkmom["signal_stopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));

      } else {
        hmap_trklen["signal_nostopmu"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
        hmap_trkmom["signal_nostopmu"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
        hmap_trkphi["signal_nostopmu"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
        hmap_trktheta["signal_nostopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
        hmap_multpfp["signal_nostopmu"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
        hmap_multtracktol["signal_nostopmu"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
        hmap_trktheta_trkmom["signal_nostopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      }
    }
    //
    // ANUMU
    //
    else if(nu_origin && t->ccnc==0 && t->nupdg==-14 && t->fv==1){
      bkg_anumu_sel ++;
      pEff->Fill(false, t->nu_e);
      hmap_trklen["anumu"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
      hmap_trkmom["anumu"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      hmap_trkphi["anumu"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
      hmap_trktheta["anumu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
      hmap_multpfp["anumu"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
      hmap_multtracktol["anumu"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
      hmap_trktheta_trkmom["anumu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
    }
    //
    // NUE
    //
    else if(nu_origin && t->ccnc==0 && (t->nupdg==-12 || t->nupdg==12) && t->fv==1){
      bkg_nue_sel ++;
      pEff->Fill(false, t->nu_e);
      hmap_trklen["nue"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
      hmap_trkmom["nue"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      hmap_trkphi["nue"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
      hmap_trktheta["nue"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
      hmap_multpfp["nue"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
      hmap_multtracktol["nue"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
      hmap_trktheta_trkmom["nue"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      if (t->nupdg == 12)
        nue_cc_selected+=1;//t->bnb_weight;
    }
    //
    // NC
    //
    else if(nu_origin && t->ccnc==1 && t->fv==1){
      bkg_nc_sel ++;
      pEff->Fill(false, t->nu_e);
      hmap_trklen["nc"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
      hmap_trkmom["nc"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      hmap_trkphi["nc"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
      hmap_trktheta["nc"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
      hmap_multpfp["nc"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
      hmap_multtracktol["nc"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
      hmap_trktheta_trkmom["nc"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      // proton
      if (t->slc_origin_extra.at(scl_ll_max) == 3) {
        hmap_trklen["nc_proton"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
        hmap_trkmom["nc_proton"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
        hmap_trkphi["nc_proton"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
        hmap_trktheta["nc_proton"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
        hmap_multpfp["nc_proton"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
        hmap_multtracktol["nc_proton"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
        hmap_trktheta_trkmom["nc_proton"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      }
      // pion
      else if (t->slc_origin_extra.at(scl_ll_max) == 2) {
        hmap_trklen["nc_pion"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
        hmap_trkmom["nc_pion"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
        hmap_trkphi["nc_pion"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
        hmap_trktheta["nc_pion"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
        hmap_multpfp["nc_pion"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
        hmap_multtracktol["nc_pion"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
        hmap_trktheta_trkmom["nc_pion"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      }
      // other
      else {
        hmap_trklen["nc_other"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
        hmap_trkmom["nc_other"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
        hmap_trkphi["nc_other"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
        hmap_trktheta["nc_other"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
        hmap_multpfp["nc_other"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
        hmap_multtracktol["nc_other"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
        hmap_trktheta_trkmom["nc_other"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      }
    }
    //
    // OUTFV
    //
    else if(nu_origin && t->fv==0){
      bkg_outfv_sel ++;
      pEff->Fill(false, t->nu_e);
      hmap_trklen["outfv"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
      hmap_trkmom["outfv"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      hmap_trkphi["outfv"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
      hmap_trktheta["outfv"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
      hmap_multpfp["outfv"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
      hmap_multtracktol["outfv"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
      hmap_trktheta_trkmom["outfv"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      
      if (t->slc_origin_extra.at(scl_ll_max) == 0) {
        hmap_trklen["outfv_stopmu"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
        hmap_trkmom["outfv_stopmu"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
        hmap_trkphi["outfv_stopmu"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
        hmap_trktheta["outfv_stopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
        hmap_multpfp["outfv_stopmu"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
        hmap_multtracktol["outfv_stopmu"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
        hmap_trktheta_trkmom["outfv_stopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      } else {
        hmap_trklen["outfv_nostopmu"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
        hmap_trkmom["outfv_nostopmu"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
        hmap_trkphi["outfv_nostopmu"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
        hmap_trktheta["outfv_nostopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
        hmap_multpfp["outfv_nostopmu"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
        hmap_multtracktol["outfv_nostopmu"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
        hmap_trktheta_trkmom["outfv_nostopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      }
    }
    //
    // COSMIC
    //
    else{
      bkg_cosmic_sel ++;
      if (t->slc_crosses_top_boundary.at(scl_ll_max) == 1 )
        bkg_cosmic_top_sel++;
      pEff->Fill(false, t->nu_e);
      hmap_trklen["cosmic"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
      hmap_trkmom["cosmic"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      hmap_trkphi["cosmic"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
      hmap_trktheta["cosmic"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
      hmap_multpfp["cosmic"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
      hmap_multtracktol["cosmic"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
      hmap_trktheta_trkmom["cosmic"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      //std::cout << "Is a cosmic but is selected. event: " << t->event << std::endl;
      
      if (t->slc_origin_extra.at(scl_ll_max) == 0) {
        hmap_trklen["cosmic_stopmu"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
        hmap_trkmom["cosmic_stopmu"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
        hmap_trkphi["cosmic_stopmu"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
        hmap_trktheta["cosmic_stopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
        hmap_multpfp["cosmic_stopmu"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
        hmap_multtracktol["cosmic_stopmu"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
        hmap_trktheta_trkmom["cosmic_stopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      } else {
        hmap_trklen["cosmic_nostopmu"]->Fill(t->slc_longesttrack_length.at(scl_ll_max));
        hmap_trkmom["cosmic_nostopmu"]->Fill(t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
        hmap_trkphi["cosmic_nostopmu"]->Fill(t->slc_longesttrack_phi.at(scl_ll_max));
        hmap_trktheta["cosmic_nostopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max));
        hmap_multpfp["cosmic_nostopmu"]->Fill(t->slc_mult_pfp.at(scl_ll_max));
        hmap_multtracktol["cosmic_nostopmu"]->Fill(t->slc_mult_track_tolerance.at(scl_ll_max));
        hmap_trktheta_trkmom["cosmic_nostopmu"]->Fill(t->slc_longesttrack_theta.at(scl_ll_max), t->slc_muoncandidate_mom_mcs.at(scl_ll_max));
      }
    }
    
    
  } // end of event loop
  
  
  h_pot->SetBinContent(1, totalPOT);
  h_nevts->SetBinContent(1, total_events);


  
  cout << endl << endl << "********************************" << endl;
  
  
  
  
  // ************************
  //
  //  Printing
  //
  // ************************
  
  std::cout << "nsignal is " << nsignal << std::endl;
  int sel_tot = signal_sel + bkg_anumu_sel + bkg_nue_sel + bkg_nc_sel + bkg_outfv_sel + bkg_cosmic_sel;
  std::cout << "signal_sel is     " << signal_sel     << ", " << (double)signal_sel/(double)sel_tot * 100. << std::endl;
  std::cout << "bkg_anumu_sel is  " << bkg_anumu_sel  << ", " << (double)bkg_anumu_sel/(double)sel_tot * 100. << std::endl;
  std::cout << "bkg_nue_sel is    " << bkg_nue_sel    << ", " << (double)bkg_nue_sel/(double)sel_tot * 100. << std::endl;
  std::cout << "bkg_nc_sel is     " << bkg_nc_sel     << ", " << (double)bkg_nc_sel/(double)sel_tot * 100. << std::endl;
  std::cout << "bkg_outfv_sel is  " << bkg_outfv_sel  << ", " << (double)bkg_outfv_sel/(double)sel_tot * 100. << std::endl;
  std::cout << "bkg_cosmic_sel is " << bkg_cosmic_sel << ", " << (double)bkg_cosmic_sel/(double)sel_tot * 100. << std::endl << std::endl;
  
  std::cout << "Efficiency: " << signal_sel/(double)nsignal << std::endl;
  std::cout << "Purity: " << signal_sel/(double)(signal_sel+bkg_anumu_sel+bkg_nue_sel+bkg_nc_sel+bkg_outfv_sel+bkg_cosmic_sel) << std::endl;
  std::cout << "Cosmic contamination: " << bkg_cosmic_sel/(double)(bkg_anumu_sel+bkg_nue_sel+bkg_nc_sel+bkg_outfv_sel+bkg_cosmic_sel) << std::endl;
  std::cout << "  of which crossing top: " << bkg_cosmic_top_sel/(double)bkg_cosmic_sel << std::endl;
  std::cout << "NC contamination: " << bkg_nc_sel/(double)(bkg_anumu_sel+bkg_nue_sel+bkg_nc_sel+bkg_outfv_sel+bkg_cosmic_sel) << std::endl;
  std::cout << "OUTFV contamination: " << bkg_outfv_sel/(double)(bkg_anumu_sel+bkg_nue_sel+bkg_nc_sel+bkg_outfv_sel+bkg_cosmic_sel) << std::endl << std::endl;
  
  std::cout << "Efficiency QE: " << signal_sel_qe/(double)nsignal_qe << " +- " << eff_uncertainty(signal_sel_qe, nsignal_qe) << std::endl;
  std::cout << "Efficiency RES: " << signal_sel_res/(double)nsignal_res << " +- " << eff_uncertainty(signal_sel_res, nsignal_res) << std::endl;
  std::cout << "Efficiency COH: " << signal_sel_coh/(double)nsignal_coh << " +- " << eff_uncertainty(signal_sel_coh, nsignal_coh) << std::endl;
  std::cout << "Efficiency DIS: " << signal_sel_dis/(double)nsignal_dis << " +- " << eff_uncertainty(signal_sel_dis, nsignal_dis) << std::endl;
  std::cout << "Efficiency MEC: " << signal_sel_mec/(double)nsignal_mec << " +- " << eff_uncertainty(signal_sel_mec, nsignal_mec) << std::endl << std::endl;

  std::cout << "n events with a flash in the beam spill: " << nEvtsWFlashInBeamSpill << std::endl;
  std::cout << "n events numu CC (all voulumes): " << nNumuCC << std::endl;
  std::cout << " Signal events that have a recon muon: " << nSignalWMuonReco << std::endl;
  std::cout << " Signal events that have a recon muon and a recon vertex 10 cm close in YZ plane: " << nSignalMuonRecoVtxOk << std::endl << std::endl;
  
  std::cout << "Number of signal events that were correctly flash-matched: " << nSignalFlashMatched << std::endl << std::endl;
  
  std::cout << "Number of neutrino origin slices in total: " << n_slc_nu_origin << std::endl;
  std::cout << "Number of neutrino origin slices tagged as cosmic by the ACPT algo in total: " << n_slc_acpt_tag_nu << std::endl << std::endl;
  
  
  
  std::cout << "Number of simulated nue CC in FV: " << nue_cc_fv << std::endl;
  std::cout << "Number of selected nue CC in FV (as such):  " << nue_cc_selected << std::endl;
  std::cout << "Number of selected nue CC in FV (total):  " << nue_cc_selected_total << std::endl;
  std::cout << "Number of selected nue CC in FV in [0.05, 1.5] GeV (total):  " << nue_cc_selected_total_energy_range << std::endl;
  std::cout << "Number of selected nue in [0.05, 1.5] GeV (total):  " << nue_selected_total_energy_range << std::endl << std::endl << std::endl;
  //std::cout << "\t Ratio: " << (double)nue_cc_selected/(double)nue_cc_fv << std::endl;
  
  std::cout << "Number of selected nue where an electron is selected: " << n_nue_electron * 6.6e20/totalPOT<< std::endl;
  std::cout << "Number of selected nue where a proton is selected: " << n_nue_proton * 6.6e20/totalPOT << std::endl;
  std::cout << "Number of selected nue where a pion is selected: " << n_nue_pion * 6.6e20/totalPOT << std::endl;

  // ************************
  //
  //  Plotting
  //
  // ************************
  
  TString temp2;
  
  TCanvas * canvas_efficiency = new TCanvas();
  TEfficiency* pEff2 = new TEfficiency(*h_eff_num,*h_eff_den);
  pEff2->SetTitle(";True Neutrino Energy [GeV];Efficiency");
  pEff2->SetLineColor(kGreen+3);
  pEff2->SetMarkerColor(kGreen+3);
  pEff2->SetMarkerStyle(20);
  pEff2->SetMarkerSize(0.5);
  pEff2->Draw("AP");
  gPad->Update();
  auto g = pEff2->GetPaintedGraph();
  g->SetMinimum(0);
  g->SetMaximum(1);
  gPad->Update();
  
  temp2 = "./output/efficiency";
  canvas_efficiency->SaveAs(temp2 + ".pdf");
  canvas_efficiency->SaveAs(temp2 + ".C","C");
  
  TCanvas * canvas_muon_reco_efficiency = new TCanvas();
  TEfficiency* pEff3 = new TEfficiency(*h_mueff_num,*h_mueff_den);
  pEff3->SetTitle(";True Muon Momentum [GeV];Reconstruction Efficiency");
  pEff3->Draw("AP");
  
  temp2 = "./output/muon_reco_efficiency";
  canvas_muon_reco_efficiency->SaveAs(temp2 + ".pdf");
  canvas_muon_reco_efficiency->SaveAs(temp2 + ".C","C");

  TCanvas * canvas_muon_reco_efficiency_angle = new TCanvas();
  TEfficiency* pEff3_2 = new TEfficiency(*h_mueff_angle_num,*h_mueff_angle_den);
  pEff3_2->SetTitle(";True Muon cos(#theta);Reconstruction Efficiency");
  pEff3_2->Draw("AP");
  gPad->Update();
  g = pEff3_2->GetPaintedGraph();
  g->SetMinimum(0);
  g->SetMaximum(1);
  gPad->Update();
  
  temp2 = "./output/muon_reco_efficiency_angle";
  canvas_muon_reco_efficiency_angle->SaveAs(temp2 + ".pdf");
  canvas_muon_reco_efficiency_angle->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_efficiency_mumom = new TCanvas();
  TEfficiency* pEff4 = new TEfficiency(*h_eff_mumom_num,*h_eff_mumom_den);
  pEff4->SetTitle(";True Muon Momentum [GeV];Efficiency");
  pEff4->SetLineColor(kGreen+3);
  pEff4->SetMarkerColor(kGreen+3);
  pEff4->SetMarkerStyle(20);
  pEff4->SetMarkerSize(0.5);
  pEff4->Draw("AP");
  //gPad->Update();
  //auto g = pEff4->GetPaintedGraph();
  //g->SetMinimum(0);
  //g->SetMaximum(1);
  //gPad->Update();

  temp2 = "./output/efficiency_mumom";
  canvas_efficiency_mumom->SaveAs(temp2 + ".pdf");
  canvas_efficiency_mumom->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_efficiency_muangle = new TCanvas();
  TEfficiency* pEff5 = new TEfficiency(*h_eff_muangle_num,*h_eff_muangle_den);
  pEff5->SetTitle(";True Muon cos(#theta);Efficiency");
  pEff5->SetLineColor(kGreen+3);
  pEff5->SetMarkerColor(kGreen+3);
  pEff5->SetMarkerStyle(20);
  pEff5->SetMarkerSize(0.5);
  pEff5->Draw("AP");
  
  temp2 = "./output/efficiency_muangle";
  canvas_efficiency_muangle->SaveAs(temp2 + ".pdf");
  canvas_efficiency_muangle->SaveAs(temp2 + ".C","C");


  TCanvas * canvas_efficiency_muangle_mumom = new TCanvas();
  TEfficiency* pEff5_3 = new TEfficiency(*h_eff_muangle_mumom_num,*h_eff_muangle_mumom_den);
  pEff5_3->SetTitle("Efficiency;True Muon cos(#theta);True Muon Momentum [GeV]");
  pEff5_3->SetLineColor(kGreen+3);
  pEff5_3->SetMarkerColor(kGreen+3);
  pEff5_3->SetMarkerStyle(20);
  pEff5_3->SetMarkerSize(0.5);
  pEff5_3->Draw("colz");
  
  temp2 = "./output/efficiency_muangle_mumom";
  canvas_efficiency_muangle_mumom->SaveAs(temp2 + ".pdf");
  canvas_efficiency_muangle_mumom->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_efficiency_muphi = new TCanvas();
  TEfficiency* pEff5_2 = new TEfficiency(*h_eff_muphi_num,*h_eff_muphi_den);
  pEff5_2->SetTitle(";True Muon #phi angle;Efficiency");
  pEff5_2->SetLineColor(kGreen+3);
  pEff5_2->SetMarkerColor(kGreen+3);
  pEff5_2->SetMarkerStyle(20);
  pEff5_2->SetMarkerSize(0.5);
  pEff5_2->Draw("AP");
  
  temp2 = "./output/efficiency_muphi";
  canvas_efficiency_muphi->SaveAs(temp2 + ".pdf");
  canvas_efficiency_muphi->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_efficiency_mult = new TCanvas();
  TEfficiency* pEff6 = new TEfficiency(*h_eff_mult_num,*h_eff_mult_den);
  pEff6->SetTitle(";True GENIE Particle Multiplicity;Efficiency");
  pEff6->SetLineColor(kGreen+3);
  pEff6->SetMarkerColor(kGreen+3);
  pEff6->SetMarkerStyle(20);
  pEff6->SetMarkerSize(0.5);
  pEff6->Draw("AP");
  
  temp2 = "./output/efficiency_mult";
  canvas_efficiency_mult->SaveAs(temp2 + ".pdf");
  canvas_efficiency_mult->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_efficiency_mult_ch = new TCanvas();
  TEfficiency* pEff7 = new TEfficiency(*h_eff_mult_ch_num,*h_eff_mult_ch_den);
  pEff7->SetTitle(";True GENIE Charged Particle Multiplicity;Efficiency");
  pEff7->SetLineColor(kGreen+3);
  pEff7->SetMarkerColor(kGreen+3);
  pEff7->SetMarkerStyle(20);
  pEff7->SetMarkerSize(0.5);
  pEff7->Draw("AP");
  
  temp2 = "./output/efficiency_mult_ch";
  canvas_efficiency_mult_ch->SaveAs(temp2 + ".pdf");
  canvas_efficiency_mult_ch->SaveAs(temp2 + ".C","C");


  TCanvas * canvas_efficiency_mode = new TCanvas();
  TEfficiency* pEff_qe = new TEfficiency(*h_eff_qe_num,*h_eff_qe_den);
  pEff_qe->SetTitle(";True Neutrino Energy [GeV];Efficiency");
  pEff_qe->SetLineColor(kGreen+3);
  pEff_qe->SetMarkerColor(kGreen+3);
  pEff_qe->SetMarkerStyle(20);
  pEff_qe->SetMarkerSize(0.5);
  pEff_qe->Draw("ALP");
  gPad->Update();
  auto g_qe = pEff_qe->GetPaintedGraph();
  g_qe->SetMinimum(0);
  g_qe->SetMaximum(1);
  gPad->Update();

  TEfficiency* pEff_res = new TEfficiency(*h_eff_res_num,*h_eff_res_den);
  pEff_res->SetLineColor(kRed+3);
  pEff_res->SetMarkerColor(kRed+3);
  pEff_res->SetMarkerStyle(20);
  pEff_res->SetMarkerSize(0.5);
  pEff_res->Draw("LP same");

  TEfficiency* pEff_dis = new TEfficiency(*h_eff_dis_num,*h_eff_dis_den);
  pEff_dis->SetLineColor(kBlue+3);
  pEff_dis->SetMarkerColor(kBlue+3);
  pEff_dis->SetMarkerStyle(20);
  pEff_dis->SetMarkerSize(0.5);
  pEff_dis->Draw("LP same");

  TEfficiency* pEff_coh = new TEfficiency(*h_eff_coh_num,*h_eff_coh_den);
  pEff_coh->SetLineColor(kOrange-3);
  pEff_coh->SetMarkerColor(kOrange-3);
  pEff_coh->SetMarkerStyle(20);
  pEff_coh->SetMarkerSize(0.5);
  pEff_coh->Draw("LP same");

  TEfficiency* pEff_mec = new TEfficiency(*h_eff_mec_num,*h_eff_mec_den);
  pEff_mec->SetLineColor(kMagenta+1); 
  pEff_mec->SetMarkerColor(kMagenta+1);
  pEff_mec->SetMarkerStyle(20);
  pEff_mec->SetMarkerSize(0.5);
  pEff_mec->Draw("LP same");

  temp2 = "./output/efficiency_mode";
  canvas_efficiency_mode->SaveAs(temp2 + ".pdf");
  canvas_efficiency_mode->SaveAs(temp2 + ".C","C");

  
  /*TCanvas *c33 = new TCanvas();
  TEfficiency* pEff4 = new TEfficiency(*h_mueff_2_num,*h_mueff_den);
  pEff4->SetTitle(";True Muon Momentum [GeV];Reconstruction Efficiency");
  pEff4->SetLineColor(kRed);
  pEff4->Draw("P same");
   */
  
  
  TCanvas * canvas_chi2 = new TCanvas();
  h_chi2->Draw("histo");
  
  temp2 = "./output/chi2_mult";
  canvas_chi2->SaveAs(temp2 + ".pdf");
  canvas_chi2->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_flsTime = new TCanvas();
  h_flsTime->Draw("histo");
  
  temp2 = "./output/flsTime";
  canvas_flsTime->SaveAs(temp2 + ".pdf");
  canvas_flsTime->SaveAs(temp2 + ".C","C");
  
  new TCanvas();
  h_nslices->Draw("histo");
  
  new TCanvas();
  h_vtx_resolution->Draw("histo");
  
  new TCanvas();
  h_frac_diff->Draw("colz");
  
  new TCanvas();
  h_frac_diff_others->Draw("colz");
  
  // PE spec
  new TCanvas();
  TGraph* gr = new TGraph(32,hypo_spec_x,hypo_spec_y);
  TGraph* gr2 = new TGraph(32,meas_spec_x,meas_spec_y);
  TGraph* gr3 = new TGraph(32,numc_spec_x,numc_spec_y);
  gr->SetLineColor(kGreen+2);
  gr->SetLineWidth(2);
  gr->SetMarkerColor(kGreen+2);
  gr->SetMarkerSize(1.2);
  gr->SetMarkerStyle(20);
  gr->SetTitle("");
  gr->GetXaxis()->SetTitle("PMT ID");
  gr->GetYaxis()->SetTitle("PE Count");
  gr->Draw("ALP");
  gr2->SetLineColor(kBlue+2);
  gr2->SetLineWidth(2);
  gr2->SetMarkerColor(kBlue+2);
  gr2->SetMarkerSize(1.2);
  gr2->SetMarkerStyle(20);
  gr2->SetTitle("");
  gr2->GetXaxis()->SetTitle("PMT ID");
  gr2->GetYaxis()->SetTitle("PE Count");
  gr2->Draw("LP");
  gr3->SetLineColor(kRed+2);
  gr3->SetLineWidth(2);
  gr3->SetMarkerColor(kRed+2);
  gr3->SetMarkerSize(1.2);
  gr3->SetMarkerStyle(20);
  gr3->SetTitle("");
  gr3->GetXaxis()->SetTitle("PMT ID");
  gr3->GetYaxis()->SetTitle("PE Count");
  //gr3->Draw("LP");
  TLegend* leg = new TLegend(0.1,0.7,0.48,0.9);
  leg->AddEntry(gr,"Hypo flash","l");
  leg->AddEntry(gr2,"Reco flash","l");
  //leg->AddEntry(gr3,"Neutrino MCFlash","l");
  
  leg->Draw();
  
  TCanvas * canvas_vtxcheck = new TCanvas();
  h_vtxcheck_angle_good->Scale(1./h_vtxcheck_angle_good->Integral());
  h_vtxcheck_angle_bad->Scale(1./h_vtxcheck_angle_bad->Integral());
  h_vtxcheck_angle_good->Draw("histo");
  h_vtxcheck_angle_bad->Draw("histo same");
  h_vtxcheck_angle_bad->SetLineColor(kRed);

  TLegend* vtx_leg = new TLegend(0.1,0.7,0.48,0.9);
  vtx_leg->AddEntry(h_vtxcheck_angle_good,"Signal","l");
  vtx_leg->AddEntry(h_vtxcheck_angle_bad,"Background","l");
  vtx_leg->Draw();
  
  temp2 = "./output/vtxcheck";
  canvas_vtxcheck->SaveAs(temp2 + ".pdf");
  canvas_vtxcheck->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_mu_eff_mom = new TCanvas();
  h_mu_eff_mom->Draw("colz");
  
  temp2 = "./output/mu_eff_mom";
  canvas_mu_eff_mom->SaveAs(temp2 + ".pdf");
  canvas_mu_eff_mom->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_mu_eff_mom_sel = new TCanvas();
  h_mu_eff_mom_sel->Draw("colz");
  
  temp2 = "./output/mu_eff_mom_sel";
  canvas_mu_eff_mom_sel->SaveAs(temp2 + ".pdf");
  canvas_mu_eff_mom_sel->SaveAs(temp2 + ".C","C");

  
  new TCanvas();
  //h_muon_track_pur->Draw();
  h_mu_pur_mom->Draw("colz");
  
  new TCanvas();
  h_mumom_nue->Draw("colz");
  
  new TCanvas();
  h_acpt_tagged->Draw("histo");
  
  new TCanvas();
  h_xdiff->Draw("histo");
  h_xdiff_others->Draw("histo same");
  h_xdiff_others->SetLineColor(kRed);
  //TCanvas *c16 = new TCanvas();
  //h_xdiff_others->Draw("histo");
  
  new TCanvas();
  h_zdiff->Draw("histo");
  h_zdiff_others->Draw("histo same");
  h_zdiff_others->SetLineColor(kRed);
  
  
  new TCanvas();
  h_slice_origin->Draw("histo");
  
  new TCanvas();
  h_slice_npfp->DrawNormalized("histo");
  h_slice_npfp_others->DrawNormalized("histo same");
  h_slice_npfp_others->SetLineColor(kRed);
  
  new TCanvas();
  h_slice_ntrack->DrawNormalized("histo");
  h_slice_ntrack_others->DrawNormalized("histo same");
  h_slice_ntrack_others->SetLineColor(kRed);
  
  new TCanvas();
  h_fm_score->Draw("histo");
  h_fm_score_others->Draw("histo same");
  h_fm_score_others->SetLineColor(kRed);
  
  new TCanvas();
  h_n_slc_flsmatch->Draw("histo");
  
  new TCanvas();
  h_fm_score_pe->Draw("colz");
  
  
  TCanvas * final1 = new TCanvas();
  THStack *hs_trklen = new THStack("hs_trklen",";Candidate Track Length [cm]; Selected Events");
  if (_makePlots) DrawTHStack(hs_trklen, pot_scaling, _breakdownPlots, hmap_trklen);
  
  //
  // Construct legend
  // used basically for all plots
  //
  TLegend* leg2;
  if (_breakdownPlots){
    leg2 = new TLegend(0.56,0.37,0.82,0.82,NULL,"brNDC");
  } else {
    leg2 = new TLegend(0.56,0.54,0.82,0.82,NULL,"brNDC");
  }
  std::stringstream sstm;
  // numu
  if (_breakdownPlots) {
  leg2->AddEntry(hmap_trklen["signal_stopmu"],"#nu_{#mu} CC (stopping #mu)","f");
  leg2->AddEntry(hmap_trklen["signal_nostopmu"],"#nu_{#mu} CC (other)","f");
  } else {
    sstm << "#nu_{#mu} CC (signal), " << std::setprecision(2)  << hmap_trklen["signal"]->Integral() / hmap_trklen["total"]->Integral()*100. << "%";
    leg2->AddEntry(hmap_trklen["signal"],sstm.str().c_str(),"f");
    sstm.str("");
  }
  
  // nue
  sstm << "#nu_{e}, #bar{#nu}_{e} CC, " << std::setprecision(2)  << hmap_trklen["nue"]->Integral() / hmap_trklen["total"]->Integral()*100. << "%";
  leg2->AddEntry(hmap_trklen["nue"],sstm.str().c_str(),"f");
  sstm.str("");
  
  // anumu
  sstm << "#bar{#nu}_{#mu} CC, " << std::setprecision(2)  << hmap_trklen["anumu"]->Integral() / hmap_trklen["total"]->Integral()*100. << "%";
  leg2->AddEntry(hmap_trklen["anumu"],sstm.str().c_str(),"f");
  sstm.str("");
  
  // nc, outfv, cosmic
  if (_breakdownPlots) {
  leg2->AddEntry(hmap_trklen["nc_other"],"NC (other)","f");
  leg2->AddEntry(hmap_trklen["nc_pion"],"NC (pion)","f");
  leg2->AddEntry(hmap_trklen["nc_proton"],"NC (proton)","f");
  leg2->AddEntry(hmap_trklen["outfv_stopmu"],"OUTFV (stopping #mu)","f");
  leg2->AddEntry(hmap_trklen["outfv_nostopmu"],"OUTFV (other)","f");
  leg2->AddEntry(hmap_trklen["cosmic_stopmu"],"Cosmic (stopping #mu)","f");
  leg2->AddEntry(hmap_trklen["cosmic_nostopmu"],"Cosmic (other)","f");
  } else {
    sstm << "NC, " << std::setprecision(2)  << hmap_trklen["nc"]->Integral() / hmap_trklen["total"]->Integral()*100. << "%";
    leg2->AddEntry(hmap_trklen["nc"],sstm.str().c_str(),"f");
    sstm.str("");
    
    sstm << "OUTFV, " << std::setprecision(2)  << hmap_trklen["outfv"]->Integral() / hmap_trklen["total"]->Integral()*100. << "%";
    leg2->AddEntry(hmap_trklen["outfv"],sstm.str().c_str(),"f");
    sstm.str("");
    
    sstm << "Cosmic, " << std::setprecision(2)  << hmap_trklen["cosmic"]->Integral() / hmap_trklen["total"]->Integral()*100. << "%";
    leg2->AddEntry(hmap_trklen["cosmic"],sstm.str().c_str(),"f");
    sstm.str("");
  }
  leg2->AddEntry(hmap_trklen["total"],"MC Stat Unc.","f");
  leg2->Draw();
  DrawPOT2(totalPOT);
  
  temp2 = "./output/trklen";
  final1->SaveAs(temp2 + ".pdf");
  final1->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * final1_1 = new TCanvas();
  THStack *hs_trkmom = new THStack("hs_trkmom",";Reconstructed Momentum [GeV]; Selected Events");
  if (_makePlots) DrawTHStack(hs_trkmom, pot_scaling, _breakdownPlots, hmap_trkmom);
  leg2->Draw();
  DrawPOT2(totalPOT);
  
  temp2 = "./output/trkmom";
  final1_1->SaveAs(temp2 + ".pdf");
  final1_1->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * final2 = new TCanvas();
  THStack *hs_trkphi = new THStack("hs_trkphi",";Candidate Track #phi; Selected Events");
  if (_makePlots) DrawTHStack(hs_trkphi, pot_scaling, _breakdownPlots, hmap_trkphi);
  leg2->Draw();
  DrawPOT2(totalPOT);
  
  temp2 = "./output/trkphi";
  final2->SaveAs(temp2 + ".pdf");
  final2->SaveAs(temp2 + ".C","C");
  
  
  
  TCanvas * final3 = new TCanvas();
  THStack *hs_trktheta = new THStack("hs_trktheta",";Candidate Track cos(#theta); Selected Events");
  if (_makePlots) DrawTHStack(hs_trktheta, pot_scaling, _breakdownPlots, hmap_trktheta);
  leg2->Draw();
  DrawPOT2(totalPOT);
  
  temp2 = "./output/trktheta";
  final3->SaveAs(temp2 + ".pdf");
  final3->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * final4 = new TCanvas();
  THStack *hs_multpfp = new THStack("hs_multpfp",";PFP Multiplicity; Selected Events");
  if (_makePlots) DrawTHStack(hs_multpfp, pot_scaling, _breakdownPlots, hmap_multpfp);
  leg2->Draw();
  DrawPOT2(totalPOT);
  
  temp2 = "./output/multpfp";
  final4->SaveAs(temp2 + ".pdf");
  final4->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * final5 = new TCanvas();
  THStack *hs_multtracktol = new THStack("hs_multtracktol",";Track Multiplicity (5 cm); Selected Events");
  if (_makePlots) DrawTHStack(hs_multtracktol, pot_scaling, _breakdownPlots, hmap_multtracktol);
  leg2->Draw();
  DrawPOT2(totalPOT);
  
  temp2 = "./output/multtracktol";
  final5->SaveAs(temp2 + ".pdf");
  final5->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_dqdx = new TCanvas();
  THStack *hs_dqdx_trunc = new THStack("hs_dqdx_trunc",";Candidate Track <dQ/dx>_{trunc};Selected Events");
  if (_makePlots) DrawTHStack3(hs_dqdx_trunc, pot_scaling, _breakdownPlots, hmap_dqdx_trunc);
  
  temp2 = "./output/dqdx_trunc";
  canvas_dqdx->SaveAs(temp2 + ".pdf");
  canvas_dqdx->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_dqdx_length = new TCanvas();
  h_dqdx_trunc_length_muon->Draw("BOX");
  h_dqdx_trunc_length_muon->SetLineColor(kRed+2);
  h_dqdx_trunc_length_proton->Draw("BOX same");
  h_dqdx_trunc_length_proton->SetLineColor(kBlue+2);

  temp2 = "./output/dqdx_trunc_length";
  canvas_dqdx_length->SaveAs(temp2 + ".pdf");
  canvas_dqdx_length->SaveAs(temp2 + ".C","C");
  
  
  TCanvas * canvas_dqdx_length_muon = new TCanvas();
  h_dqdx_trunc_length_muon->Draw("colz");
  
  temp2 = "./output/dqdx_trunc_length_muon";
  canvas_dqdx_length_muon->SaveAs(temp2 + ".pdf");
  canvas_dqdx_length_muon->SaveAs(temp2 + ".C","C");

  
  TCanvas * canvas_dqdx_length_proton = new TCanvas();
  h_dqdx_trunc_length_proton->Draw("colz");
  
  temp2 = "./output/dqdx_trunc_length_proton";
  canvas_dqdx_length_proton->SaveAs(temp2 + ".pdf");
  canvas_dqdx_length_proton->SaveAs(temp2 + ".C","C");

  
  
  
  TCanvas * canvas_deltall_cosmic_stop = new TCanvas();
  h_deltall_cosmic_stop->Draw();
  
  temp2 = "./output/deltall_cosmic_stop";
  canvas_deltall_cosmic_stop->SaveAs(temp2 + ".pdf");
  canvas_deltall_cosmic_stop->SaveAs(temp2 + ".C","C");
  
  TCanvas * canvas_deltall_cosmic_nostop = new TCanvas();
  h_deltall_cosmic_nostop->Draw();
  
  temp2 = "./output/deltall_cosmic_nostop";
  canvas_deltall_cosmic_nostop->SaveAs(temp2 + ".pdf");
  canvas_deltall_cosmic_nostop->SaveAs(temp2 + ".C","C");

  TCanvas * canvas_deltall_nu = new TCanvas();
  h_deltall_nu->Draw();
  
  temp2 = "./output/deltall_nu";
  canvas_deltall_nu->SaveAs(temp2 + ".pdf");
  canvas_deltall_nu->SaveAs(temp2 + ".C","C");

  
  
  
  TCanvas * canvas_deltall_length_cosmic_stop = new TCanvas();
  h_deltall_length_cosmic_stop->Draw("colz");
  
  temp2 = "./output/deltall_cosmic_stop_length";
  canvas_deltall_length_cosmic_stop->SaveAs(temp2 + ".pdf");
  canvas_deltall_length_cosmic_stop->SaveAs(temp2 + ".C","C");
  
  TCanvas * canvas_deltall_length_cosmic_nostop = new TCanvas();
  h_deltall_length_cosmic_nostop->Draw("colz");
  
  temp2 = "./output/deltall_cosmic_nostop_length";
  canvas_deltall_length_cosmic_nostop->SaveAs(temp2 + ".pdf");
  canvas_deltall_length_cosmic_nostop->SaveAs(temp2 + ".C","C");
  
  TCanvas * canvas_deltall_length_nu = new TCanvas();
  h_deltall_length_nu->Draw("colz");
  
  temp2 = "./output/deltall_nu_length";
  canvas_deltall_length_nu->SaveAs(temp2 + ".pdf");
  canvas_deltall_length_nu->SaveAs(temp2 + ".C","C");
  
  
  
  
  
  TCanvas * canvas_nue_flash = new TCanvas();
  h_true_nu_eng_beforesel->SetLineColor(kBlue+2);
  h_true_nu_eng_beforesel->Draw();
  h_true_nu_eng_afterflash->SetLineColor(kRed+2);
  h_true_nu_eng_afterflash->Draw("same");
  h_true_nu_eng_aftersel->SetLineColor(kGreen+2);
  h_true_nu_eng_aftersel->Draw("same");
  TLegend* l01 = new TLegend(0.1,0.7,0.48,0.9);
  l01->AddEntry(h_true_nu_eng_beforesel,"Generated CC #nu_{#mu} events in FV","l");
  l01->AddEntry(h_true_nu_eng_afterflash,"CC #nu_{#mu} Passing Flash Finding","l");
  l01->AddEntry(h_true_nu_eng_aftersel,"Selected CC #nu_{#mu} events","l");
  l01->Draw();
  temp2 = "./output/nue_flash";
  canvas_nue_flash->SaveAs(temp2 + ".pdf");
  canvas_nue_flash->SaveAs(temp2 + ".C","C");


  TCanvas * canvas_nue_selected = new TCanvas();
  std::cout << "int: " <<h_nue_selected_energy->Integral() << std::endl;
  h_nue_selected_energy->Scale(6.6e20/totalPOT);
  std::cout << "int: " <<h_nue_selected_energy->Integral() << std::endl;
  h_nue_selected_energy->Draw("histo");
  DrawPOT2(totalPOT, 6.6e20);
  temp2 = "./output/nue_selected_contamination";
  canvas_nue_selected->SaveAs(temp2 + ".pdf");
  canvas_nue_selected->SaveAs(temp2 + ".C","C");



  // Efficiency for every cut

  TH1D * selected_percut = new TH1D("selected_percut", "selected_percut", 9, 0, 9);
  TH1D * selected_signal_percut = new TH1D("selected_signal_percut", "selected_percut", 9, 0, 9);
  //TH1D * generated_percut = new TH1D("generated_percut", "generated_percut", 8, 0, 7);
  TH1D * generated_signal_percut = new TH1D("generated_signal_percut", "generated_percut", 9, 0, 9);

  selected_percut->SetBinContent(1, selected_events_percut["initial"]);
  selected_percut->SetBinContent(2, selected_events_percut["beamflash"]);
  selected_percut->SetBinContent(3, selected_events_percut["flash_match"]);
  selected_percut->SetBinContent(4, selected_events_percut["flash_match_deltax"]);
  selected_percut->SetBinContent(5, selected_events_percut["flash_match_deltaz"]);
  selected_percut->SetBinContent(6, selected_events_percut["quality"]);
  selected_percut->SetBinContent(7, selected_events_percut["mcs_length_quality"]);
  selected_percut->SetBinContent(8, selected_events_percut["mip_consistency"]);
  selected_percut->SetBinContent(9, selected_events_percut["fiducial_volume"]);

  selected_signal_percut->SetBinContent(1, selected_signal_events_percut["initial"]);
  selected_signal_percut->SetBinContent(2, selected_signal_events_percut["beamflash"]);
  selected_signal_percut->SetBinContent(3, selected_signal_events_percut["flash_match"]);
  selected_signal_percut->SetBinContent(4, selected_signal_events_percut["flash_match_deltax"]);
  selected_signal_percut->SetBinContent(5, selected_signal_events_percut["flash_match_deltaz"]);
  selected_signal_percut->SetBinContent(6, selected_signal_events_percut["quality"]);
  selected_signal_percut->SetBinContent(7, selected_signal_events_percut["mcs_length_quality"]);
  selected_signal_percut->SetBinContent(8, selected_signal_events_percut["mip_consistency"]);
  selected_signal_percut->SetBinContent(9, selected_signal_events_percut["fiducial_volume"]);

  for (int i = 0; i < 9; i++) {
    std::cout << "cut " << i << " => " << selected_percut->GetBinContent(i+1) << std::endl;
    generated_signal_percut->SetBinContent(i+1, (double)nsignal);
    //generated_percut->SetBinContent(i+1, (double)sel_tot);
  }

  TCanvas * canvas_eff_pur_graph_percut = new TCanvas();

  TH1F *h = new TH1F("h","",9, 0, 9);
  h->SetMaximum(1);
  h->GetXaxis()->SetBinLabel(1,"initial");
  h->GetXaxis()->SetBinLabel(2,"beamflash");
  h->GetXaxis()->SetBinLabel(3,"flash_match");
  h->GetXaxis()->SetBinLabel(4,"flash_match_deltax");
  h->GetXaxis()->SetBinLabel(5,"flash_match_deltaz");
  h->GetXaxis()->SetBinLabel(6,"quality");
  h->GetXaxis()->SetBinLabel(7,"mcs_length_quality");
  h->GetXaxis()->SetBinLabel(8,"mip_consistency");
  h->GetXaxis()->SetBinLabel(9,"fiducial_volume");
  h->Draw();

  TEfficiency* pEff_percut = new TEfficiency(*selected_signal_percut,*generated_signal_percut);
  pEff_percut->SetTitle("EfficiencyPerCut;Cut index;Efficiency");
  pEff_percut->SetLineColor(kGreen+3);
  pEff_percut->SetMarkerColor(kGreen+3);
  pEff_percut->SetMarkerStyle(20);
  pEff_percut->SetMarkerSize(0.6);
  TGraphAsymmErrors * pEff_percut_graph = pEff_percut->CreateGraph();
  for (int i = 0; i < 9; i++) {
    pEff_percut_graph->SetPointEXhigh(i, 0.);
    pEff_percut_graph->SetPointEXlow(i, 0.);
  }
  auto axis = pEff_percut_graph->GetYaxis();
  axis->SetLimits(0.,1.); 

  pEff_percut_graph->GetHistogram()->GetXaxis()->SetBinLabel(1,"initial");
  pEff_percut_graph->GetHistogram()->GetXaxis()->SetBinLabel(2,"beamflash");
  pEff_percut_graph->GetHistogram()->GetXaxis()->SetBinLabel(3,"flash_match");
  pEff_percut_graph->GetHistogram()->GetXaxis()->SetBinLabel(4,"flash_match_deltax");
  pEff_percut_graph->GetHistogram()->GetXaxis()->SetBinLabel(5,"flash_match_deltaz");
  pEff_percut_graph->GetHistogram()->GetXaxis()->SetBinLabel(6,"quality");
  pEff_percut_graph->GetHistogram()->GetXaxis()->SetBinLabel(7,"mcs_length_quality");
  pEff_percut_graph->GetHistogram()->GetXaxis()->SetBinLabel(8,"mip_consistency");
  pEff_percut_graph->GetHistogram()->GetXaxis()->SetBinLabel(9,"fiducial_volume");

  pEff_percut_graph->Draw("PL");

  

  TEfficiency* pPur_percut = new TEfficiency(*selected_signal_percut,*selected_percut);
  pPur_percut->SetTitle("PurityPerCut;Cut index;Purity");
  pPur_percut->SetLineColor(kRed+3);
  pPur_percut->SetMarkerColor(kRed+3);
  pPur_percut->SetMarkerStyle(20);
  pPur_percut->SetMarkerSize(0.6);
  TGraphAsymmErrors * pPur_percut_graph = pPur_percut->CreateGraph();
  for (int i = 0; i < 9; i++) {
    pPur_percut_graph->SetPointEXhigh(i, 0.);
    pPur_percut_graph->SetPointEXlow(i, 0.);
  }
  pPur_percut_graph->Draw("PL");

  TLegend* l = new TLegend(0.6002865,0.2105263,0.8939828,0.3157895,NULL,"brNDC");
  l->AddEntry(pEff_percut_graph,"Efficiency");
  l->AddEntry(pPur_percut_graph,"Purity");
  //leg->AddEntry(gr3,"Neutrino MCFlash","l");
  
  l->Draw();

  temp2 = "./output/eff_pur_graph_percut";
  canvas_eff_pur_graph_percut->SaveAs(temp2 + ".pdf");
  canvas_eff_pur_graph_percut->SaveAs(temp2 + ".C","C");

  
  
  //
  // Save on file
  //
  
  file_out->cd();
  for (auto iter : hmap_trklen) {
    iter.second->Write();
  }
  h_pot->Write();
  h_nevts->Write();

  pEff4->Write();
  pEff5->Write();
  pEff5_3->Write();
  h_truth_xsec_mumom->Write();
  h_truth_xsec_muangle->Write();

  pEff_percut->Write();
  pPur_percut->Write();

  file_out->WriteObject(&hmap_trklen, "hmap_trklen");
  file_out->WriteObject(&hmap_trkmom, "hmap_trkmom");
  file_out->WriteObject(&hmap_trktheta, "hmap_trktheta");
  file_out->WriteObject(&hmap_trkphi, "hmap_trkphi");
  file_out->WriteObject(&hmap_multpfp, "hmap_multpfp");
  file_out->WriteObject(&hmap_multtracktol, "hmap_multtracktol");
  file_out->WriteObject(&hmap_trktheta_trkmom, "hmap_trktheta_trkmom");

  file_out->WriteObject(&hmap_vtxcheck_angle, "hmap_vtxcheck_angle");
  file_out->WriteObject(&hmap_residuals_std, "hmap_residuals_std");
  file_out->WriteObject(&hmap_residuals_mean, "hmap_residuals_mean");
  file_out->WriteObject(&hmap_perc_used_hits, "hmap_perc_used_hits");
  file_out->WriteObject(&hmap_mom_mcs_length, "hmap_mom_mcs_length");

  file_out->WriteObject(&hmap_xdiff_b, "hmap_xdiff_b");
  file_out->WriteObject(&hmap_zdiff_b, "hmap_zdiff_b");
  file_out->WriteObject(&hmap_xdiff, "hmap_xdiff");
  file_out->WriteObject(&hmap_zdiff, "hmap_zdiff");
  file_out->WriteObject(&hmap_pediff, "hmap_pediff");
  
  file_out->WriteObject(&hmap_vtxx_b, "hmap_vtxx_b");
  file_out->WriteObject(&hmap_vtxx, "hmap_vtxx");
  file_out->WriteObject(&hmap_vtxy, "hmap_vtxy");
  file_out->WriteObject(&hmap_vtxz, "hmap_vtxz");
  h_vtx_xz->Write();
  h_vtx_xy->Write();
  
  file_out->WriteObject(&hmap_dqdx_trunc, "hmap_dqdx_trunc");
  h_dqdx_trunc_length->Write();
  
  file_out->WriteObject(&hmap_ntpcobj, "hmap_ntpcobj");

  file_out->WriteObject(&hmap_flsmatch_score, "hmap_flsmatch_score");

  h_flsTime->Write();
  h_flsTime_wcut->Write();
  h_flsTime_wcut_2->Write();
  h_flsTime_wcut_3->Write();
  h_flsTime_wcut_4->Write();
  h_flsTime_wcut_5->Write();
  h_flsTime_wcut_6->Write();
  h_flsTime_wcut_7->Write();
  h_flsTime_wcut_8->Write();
  
  h_flsPe_wcut->Write();
  h_flsTime_flsPe_wcut->Write();
  
  h_deltax->Write();
  h_deltax_2d->Write();
  h_deltaz_4->Write();
  h_deltaz_6->Write();

  _mom_tree->Write();
  h_true_reco_mom->Write();
  h_true_reco_costheta->Write();

  file_out->Write();
  
  file_out->Close();
  
  
  

  
  
  // Computing time
  clock_t end = clock();
  double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
  cout << endl << endl << "Computing time: " << elapsed_secs << " seconds = " << elapsed_secs/60. << " minutes." << endl << endl;
  
  //rootapp->Run();
  //rootapp->Terminate(0);
  
  return 0;
}








