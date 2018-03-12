#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <TRandom1.h>
#include <TSystem.h>
#include <TBrowser.h>
#include <TApplication.h>
#include <TChain.h>
#include "TThread.h"
#include "THStack.h"
#include "TLegend.h"
#include "TEfficiency.h"
#include "TGraph.h"
#include <TString.h>
#include <TROOT.h>
#include <TStyle.h>

#include "AnaTree.h"
#include "Spectrum.hpp"
#include "Spectrum2D.hpp"
#include "PlottingTools.h"
#include "PlotHandler.h"
#include "MigrationMatrix2D.h"
#include "MigrationMatrix4D.h"
#include "CrossSectionCalculator2D.h"


//const bool _breakdownPlots = true;
//const double targetPOT = 4.95e19;


using namespace std;




//____________________________________________________________________________________________________
//____________________________________________________________________________________________________
//____________________________________________________________________________________________________
int main(int argc, char* argv[]) {
  
  clock_t begin = clock();
  
  std::string mc_bnbcosmic_file_name     = "ubxsecana_output.root";
  std::string mc_intimecosmic_file_name     = "ubxsecana_output.root";
  std::string bnbon_file_name  = "ubxsecana_output.root";
  std::string extbnb_file_name = "ubxsecana_output.root";
  double bnbon_pot_meas        = -1;
  double bnbon_triggers        = -1;
  double extbnb_triggers       = -1;

  
  int opt;
  while ((opt = getopt (argc, argv, "m:c:b:e:p:n:g:")) != -1)
  {
    switch (opt)
    {
      case 'm':
        std::cout << "MC BNBCosmic file: " << optarg << std::endl;
        mc_bnbcosmic_file_name = optarg;
        break;
      case 'c':
        std::cout << "MC InTime Cosmic file: " << optarg << std::endl;
        mc_intimecosmic_file_name = optarg;
        break;
      case 'b':
        std::cout << "Data BNBON file: " << optarg << std::endl;
        bnbon_file_name = optarg;
        break;
      case 'e':
        std::cout << "Data EXTBNB file: " << optarg << std::endl;
        extbnb_file_name = optarg;
        break;
      case 'p':
        std::cout << "BNBON POT: " << optarg << std::endl;
        bnbon_pot_meas = atof(optarg);
        break;
      case 'n':
        std::cout << "BNBON Triggers: " << optarg << std::endl;
        bnbon_triggers = atof(optarg);
        break;
      case 'g':
        std::cout << "EXTBNB Triggers: " << optarg << std::endl;
        extbnb_triggers = atof(optarg);
        break;
      default:
        std::cerr << "Usage: " << argv[0] << " [-m mc bnbcosmic file] [-c mc intimecosmic file] [-b bnbon data file] [-e extbnb data file] [-n bnbon triggers] [-g extbnb triggers] [-p bnbon pot]" << std::endl;
        exit(EXIT_FAILURE);
    }
    
  }
  
  if (bnbon_pot_meas < 0) {
    std::cerr << "Provide POT for the BNBON sample." << std::endl;
    exit(EXIT_FAILURE);
  }
  

  // Reset input in order to use TApplication
  char* temp[1] = {argv[0]};
  argc = 1;
  argv = temp;

  std::string env = std::getenv("UBXSecAnaRoot");

  TApplication* rootapp = new TApplication("ROOT Application",&argc, argv);
  //gROOT->SetBatch(kTRUE);
  gROOT->ProcessLine("gErrorIgnoreLevel = 2001;"); // 1001: INFO, 2001: WARNINGS, 3001: ERRORS
  gROOT->ProcessLine(".x ~/rootlogon.C");

  std::string library = ".L " + env + "loader_C.so";
  gROOT->ProcessLine(library.c_str());
  
  int bnbon_total_events = 1000;
  int extbnb_total_events = 1000;
  //int intimecosmic_total_events = 1000;
  int bnbcosmic_total_events = 1000;
  double mc_pot_sim = 6.e19;
  
  // *************************************
  // Opening files
  // *************************************
  TFile* mc_bnbcosmic_file = TFile::Open(mc_bnbcosmic_file_name.c_str(), "READ");
  //TFile* mc_intimecosmic_file = TFile::Open(mc_intimecosmic_file_name.c_str(), "READ");
  TFile* bnbon_file = TFile::Open(bnbon_file_name.c_str(), "READ");
  TFile* extbnb_file = TFile::Open(extbnb_file_name.c_str(), "READ");
  
  
  // *************************************
  // Getting number of events for bnbon
  // *************************************
  TH1D* h_nevts_bnbon = (TH1D*)bnbon_file->Get("h_nevts");
  bnbon_total_events = h_nevts_bnbon->GetBinContent(1);
  std::cout << "Number of events (BNBON):  " << bnbon_total_events << std::endl;
  
  
  // *************************************
  // Getting number of events for extbnb
  // *************************************
  TH1D* h_nevts_extbnb = (TH1D*)extbnb_file->Get("h_nevts");
  extbnb_total_events = h_nevts_extbnb->GetBinContent(1);
  std::cout << "Number of events (EXTBNB): " << extbnb_total_events << std::endl << std::endl;
  
  // *************************************
  // Getting number of events for bnbcosmic
  // *************************************
  TH1D* h_nevts_bnbcosmic = (TH1D*)mc_bnbcosmic_file->Get("h_nevts");
  bnbcosmic_total_events = h_nevts_bnbcosmic->GetBinContent(1);
  std::cout << "Number of events (BNBCosmic): " << bnbcosmic_total_events << std::endl;
  
  // *************************************
  // Getting number of events for intimecosmic
  // *************************************
  //TH1D* h_nevts_intimecosmic = (TH1D*)mc_intimecosmic_file->Get("h_nevts");
  //intimecosmic_total_events = h_nevts_intimecosmic->GetBinContent(1);
  //std::cout << "Number of events (InTimeCosmic): " << intimecosmic_total_events << std::endl;
  
  std::cout << std::endl;
  
  
  // *************************************
  // Getting simulated POT
  // *************************************
  TH1D* h_simpot = (TH1D*)mc_bnbcosmic_file->Get("h_pot");
  mc_pot_sim = h_simpot->GetBinContent(1);
  std::cout << "Simulated POT:      " << mc_pot_sim << std::endl;
  std::cout << "BNBON Measured POT: " << bnbon_pot_meas << std::endl << std::endl;
  
  
  // *************************************
  // Calculating scale factors
  // *************************************
  double scale_factor_extbnb = (double)bnbon_triggers/(double)extbnb_triggers;// 0.7432886072; //0.7937414399; //0.78661374; //0.278463;//1.23 * (382718./(double)extbnb_total_events) * ((double)bnbon_total_events/547616.);
  //double scale_factor_extbnb = 10378764. / 8243750.;
  double scale_factor_bnbon = 1.; //bnbon_pot_meas * bnbon_pot_target;
  double scale_factor_mc_bnbcosmic = bnbon_pot_meas / mc_pot_sim;
  //double scale_factor_mc_intimecosmic = ((double)bnbcosmic_total_events / (double)intimecosmic_total_events);
  //std::cout << "(1) INTIMECOSMIC: " << scale_factor_mc_intimecosmic << std::endl;
  //scale_factor_mc_intimecosmic *= 10.279;
  //std::cout << "(2) INTIMECOSMIC: " << scale_factor_mc_intimecosmic << std::endl;
  //scale_factor_mc_intimecosmic *= bnbon_pot_meas / mc_pot_sim;
  //std::cout << "(3) INTIMECOSMIC: " << scale_factor_mc_intimecosmic << std::endl;
  std::cout << "Data Scale Factors:" << std::endl;
  std::cout << "\t BNBON: " << scale_factor_bnbon << std::endl;
  std::cout << "\t EXTBNB: " << scale_factor_extbnb << std::endl;
  std::cout << "MC Scale Factors:" << std::endl;
  std::cout << "\t BNBCOSMIC: " << scale_factor_mc_bnbcosmic << std::endl;
  //std::cout << "\t INTIMECOSMIC: " << scale_factor_mc_intimecosmic << std::endl;

  
  // *************************************
  // Getting the relevant histograms
  // *************************************
  std::map<std::string,TH1D*>* temp_map;
  mc_bnbcosmic_file->GetObject("hmap_trklen", temp_map);
  std::map<std::string,TH1D*> hmap_trklen_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_trkmom", temp_map);
  std::map<std::string,TH1D*> hmap_trkmom_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_trktheta", temp_map);
  std::map<std::string,TH1D*> hmap_trktheta_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_trkphi", temp_map);
  std::map<std::string,TH1D*> hmap_trkphi_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_multpfp", temp_map);
  std::map<std::string,TH1D*> hmap_multpfp_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_multtracktol", temp_map);
  std::map<std::string,TH1D*> hmap_multtracktol_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_xdiff_b", temp_map);
  std::map<std::string,TH1D*> hmap_xdiff_b_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_zdiff_b", temp_map);
  std::map<std::string,TH1D*> hmap_zdiff_b_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_xdiff", temp_map);
  std::map<std::string,TH1D*> hmap_xdiff_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_zdiff", temp_map);
  std::map<std::string,TH1D*> hmap_zdiff_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_vtxx", temp_map);
  std::map<std::string,TH1D*> hmap_vtxx_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_vtxy", temp_map);
  std::map<std::string,TH1D*> hmap_vtxy_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_vtxz", temp_map);
  std::map<std::string,TH1D*> hmap_vtxz_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_flsmatch_score", temp_map);
  std::map<std::string,TH1D*> hmap_flsmatch_score_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_ntpcobj", temp_map);
  std::map<std::string,TH1D*> hmap_ntpcobj_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_vtxcheck_angle", temp_map);
  std::map<std::string,TH1D*> hmap_vtxcheck_angle_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_residuals_std", temp_map);
  std::map<std::string,TH1D*> hmap_residuals_std_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_residuals_mean", temp_map);
  std::map<std::string,TH1D*> hmap_residuals_mean_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_perc_used_hits", temp_map);
  std::map<std::string,TH1D*> hmap_perc_used_hits_mc = *temp_map;
  mc_bnbcosmic_file->GetObject("hmap_mom_mcs_length", temp_map);
  std::map<std::string,TH1D*> hmap_mom_mcs_length_mc = *temp_map;

  
  mc_bnbcosmic_file->GetObject("hmap_dqdx_trunc", temp_map);
  std::map<std::string,TH1D*> hmap_dqdx_trunc_mc = *temp_map;
  TH2D* h_dqdx_trunc_length_mc = (TH2D*)mc_bnbcosmic_file->Get("h_dqdx_trunc_length");
  
  //TH1D* h_trklen_intimecosmic = (TH1D*)mc_intimecosmic_file->Get("h_trklen_total");
  //hmap_trklen_mc["intimecosmic"] = NULL;
  
  TH1D* h_trklen_total_bnbon = (TH1D*)bnbon_file->Get("h_trklen_total");
  TH1D* h_trklen_total_extbnb = (TH1D*)extbnb_file->Get("h_trklen_total");
  TH1D* h_trkmom_total_bnbon = (TH1D*)bnbon_file->Get("h_trkmom_total");
  TH1D* h_trkmom_total_extbnb = (TH1D*)extbnb_file->Get("h_trkmom_total");
  TH1D* h_trktheta_total_bnbon = (TH1D*)bnbon_file->Get("h_trktheta_total");
  TH1D* h_trktheta_total_extbnb = (TH1D*)extbnb_file->Get("h_trktheta_total");
  TH1D* h_trkphi_total_bnbon = (TH1D*)bnbon_file->Get("h_trkphi_total");
  TH1D* h_trkphi_total_extbnb = (TH1D*)extbnb_file->Get("h_trkphi_total");
  TH1D* h_multpfp_total_bnbon = (TH1D*)bnbon_file->Get("h_multpfp_total");
  TH1D* h_multpfp_total_extbnb = (TH1D*)extbnb_file->Get("h_multpfp_total");
  TH1D* h_multtracktol_total_bnbon = (TH1D*)bnbon_file->Get("h_multtracktol_total");
  TH1D* h_multtracktol_total_extbnb = (TH1D*)extbnb_file->Get("h_multtracktol_total");
  TH1D* h_xdiff_b_total_bnbon = (TH1D*)bnbon_file->Get("h_xdiff_total_b");
  TH1D* h_xdiff_b_total_extbnb = (TH1D*)extbnb_file->Get("h_xdiff_total_b");
  TH1D* h_zdiff_b_total_bnbon = (TH1D*)bnbon_file->Get("h_zdiff_total_b");
  TH1D* h_zdiff_b_total_extbnb = (TH1D*)extbnb_file->Get("h_zdiff_total_b");
  TH1D* h_xdiff_total_bnbon = (TH1D*)bnbon_file->Get("h_xdiff_total");
  TH1D* h_xdiff_total_extbnb = (TH1D*)extbnb_file->Get("h_xdiff_total");
  TH1D* h_zdiff_total_bnbon = (TH1D*)bnbon_file->Get("h_zdiff_total");
  TH1D* h_zdiff_total_extbnb = (TH1D*)extbnb_file->Get("h_zdiff_total");
  TH1D* h_vtxx_total_bnbon = (TH1D*)bnbon_file->Get("h_vtxx_total");
  TH1D* h_vtxx_total_extbnb = (TH1D*)extbnb_file->Get("h_vtxx_total");
  TH1D* h_vtxy_total_bnbon = (TH1D*)bnbon_file->Get("h_vtxy_total");
  TH1D* h_vtxy_total_extbnb = (TH1D*)extbnb_file->Get("h_vtxy_total");
  TH1D* h_vtxz_total_bnbon = (TH1D*)bnbon_file->Get("h_vtxz_total");
  TH1D* h_vtxz_total_extbnb = (TH1D*)extbnb_file->Get("h_vtxz_total");
  TH1D* h_flsmatch_score_total_bnbon = (TH1D*)bnbon_file->Get("h_flsmatch_score_total");
  TH1D* h_flsmatch_score_total_extbnb = (TH1D*)extbnb_file->Get("h_flsmatch_score_total");
  TH1D* h_ntpcobj_total_bnbon = (TH1D*)bnbon_file->Get("h_ntpcobj_total");
  TH1D* h_ntpcobj_total_extbnb = (TH1D*)extbnb_file->Get("h_ntpcobj_total");
  TH1D* h_vtxcheck_angle_total_bnbon = (TH1D*)bnbon_file->Get("h_vtxcheck_angle_total");
  TH1D* h_vtxcheck_angle_total_extbnb = (TH1D*)extbnb_file->Get("h_vtxcheck_angle_total");
  TH1D* h_residuals_std_total_bnbon = (TH1D*)bnbon_file->Get("h_residuals_std_total");
  TH1D* h_residuals_std_total_extbnb = (TH1D*)extbnb_file->Get("h_residuals_std_total");
  TH1D* h_residuals_mean_total_bnbon = (TH1D*)bnbon_file->Get("h_residuals_mean_total");
  TH1D* h_residuals_mean_total_extbnb = (TH1D*)extbnb_file->Get("h_residuals_mean_total");
  TH1D* h_perc_used_hits_total_bnbon = (TH1D*)bnbon_file->Get("h_perc_used_hits_total");
  TH1D* h_perc_used_hits_total_extbnb = (TH1D*)extbnb_file->Get("h_perc_used_hits_total");
  TH1D* h_mom_mcs_length_total_bnbon = (TH1D*)bnbon_file->Get("h_mom_mcs_length_total");
  TH1D* h_mom_mcs_length_total_extbnb = (TH1D*)extbnb_file->Get("h_mom_mcs_length_total");

  TH1D* h_dqdx_trunc_total_bnbon = (TH1D*)bnbon_file->Get("h_dqdx_trunc_total");
  TH1D* h_dqdx_trunc_total_extbnb = (TH1D*)extbnb_file->Get("h_dqdx_trunc_total");
  TH2D* h_dqdx_trunc_length_bnbon = (TH2D*)bnbon_file->Get("h_dqdx_trunc_length");
  TH2D* h_dqdx_trunc_length_extbnb = (TH2D*)extbnb_file->Get("h_dqdx_trunc_length");
  
  TH1D* h_deltax_2d_mc = (TH1D*)mc_bnbcosmic_file->Get("h_deltax_2d");
  TH1D* h_deltax_2d_bnbon = (TH1D*)bnbon_file->Get("h_deltax_2d");
  TH1D* h_deltax_2d_extbnb = (TH1D*)extbnb_file->Get("h_deltax_2d");


  // Muon momentum
  //TEfficiency * eff_mumom = (TEfficiency*)mc_bnbcosmic_file->Get("h_eff_mumom_den_clone");
  TH1D * h_truth_xsec_mumom = (TH1D*)mc_bnbcosmic_file->Get("h_truth_xsec_mumom");
  TH1D * h_eff_mumom_num = (TH1D*)mc_bnbcosmic_file->Get("h_eff_mumom_num");
  TH1D * h_eff_mumom_den = (TH1D*)mc_bnbcosmic_file->Get("h_eff_mumom_den");
  TH2D * h_true_reco_mom = (TH2D*)mc_bnbcosmic_file->Get("h_true_reco_mom");

  // Muon costheta
  //TEfficiency * eff_muangle = (TEfficiency*)mc_bnbcosmic_file->Get("h_eff_muangle_den_clone");
  TH1D * h_truth_xsec_muangle = (TH1D*)mc_bnbcosmic_file->Get("h_truth_xsec_muangle");
  TH1D * h_eff_muangle_num = (TH1D*)mc_bnbcosmic_file->Get("h_eff_muangle_num");
  TH1D * h_eff_muangle_den = (TH1D*)mc_bnbcosmic_file->Get("h_eff_muangle_den");
  TH2D * h_true_reco_costheta = (TH2D*)mc_bnbcosmic_file->Get("h_true_reco_costheta");

  // Double differential
  std::map<std::string,TH2D*>* temp_map2;
  mc_bnbcosmic_file->GetObject("hmap_trktheta_trkmom", temp_map2);
  std::map<std::string,TH2D*> hmap_trktheta_trkmom_mc = *temp_map2;
  TH2D* h_trktheta_trkmom_total_bnbon = (TH2D*)bnbon_file->Get("h_trktheta_trkmom_total");
  TH2D* h_trktheta_trkmom_total_extbnb = (TH2D*)extbnb_file->Get("h_trktheta_trkmom_total");
  TH2D* h_eff_muangle_mumom_num = (TH2D*)mc_bnbcosmic_file->Get("h_eff_muangle_mumom_num");
  TH2D* h_eff_muangle_mumom_den = (TH2D*)mc_bnbcosmic_file->Get("h_eff_muangle_mumom_den");




  ubana::PlotHandler _plot_handler;
  _plot_handler.SetScaleFactors(scale_factor_mc_bnbcosmic, scale_factor_bnbon, scale_factor_extbnb);
  _plot_handler.SetPOT(bnbon_pot_meas);
  _plot_handler.SetOutDir("output_data_mc");
  std::cout << "FLUX: " << _plot_handler.EstimateFlux() << std::endl;

  /*_plot_handler.SetHistograms(hmap_trklen_mc, h_trklen_total_bnbon, h_trklen_total_extbnb);  
  _plot_handler.SetNameAndLabel("trklen", ";Candidate Track Length [cm]; Selected Events");
  _plot_handler.ProcessPlots();
  _plot_handler.Draw();*/

  std::vector<std::string> hist_to_subtract = {"beam-off", "cosmic", "outfv", "nc", "nue", "anumu"};

  //
  // Muon Momentum
  //
  TMatrix S_2d; S_2d.Clear(); S_2d.ResizeTo(7, 6);
  ::ubana::MigrationMatrix2D migrationmatrix2d;
  migrationmatrix2d.SetNBins(7, 6);
  migrationmatrix2d.SetTrueRecoHistogram(h_true_reco_mom);
  S_2d = migrationmatrix2d.CalculateMigrationMatrix();
  migrationmatrix2d.PlotMatrix();
  _plot_handler.Reset();
  _plot_handler.SetHistograms(hmap_trkmom_mc, h_trkmom_total_bnbon, h_trkmom_total_extbnb);  
  _plot_handler.SetTruthHistograms(h_eff_mumom_num, h_eff_mumom_den, h_true_reco_mom);
  _plot_handler.SetTruthXSec(h_truth_xsec_mumom);
  _plot_handler.SetNameAndLabel("trkmom", ";Candidate Track Momentum (MCS) [GeV]; Selected Events");
  _plot_handler.ProcessPlots();
  _plot_handler.Draw();
  _plot_handler.Draw(hist_to_subtract);
  _plot_handler.SetMigrationMatrix(S_2d);
  _plot_handler.Smear(7, 6);
  _plot_handler.ExtractCrossSection("p_{#mu} [GeV]", "d#sigma/dp_{#mu} [10^{-38} cm^{2}/GeV]");

  //
  // Muon CosTheta
  // 
  S_2d.Clear(); S_2d.ResizeTo(9, 9);
  migrationmatrix2d.SetNBins(9, 9);
  migrationmatrix2d.SetTrueRecoHistogram(h_true_reco_costheta);
  S_2d = migrationmatrix2d.CalculateMigrationMatrix();
  migrationmatrix2d.PlotMatrix();
  _plot_handler.Reset();
  _plot_handler.SetHistograms(hmap_trktheta_mc, h_trktheta_total_bnbon, h_trktheta_total_extbnb);  
  _plot_handler.SetTruthHistograms(h_eff_muangle_num, h_eff_muangle_den, h_true_reco_costheta);
  _plot_handler.SetTruthXSec(h_truth_xsec_muangle);
  _plot_handler.SetNameAndLabel("trkcostheta", ";Candidate Track cos(#theta) [GeV]; Selected Events");
  _plot_handler.ProcessPlots();
  _plot_handler.Draw();
  _plot_handler.Draw(hist_to_subtract);
  _plot_handler.SetMigrationMatrix(S_2d);
  _plot_handler.Smear(9, 9);
  _plot_handler.ExtractCrossSection("cos(#theta_{#mu})", "d#sigma/dcos(#theta_{#mu}) [10^{-38} cm^{2}]");

  //
  // Double Differential
  // 
  ::ubana::MigrationMatrix4D migrationmatrix4d;
  TTree * tt;
  mc_bnbcosmic_file->GetObject("mom_tree", tt);
  migrationmatrix4d.SetTTree(tt);
  int n_bins_mumom_temp = 4;
  double bins_mumom_temp[5] = {0.00, 0.25, 0.50, 1.0, 2.50};
  int n_bins_mucostheta_temp = 6;
  double bins_mucostheta_temp[7] = {-1.00, -0.50, 0.00, 0.25, 0.50, 0.75, 1.00};
  migrationmatrix4d.SetBins(bins_mucostheta_temp, n_bins_mucostheta_temp, bins_mumom_temp, n_bins_mumom_temp);
  auto S_4d = migrationmatrix4d.CalculateMigrationMatrix();
  migrationmatrix4d.SetOutputFileName("latex_test.tex");
  migrationmatrix4d.PrintSmearingMatrixLatex();
  migrationmatrix4d.PlotMatrix();

  ::ubana::CrossSectionCalculator2D xseccalc2d;
  xseccalc2d.SetScaleFactors(scale_factor_mc_bnbcosmic, scale_factor_bnbon, scale_factor_extbnb);
  xseccalc2d.SetPOT(bnbon_pot_meas);
  xseccalc2d.SetOutDir("output_data_mc_xsec2d");
  std::cout << "FLUX: " << xseccalc2d.EstimateFlux() << std::endl;

  xseccalc2d.SetHistograms(hmap_trktheta_trkmom_mc, h_trktheta_trkmom_total_bnbon, h_trktheta_trkmom_total_extbnb);
  xseccalc2d.SetTruthHistograms(h_eff_muangle_mumom_num, h_eff_muangle_mumom_den);
  xseccalc2d.SetNameAndLabel("trkcostheta_trkmumom_", ";Candidate Track cos(#theta) [GeV];Candidate Track Momentum (MCS) [GeV]");
  xseccalc2d.ProcessPlots();
  xseccalc2d.SetSmearingMatrix(S_4d);
  xseccalc2d.Smear();

  xseccalc2d.ExtractCrossSection("cos(#theta_{#mu})", "p_{#mu} [GeV]", "d^{2}#sigma/dcos(#theta_{#mu}dp_{#mu}) [10^{-38} cm^{2}/GeV]");
  

  
  // *************************************
  // Doing beam-on minus beam-off subtraction
  // *************************************
  h_trklen_total_extbnb->Scale(scale_factor_extbnb);
  h_trklen_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_trklen_data = (TH1D*)h_trklen_total_bnbon->Clone("h_trklen_data");
  h_trklen_data->Sumw2();
  h_trklen_data->Add(h_trklen_total_extbnb, -1.);
  
  std::cout << "With the correct normalisation: " << std::endl;
  std::cout << "\t Number of BNBON events:   " << h_trklen_total_bnbon->Integral() << std::endl;
  std::cout << "\t Number of EXTBNB events:  " << h_trklen_total_extbnb->Integral() << std::endl;
  std::cout << "\t           BNBON - EXTBNB: " << h_trklen_total_bnbon->Integral() - h_trklen_total_extbnb->Integral() << std::endl;

  std::cout << std::endl;

  std::cout << "With the correct normalisation (including under/overflows): " << std::endl;
  std::cout << "\t Number of BNBON events:   " << h_trklen_total_bnbon->Integral(0, h_trklen_total_bnbon->GetNbinsX()+1) << std::endl;
  std::cout << "\t Number of EXTBNB events:  " << h_trklen_total_extbnb->Integral(0, h_trklen_total_extbnb->GetNbinsX()+1) << std::endl;
  std::cout << "\t           BNBON - EXTBNB: " << h_trklen_total_bnbon->Integral(0, h_trklen_total_bnbon->GetNbinsX()+1) - h_trklen_total_extbnb->Integral(0, h_trklen_total_extbnb->GetNbinsX()+1) << std::endl;

/*
  h_trkmom_total_extbnb->Scale(scale_factor_extbnb);
  h_trkmom_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_trkmom_data = (TH1D*)h_trkmom_total_bnbon->Clone("h_trkmom_data");
  h_trkmom_data->Sumw2();
  h_trkmom_data->Add(h_trkmom_total_extbnb, -1.);
  */
  
  /*
  h_trktheta_total_extbnb->Scale(scale_factor_extbnb);
  h_trktheta_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_trktheta_data = (TH1D*)h_trktheta_total_bnbon->Clone("h_trktheta_data");
  h_trktheta_data->Sumw2();
  h_trktheta_data->Add(h_trktheta_total_extbnb, -1.);
  */

  h_trktheta_trkmom_total_extbnb->Scale(scale_factor_extbnb);
  h_trktheta_trkmom_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_trktheta_trkmom_data = (TH1D*)h_trktheta_trkmom_total_bnbon->Clone("h_trktheta_trkmom_data");
  h_trktheta_trkmom_data->Sumw2();
  h_trktheta_trkmom_data->Add(h_trktheta_trkmom_total_extbnb, -1.);

  h_trkphi_total_extbnb->Scale(scale_factor_extbnb);
  h_trkphi_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_trkphi_data = (TH1D*)h_trkphi_total_bnbon->Clone("h_trklen_data");
  h_trkphi_data->Sumw2();
  h_trkphi_data->Add(h_trkphi_total_extbnb, -1.);
  
  h_multpfp_total_extbnb->Scale(scale_factor_extbnb);
  h_multpfp_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_multpfp_data = (TH1D*)h_multpfp_total_bnbon->Clone("h_multpfp_data");
  h_multpfp_data->Sumw2();
  h_multpfp_data->Add(h_multpfp_total_extbnb, -1.);
  
  h_multtracktol_total_extbnb->Scale(scale_factor_extbnb);
  h_multtracktol_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_multtracktol_data = (TH1D*)h_multtracktol_total_bnbon->Clone("h_multtracktol_data");
  h_multtracktol_data->Sumw2();
  h_multtracktol_data->Add(h_multtracktol_total_extbnb, -1.);
  
  h_xdiff_b_total_extbnb->Scale(scale_factor_extbnb);
  h_xdiff_b_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_xdiff_b_data = (TH1D*)h_xdiff_b_total_bnbon->Clone("h_xdiff_b_data");
  h_xdiff_b_data->Sumw2();
  h_xdiff_b_data->Add(h_xdiff_b_total_extbnb, -1.);
  
  h_zdiff_b_total_extbnb->Scale(scale_factor_extbnb);
  h_zdiff_b_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_zdiff_b_data = (TH1D*)h_zdiff_b_total_bnbon->Clone("h_zdiff_b_data");
  h_zdiff_b_data->Sumw2();
  h_zdiff_b_data->Add(h_zdiff_b_total_extbnb, -1.);
  
  h_xdiff_total_extbnb->Scale(scale_factor_extbnb);
  h_xdiff_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_xdiff_data = (TH1D*)h_xdiff_total_bnbon->Clone("h_xdiff_data");
  h_xdiff_data->Sumw2();
  h_xdiff_data->Add(h_xdiff_total_extbnb, -1.);
  
  h_zdiff_total_extbnb->Scale(scale_factor_extbnb);
  h_zdiff_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_zdiff_data = (TH1D*)h_zdiff_total_bnbon->Clone("h_zdiff_data");
  h_zdiff_data->Sumw2();
  h_zdiff_data->Add(h_zdiff_total_extbnb, -1.);
  
  h_vtxx_total_extbnb->Scale(scale_factor_extbnb);
  h_vtxx_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_vtxx_data = (TH1D*)h_vtxx_total_bnbon->Clone("h_vtxx_data");
  h_vtxx_data->Sumw2();
  h_vtxx_data->Add(h_vtxx_total_extbnb, -1.);
  
  h_vtxy_total_extbnb->Scale(scale_factor_extbnb);
  h_vtxy_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_vtxy_data = (TH1D*)h_vtxy_total_bnbon->Clone("h_vtxx_data");
  h_vtxy_data->Sumw2();
  h_vtxy_data->Add(h_vtxy_total_extbnb, -1.);
  
  h_vtxz_total_extbnb->Scale(scale_factor_extbnb);
  h_vtxz_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_vtxz_data = (TH1D*)h_vtxz_total_bnbon->Clone("h_vtxx_data");
  h_vtxz_data->Sumw2();
  h_vtxz_data->Add(h_vtxz_total_extbnb, -1.);
  
  h_flsmatch_score_total_extbnb->Scale(scale_factor_extbnb);
  h_flsmatch_score_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_flsmatch_score_data = (TH1D*)h_flsmatch_score_total_bnbon->Clone("h_flsmatch_score_data");
  h_flsmatch_score_data->Sumw2();
  h_flsmatch_score_data->Add(h_flsmatch_score_total_extbnb, -1.);
  
  h_ntpcobj_total_extbnb->Scale(scale_factor_extbnb);
  h_ntpcobj_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_ntpcobj_data = (TH1D*)h_ntpcobj_total_bnbon->Clone("h_ntpcobj_data");
  h_ntpcobj_data->Sumw2();
  h_ntpcobj_data->Add(h_ntpcobj_total_extbnb, -1.);
  
  h_dqdx_trunc_total_extbnb->Scale(scale_factor_extbnb);
  h_dqdx_trunc_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_dqdx_trunc_data = (TH1D*)h_dqdx_trunc_total_bnbon->Clone("h_dqdx_trunc_data");
  h_dqdx_trunc_data->Sumw2();
  h_dqdx_trunc_data->Add(h_dqdx_trunc_total_extbnb, -1.);
  
  h_dqdx_trunc_length_extbnb->Scale(scale_factor_extbnb);
  h_dqdx_trunc_length_bnbon->Scale(scale_factor_bnbon);
  TH2D* h_dqdx_trunc_length_data = (TH2D*)h_dqdx_trunc_length_bnbon->Clone("h_dqdx_trunc_length_data");
  h_dqdx_trunc_length_data->Sumw2();
  h_dqdx_trunc_length_data->Add(h_dqdx_trunc_length_extbnb, -1.);
  
  h_deltax_2d_mc->Scale(scale_factor_mc_bnbcosmic);
  h_deltax_2d_bnbon->Scale(scale_factor_bnbon);
  h_deltax_2d_extbnb->Scale(scale_factor_extbnb);
  TH2D* h_deltax_2d_data = (TH2D*)h_deltax_2d_bnbon->Clone("h_deltax_2d_bnbon");
  h_deltax_2d_data->Sumw2();
  h_deltax_2d_data->Add(h_deltax_2d_extbnb, -1.);

  h_vtxcheck_angle_total_extbnb->Scale(scale_factor_extbnb);
  h_vtxcheck_angle_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_vtxcheck_angle_total_data = (TH1D*)h_vtxcheck_angle_total_bnbon->Clone("h_vtxcheck_angle_total_data");
  h_vtxcheck_angle_total_data->Sumw2();
  h_vtxcheck_angle_total_data->Add(h_vtxcheck_angle_total_extbnb, -1.);

  h_residuals_std_total_extbnb->Scale(scale_factor_extbnb);
  h_residuals_std_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_residuals_std_total_data = (TH1D*)h_residuals_std_total_bnbon->Clone("h_residuals_std_total_data");
  h_residuals_std_total_data->Sumw2();
  h_residuals_std_total_data->Add(h_residuals_std_total_extbnb, -1.);

  h_residuals_mean_total_extbnb->Scale(scale_factor_extbnb);
  h_residuals_mean_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_residuals_mean_total_data = (TH1D*)h_residuals_mean_total_bnbon->Clone("h_residuals_mean_total_data");
  h_residuals_mean_total_data->Sumw2();
  h_residuals_mean_total_data->Add(h_residuals_mean_total_extbnb, -1.);

  h_perc_used_hits_total_extbnb->Scale(scale_factor_extbnb);
  h_perc_used_hits_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_perc_used_hits_total_data = (TH1D*)h_perc_used_hits_total_bnbon->Clone("h_perc_used_hits_total_data");
  h_perc_used_hits_total_data->Sumw2();
  h_perc_used_hits_total_data->Add(h_perc_used_hits_total_extbnb, -1.);

  h_mom_mcs_length_total_extbnb->Scale(scale_factor_extbnb);
  h_mom_mcs_length_total_bnbon->Scale(scale_factor_bnbon);
  TH1D* h_mom_mcs_length_total_data = (TH1D*)h_mom_mcs_length_total_bnbon->Clone("h_mom_mcs_length_total_data");
  h_mom_mcs_length_total_data->Sumw2();
  h_mom_mcs_length_total_data->Add(h_mom_mcs_length_total_extbnb, -1.);

  // *************************************
  // Plotting data and MC distribution
  // *************************************
  
  TLegend* leg;
  TString name;
  TString outdir = "./output_data_mc/";
  
  TCanvas* canvas_trklen = new TCanvas();
  THStack *hs_trklen_mc = new THStack("hs_trklen",";Candidate Track Length [cm]; Selected Events");
  hmap_trklen_mc["beam-off"] = h_trklen_total_extbnb;
  leg = DrawTHStack(hs_trklen_mc, scale_factor_mc_bnbcosmic, true, hmap_trklen_mc);
  std::cout << "\t             MC BNBCOSMIC: " << hmap_trklen_mc["total"]->Integral(0, hmap_trklen_mc["total"]->GetNbinsX()+1) << std::endl;
  DrawDataHisto(h_trklen_total_bnbon);
  leg->AddEntry(hmap_trklen_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_trklen_total_bnbon,"Data (Beam-on)","lep");
  //DrawDataHisto(h_trklen_data);
  //leg->AddEntry(h_trklen_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  
  std::cout << "\t             DATA (on-off): " << h_trklen_data->Integral(0, h_trklen_data->GetNbinsX()+1) << std::endl;
  

  double den = hmap_trklen_mc["signal"]->Integral() +
  hmap_trklen_mc["cosmic"]->Integral() +
  hmap_trklen_mc["outfv"]->Integral() +
  hmap_trklen_mc["nc"]->Integral() +
  hmap_trklen_mc["anumu"]->Integral() +
  hmap_trklen_mc["nue"]->Integral() +
  hmap_trklen_mc["beam-off"]->Integral();

  std::cout << std::endl;
  
  std::cout << "Number of events per channel:" << std::endl;
  std::cout << "signal: " << hmap_trklen_mc["signal"]->Integral() << ", " << hmap_trklen_mc["signal"]->Integral() / den << std::endl;
  std::cout << "cosmic: " << hmap_trklen_mc["cosmic"]->Integral() << ", " << hmap_trklen_mc["cosmic"]->Integral() / den << std::endl;
  std::cout << "outfv: " << hmap_trklen_mc["outfv"]->Integral() << ", " << hmap_trklen_mc["outfv"]->Integral() / den << std::endl;
  std::cout << "nc: " << hmap_trklen_mc["nc"]->Integral() << ", " << hmap_trklen_mc["nc"]->Integral() / den << std::endl;
  std::cout << "anumu: " << hmap_trklen_mc["anumu"]->Integral() << ", " << hmap_trklen_mc["anumu"]->Integral() / den << std::endl;
  std::cout << "nue: " << hmap_trklen_mc["nue"]->Integral() << ", " << hmap_trklen_mc["nue"]->Integral() / den << std::endl;
  std::cout << "beam-off: " << hmap_trklen_mc["beam-off"]->Integral() << ", " << hmap_trklen_mc["beam-off"]->Integral() / den << std::endl;
  std::cout << std::endl;
  std::cout << "PURITY: " << hmap_trklen_mc["signal"]->Integral() / den << std::endl;
  
  std::cout << std::endl;

  std::cout << "Number of events per channel (including over/underflow bins):" << std::endl;

  int nbins = hmap_trklen_mc["signal"]->GetNbinsX();

  den = hmap_trklen_mc["signal"]->Integral(0, nbins+1) +
  hmap_trklen_mc["cosmic"]->Integral(0, nbins+1) +
  hmap_trklen_mc["outfv"]->Integral(0, nbins+1) +
  hmap_trklen_mc["nc"]->Integral(0, nbins+1) +
  hmap_trklen_mc["anumu"]->Integral(0, nbins+1) +
  hmap_trklen_mc["nue"]->Integral(0, nbins+1) +
  hmap_trklen_mc["beam-off"]->Integral(0, nbins+1);

  std::cout << "signal: " << hmap_trklen_mc["signal"]->Integral(0, nbins+1) << ", " << hmap_trklen_mc["signal"]->Integral(0, nbins+1) / den << std::endl;
  std::cout << "cosmic: " << hmap_trklen_mc["cosmic"]->Integral(0, nbins+1) << ", " << hmap_trklen_mc["cosmic"]->Integral(0, nbins+1) / den << std::endl;
  std::cout << "outfv: " << hmap_trklen_mc["outfv"]->Integral(0, nbins+1) << ", " << hmap_trklen_mc["outfv"]->Integral(0, nbins+1) / den << std::endl;
  std::cout << "nc: " << hmap_trklen_mc["nc"]->Integral(0, nbins+1) << ", " << hmap_trklen_mc["nc"]->Integral(0, nbins+1) / den << std::endl;
  std::cout << "anumu: " << hmap_trklen_mc["anumu"]->Integral(0, nbins+1) << ", " << hmap_trklen_mc["anumu"]->Integral(0, nbins+1) / den << std::endl;
  std::cout << "nue: " << hmap_trklen_mc["nue"]->Integral(0, nbins+1) << ", " << hmap_trklen_mc["nue"]->Integral(0, nbins+1) / den << std::endl;
  std::cout << "beam-off: " << hmap_trklen_mc["beam-off"]->Integral(0, nbins+1) << ", " << hmap_trklen_mc["beam-off"]->Integral(0, nbins+1) / den << std::endl;
  std::cout << std::endl;
  std::cout << "PURITY: " << hmap_trklen_mc["signal"]->Integral(0, nbins+1) / den << std::endl;
  
  
  name = outdir + "trklen";
  canvas_trklen->SaveAs(name + ".pdf");
  canvas_trklen->SaveAs(name + ".C","C");

/*
  TCanvas* canvas_trkmom = new TCanvas();
  THStack *hs_trkmom_mc = new THStack("hs_trkmom",";Reconstructed Muon Momentum [GeV]; Selected Events");
  hmap_trkmom_mc["beam-off"] = h_trkmom_total_extbnb;
  leg = DrawTHStack(hs_trkmom_mc, scale_factor_mc_bnbcosmic, true, hmap_trkmom_mc);
  DrawDataHisto(h_trkmom_total_bnbon);
  leg->AddEntry(hmap_trkmom_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_trkmom_total_bnbon,"Data (Beam-on)","lep");  //DrawDataHisto(h_trkmom_data);
  //leg->AddEntry(h_trkmom_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  
  name = outdir + "trkmom";
  canvas_trkmom->SaveAs(name + ".pdf");
  canvas_trkmom->SaveAs(name + ".C","C");
  */
  /*
  TCanvas* canvas_trktheta = new TCanvas();
  THStack *hs_trktheta_mc = new THStack("hs_trktheta",";Candidate Track cos(#theta); Selected Events");
  hmap_trktheta_mc["beam-off"] = h_trktheta_total_extbnb;
  leg = DrawTHStack(hs_trktheta_mc, scale_factor_mc_bnbcosmic, true, hmap_trktheta_mc);
  DrawDataHisto(h_trktheta_total_bnbon);
  leg->AddEntry(hmap_trktheta_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_trktheta_total_bnbon,"Data (Beam-on)","lep");  //DrawDataHisto(h_trktheta_data);
  //leg->AddEntry(h_trktheta_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  
  name = outdir + "trkcostheta";
  canvas_trktheta->SaveAs(name + ".pdf");
  canvas_trktheta->SaveAs(name + ".C","C");
  */

  /*

  TCanvas* canvas_trktheta_trkmom = new TCanvas();
  THStack *hs_trktheta_trkmom_mc = new THStack("hs_trktheta_trkmom",";cos(#theta_{#mu});p_{#mu} [GeV]");
  hmap_trktheta_trkmom_mc["beam-off"] = h_trktheta_trkmom_total_extbnb;
  leg = DrawTHStack2D(hs_trktheta_trkmom_mc, scale_factor_mc_bnbcosmic, true, hmap_trktheta_trkmom_mc);
  //DrawDataHisto2D(h_trktheta_trkmom_total_bnbon);
  leg->AddEntry(hmap_trktheta_trkmom_mc["beam-off"],"Data (Beam-off)","f");
  //leg->AddEntry(h_trktheta_trkmom_total_bnbon,"Data (Beam-on)","lep");  //DrawDataHisto(h_trktheta_data);
  //leg->AddEntry(h_trktheta_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);

  name = outdir + "trkcostheta_trkmom";
  canvas_trktheta_trkmom->SaveAs(name + ".pdf");
  canvas_trktheta_trkmom->SaveAs(name + ".C","C");

  TCanvas* canvas_trktheta_trkmom_signal = new TCanvas();
  hmap_trktheta_trkmom_mc["signal"]->GetXaxis()->SetTitle("cos(#theta_{#mu})");
  hmap_trktheta_trkmom_mc["signal"]->GetYaxis()->SetTitle("p_{#mu} [GeV]");
  hmap_trktheta_trkmom_mc["signal"]->GetXaxis()->SetTitleOffset(1.45);
  hmap_trktheta_trkmom_mc["signal"]->GetYaxis()->SetTitleOffset(1.45);
  hmap_trktheta_trkmom_mc["signal"]->SetFillColor(kRed);
  hmap_trktheta_trkmom_mc["signal"]->Draw("lego1");
  DrawPOT(bnbon_pot_meas);
  name = outdir + "trkcostheta_trkmom_signal";
  canvas_trktheta_trkmom_signal->SaveAs(name + ".pdf");
  canvas_trktheta_trkmom_signal->SaveAs(name + ".C","C");

  TCanvas* canvas_trktheta_trkmom_data = new TCanvas();
  h_trktheta_trkmom_total_bnbon->GetXaxis()->SetTitle("cos(#theta_{#mu})");
  h_trktheta_trkmom_total_bnbon->GetYaxis()->SetTitle("p_{#mu} [GeV]");
  h_trktheta_trkmom_total_bnbon->GetXaxis()->SetTitleOffset(1.45);
  h_trktheta_trkmom_total_bnbon->GetYaxis()->SetTitleOffset(1.45);
  h_trktheta_trkmom_total_bnbon->Draw("lego");
  DrawPOT(bnbon_pot_meas);
  name = outdir + "trkcostheta_trkmom_data";
  canvas_trktheta_trkmom_data->SaveAs(name + ".pdf");
  canvas_trktheta_trkmom_data->SaveAs(name + ".C","C");
  */
  


  TCanvas* canvas_trkphi = new TCanvas();
  THStack *hs_trkphi_mc = new THStack("hs_trkphi",";Candidate Track #phi; Selected Events");
  hmap_trkphi_mc["beam-off"] = h_trkphi_total_extbnb;
  leg = DrawTHStack(hs_trkphi_mc, scale_factor_mc_bnbcosmic, true, hmap_trkphi_mc);
  DrawDataHisto(h_trkphi_total_bnbon);
  leg->AddEntry(hmap_trkphi_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_trkphi_total_bnbon,"Data (Beam-on)","lep");  //DrawDataHisto(h_trkphi_data);
  //leg->AddEntry(h_trkphi_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "trkphi";
  canvas_trkphi->SaveAs(name + ".pdf");
  canvas_trkphi->SaveAs(name + ".C","C");
  
  TCanvas* canvas_multpfp = new TCanvas();
  THStack *hs_multpfp_mc = new THStack("hs_multpfp",";PFP Multiplicity; Selected Events");
  leg = DrawTHStack(hs_multpfp_mc, scale_factor_mc_bnbcosmic, true, hmap_multpfp_mc);
  DrawDataHisto(h_multpfp_data);
  leg->AddEntry(h_multpfp_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "multpfp";
  canvas_multpfp->SaveAs(name + ".pdf");
  canvas_multpfp->SaveAs(name + ".C","C");
  
  TCanvas* canvas_multtracktol = new TCanvas();
  THStack *hs_multtracktol_mc = new THStack("hs_multtracktol",";Track Multiplicity (5 cm); Selected Events");
  leg = DrawTHStack(hs_multtracktol_mc, scale_factor_mc_bnbcosmic, true, hmap_multtracktol_mc);
  DrawDataHisto(h_multtracktol_data);
  leg->AddEntry(h_multtracktol_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "multtracktol";
  canvas_multtracktol->SaveAs(name + ".pdf");
  canvas_multtracktol->SaveAs(name + ".C","C");
  
  TCanvas* canvas_xdiff_b = new TCanvas();
  THStack *hs_xdiff_b_mc = new THStack("hs_xdiff_b",";QLL x - TPC x [cm]; TPCObjects (Before Selection)");
  leg = DrawTHStack2(hs_xdiff_b_mc, scale_factor_mc_bnbcosmic, true, hmap_xdiff_b_mc);
  DrawDataHisto(h_xdiff_b_data);
  leg->AddEntry(h_xdiff_b_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "xdiff_b";
  canvas_xdiff_b->SaveAs(name + ".pdf");
  canvas_xdiff_b->SaveAs(name + ".C","C");
  
  TCanvas* canvas_b_zdiff = new TCanvas();
  THStack *hs_zdiff_b_mc = new THStack("hs_zdiff_b",";Hypo z - Flash z [cm]; TPCObjects (Before Selection)");
  leg = DrawTHStack2(hs_zdiff_b_mc, scale_factor_mc_bnbcosmic, true, hmap_zdiff_b_mc);
  DrawDataHisto(h_zdiff_b_data);
  leg->AddEntry(h_zdiff_b_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "zdiff_b";
  canvas_b_zdiff->SaveAs(name + ".pdf");
  canvas_b_zdiff->SaveAs(name + ".C","C");
  
  TCanvas* canvas_xdiff = new TCanvas();
  THStack *hs_xdiff_mc = new THStack("hs_xdiff",";QLL x - TPC x [cm]; Selected Events");
  leg = DrawTHStack2(hs_xdiff_mc, scale_factor_mc_bnbcosmic, true, hmap_xdiff_mc);
  DrawDataHisto(h_xdiff_data);
  leg->AddEntry(h_xdiff_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "xdiff";
  canvas_xdiff->SaveAs(name + ".pdf");
  canvas_xdiff->SaveAs(name + ".C","C");

  TCanvas* canvas_zdiff = new TCanvas();
  THStack *hs_zdiff_mc = new THStack("hs_zdiff",";Hypo z - Flash z [cm]; Selected Events");
  leg = DrawTHStack2(hs_zdiff_mc, scale_factor_mc_bnbcosmic, true, hmap_zdiff_mc);
  DrawDataHisto(h_zdiff_data);
  leg->AddEntry(h_zdiff_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "zdiff";
  canvas_zdiff->SaveAs(name + ".pdf");
  canvas_zdiff->SaveAs(name + ".C","C");




  TCanvas* canvas_vtxcheck_angle = new TCanvas();
  THStack *hs_vtxcheck_angle_mc = new THStack("hs_vtxcheck_angle",";Angle [rad]; TPCObjects (Before Selection)");
  hmap_vtxcheck_angle_mc["beam-off"] = h_vtxcheck_angle_total_extbnb;
  leg = DrawTHStack2(hs_vtxcheck_angle_mc, scale_factor_mc_bnbcosmic, true, hmap_vtxcheck_angle_mc);
  //leg->AddEntry(hmap_vtxcheck_angle_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_vtxcheck_angle_total_bnbon,"Data (Beam-on)","lep");  // DrawDataHisto(h_vtxx_total_bnbon);
  DrawDataHisto(h_vtxcheck_angle_total_bnbon);
  //leg->AddEntry(h_vtxcheck_angle_total_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "vtxcheck_angle";
  canvas_vtxcheck_angle->SaveAs(name + ".pdf");
  canvas_vtxcheck_angle->SaveAs(name + ".C","C");  



  TCanvas* canvas_residuals_std = new TCanvas();
  THStack *hs_residulas_std_mc = new THStack("hs_residulas_std",";#sigma_{r_{i}}; TPCObjects (Before Selection)");
  hmap_residuals_std_mc["beam-off"] = h_residuals_std_total_extbnb;
  leg = DrawTHStack2(hs_residulas_std_mc, scale_factor_mc_bnbcosmic, true, hmap_residuals_std_mc);
  //leg->AddEntry(hmap_residuals_std_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_residuals_std_total_bnbon,"Data (Beam-on)","lep");  // DrawDataHisto(h_vtxx_total_bnbon);
  DrawDataHisto(h_residuals_std_total_bnbon);
  //leg->AddEntry(h_vtxcheck_angle_total_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "residulas_std";
  canvas_residuals_std->SaveAs(name + ".pdf");
  canvas_residuals_std->SaveAs(name + ".C","C");  



  TCanvas* canvas_residuals_mean = new TCanvas();
  THStack *hs_residulas_mean_mc = new THStack("hs_residulas_mean",";<r_{i}>; TPCObjects (Before Selection)");
  hmap_residuals_mean_mc["beam-off"] = h_residuals_mean_total_extbnb;
  leg = DrawTHStack2(hs_residulas_mean_mc, scale_factor_mc_bnbcosmic, true, hmap_residuals_mean_mc);
  //leg->AddEntry(hmap_residuals_mean_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_residuals_mean_total_bnbon,"Data (Beam-on)","lep");  // DrawDataHisto(h_vtxx_total_bnbon);
  DrawDataHisto(h_residuals_mean_total_bnbon);
  //leg->AddEntry(h_vtxcheck_angle_total_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "residulas_mean";
  canvas_residuals_mean->SaveAs(name + ".pdf");
  canvas_residuals_mean->SaveAs(name + ".C","C");




  TCanvas* canvas_perc_used_hits = new TCanvas();
  THStack *hs_perc_used_hits_mc = new THStack("hs_residulas_mean",";Fraction of used hits in cluster; TPCObjects (Before Selection)");
  hmap_perc_used_hits_mc["beam-off"] = h_perc_used_hits_total_extbnb;
  leg = DrawTHStack2(hs_perc_used_hits_mc, scale_factor_mc_bnbcosmic, true, hmap_perc_used_hits_mc);
  //leg->AddEntry(hmap_perc_used_hits_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_perc_used_hits_total_bnbon,"Data (Beam-on)","lep");  // DrawDataHisto(h_vtxx_total_bnbon);
  DrawDataHisto(h_perc_used_hits_total_bnbon);
  //leg->AddEntry(h_vtxcheck_angle_total_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "residulas_mean";
  canvas_perc_used_hits->SaveAs(name + ".pdf");
  canvas_perc_used_hits->SaveAs(name + ".C","C");

  
  TCanvas* canvas_mom_mcs_length = new TCanvas();
  THStack *hs_mom_mcs_length_mc = new THStack("hs_residulas_mean",";(MCS - Length) Reconstructed Momentum [GeV]; TPCObjects (Before Selection)");
  hmap_mom_mcs_length_mc["beam-off"] = h_mom_mcs_length_total_extbnb;
  leg = DrawTHStack2(hs_mom_mcs_length_mc, scale_factor_mc_bnbcosmic, true, hmap_mom_mcs_length_mc);
  //leg->AddEntry(hmap_perc_used_hits_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_mom_mcs_length_total_bnbon,"Data (Beam-on)","lep");  // DrawDataHisto(h_vtxx_total_bnbon);
  DrawDataHisto(h_mom_mcs_length_total_bnbon);
  //leg->AddEntry(h_vtxcheck_angle_total_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "mom_mcs_length";
  canvas_mom_mcs_length->SaveAs(name + ".pdf");
  canvas_mom_mcs_length->SaveAs(name + ".C","C");
  
  
  TCanvas* canvas_vtxx = new TCanvas();
  THStack *hs_vtxx_mc = new THStack("hs_vtxx",";Candidate Neutrino Vertex X [cm]; Selected Events");
  hmap_vtxx_mc["beam-off"] = h_vtxx_total_extbnb;
  leg = DrawTHStack2(hs_vtxx_mc, scale_factor_mc_bnbcosmic, true, hmap_vtxx_mc);
  //leg->AddEntry(hmap_vtxx_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_vtxx_total_bnbon,"Data (Beam-on)","lep");  // DrawDataHisto(h_vtxx_total_bnbon);
  DrawDataHisto(h_vtxx_total_bnbon);
  hs_vtxx_mc->SetMaximum(600);
  //leg->AddEntry(h_vtxx_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "vtxx";
  canvas_vtxx->SaveAs(name + ".pdf");
  canvas_vtxx->SaveAs(name + ".C","C");

  std::cout << "[vtxx] beam-off integral: " << hmap_vtxx_mc["beam-off"]->Integral() << std::endl;
  std::cout << "[vtxx] beam-on integral: " << h_vtxx_total_bnbon->Integral() << std::endl;
  std::cout << "[vtxx] MC integral: " << hmap_vtxx_mc["total"]->Integral() << std::endl;
   
  TCanvas* canvas_vtxy = new TCanvas();
  THStack *hs_vtxy_mc = new THStack("hs_vtxy",";Candidate Neutrino Vertex Y [cm]; Selected Events");
  hmap_vtxy_mc["beam-off"] = h_vtxy_total_extbnb;
  leg = DrawTHStack2(hs_vtxy_mc, scale_factor_mc_bnbcosmic, true, hmap_vtxy_mc);
  //leg->AddEntry(hmap_vtxy_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_vtxy_total_bnbon,"Data (Beam-on)","lep");  // DrawDataHisto(h_vtxy_data);
  DrawDataHisto(h_vtxy_total_bnbon);
  hs_vtxy_mc->SetMaximum(650);
  //leg->AddEntry(h_vtxy_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "vtxy";
  canvas_vtxy->SaveAs(name + ".pdf");
  canvas_vtxy->SaveAs(name + ".C","C");
  
  TCanvas* canvas_vtxz = new TCanvas();
  THStack *hs_vtxz_mc = new THStack("hs_vtxz",";Candidate Neutrino Vertex Z [cm]; Selected Events");
  hmap_vtxz_mc["beam-off"] = h_vtxz_total_extbnb;
  leg = DrawTHStack2(hs_vtxz_mc, scale_factor_mc_bnbcosmic, true, hmap_vtxz_mc);
  //h_vtxz_data->Draw("E1 same");
  //leg->AddEntry(hmap_vtxz_mc["beam-off"],"Data (Beam-off)","f");
  leg->AddEntry(h_vtxz_total_bnbon,"Data (Beam-on)","lep");  // DrawDataHisto(h_vtxz_data);
  DrawDataHisto(h_vtxz_total_bnbon);
  hs_vtxz_mc->SetMaximum(900);
  //leg->AddEntry(h_vtxz_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "vtxz";
  canvas_vtxz->SaveAs(name + ".pdf");
  canvas_vtxz->SaveAs(name + ".C","C");
  
  TCanvas* canvas_flsmatch_score = new TCanvas();
  THStack *hs_flsmatch_score_mc = new THStack("hs_flsmatch_score",";1/(-log(L)); Selected Events");
  leg = DrawTHStack2(hs_flsmatch_score_mc, scale_factor_mc_bnbcosmic, true, hmap_flsmatch_score_mc);
  //h_vtxz_data->Draw("E1 same");
  DrawDataHisto(h_flsmatch_score_data);
  //hs_flsmatch_score_mc->SetMaximum(900);
  leg->AddEntry(h_flsmatch_score_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "flsmatch_score";
  canvas_flsmatch_score->SaveAs(name + ".pdf");
  canvas_flsmatch_score->SaveAs(name + ".C","C");
  
  TCanvas* canvas_ntpcobj = new TCanvas();
  THStack *hs_ntpcobj_mc = new THStack("hs_ntpcobj",";Number of TPCObjects per events; ");
  leg = DrawTHStack2(hs_ntpcobj_mc, scale_factor_mc_bnbcosmic, true, hmap_ntpcobj_mc);
  DrawDataHisto(h_ntpcobj_data);
  leg->AddEntry(h_ntpcobj_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();
  
  name = outdir + "ntpcobj";
  canvas_ntpcobj->SaveAs(name + ".pdf");
  canvas_ntpcobj->SaveAs(name + ".C","C");
  
  TCanvas* canvas_dqdx_trunc = new TCanvas();
  THStack *hs_dqdx_trunc_mc = new THStack("hs_dqdx_trunc",";Candidate Track <dQ/dx>_{trunc};");
  leg = DrawTHStack3(hs_dqdx_trunc_mc, scale_factor_mc_bnbcosmic, true, hmap_dqdx_trunc_mc);
  DrawDataHisto(h_dqdx_trunc_data);
  leg->AddEntry(h_dqdx_trunc_data,"Data (Beam-on - Beam-off)","lep");
  DrawPOT(bnbon_pot_meas);
  leg->Draw();

  name = outdir + "dqdx_trunc";
  canvas_dqdx_trunc->SaveAs(name + ".pdf");
  canvas_dqdx_trunc->SaveAs(name + ".C","C");
  
  TCanvas* canvas_dqdx_trunc_length_mc = new TCanvas();
  h_dqdx_trunc_length_mc->Draw("colz");
 
  name = outdir + "dqdx_trunc_length_mc";
  canvas_dqdx_trunc_length_mc->SaveAs(name + ".pdf");
  canvas_dqdx_trunc_length_mc->SaveAs(name + ".C","C");
 
  TCanvas* canvas_dqdx_trunc_length_data = new TCanvas();
  h_dqdx_trunc_length_data->Draw("colz");

  name = outdir + "dqdx_trunc_length_data";
  canvas_dqdx_trunc_length_data->SaveAs(name + ".pdf");
  canvas_dqdx_trunc_length_data->SaveAs(name + ".C","C");
 
  // *************************************
  // Plotting data and MC w/o subtraction
  // *************************************
  /*
  hmap_trklen_mc["intimecosmic"] = h_trklen_intimecosmic;
  std::cout << "1) " << hmap_trklen_mc["intimecosmic"]->Integral() << std::endl;
  hmap_trklen_mc["intimecosmic"]->Scale(scale_factor_mc_intimecosmic);
  std::cout << "2) " << hmap_trklen_mc["intimecosmic"]->Integral() << std::endl;
  TCanvas* canvas_trklen_nosub = new TCanvas();
  THStack *hs_trklen_mc_nosub = new THStack("hs_trklen_nosub",";Candidate Track Length [cm]; Selected Events");
  leg = DrawTHStack(hs_trklen_mc_nosub, scale_factor_mc_bnbcosmic, true, hmap_trklen_mc);
  DrawDataHisto(h_trklen_total_bnbon);
  leg->AddEntry(h_trklen_total_bnbon,"Data Beam-on","lep");
  DrawDataHisto(h_trklen_total_extbnb);
  leg->AddEntry(h_trklen_total_extbnb,"Data Beam-off","lep");
  DrawPOT(bnbon_pot_meas);
  std::cout << "3) " << hmap_trklen_mc["intimecosmic"]->Integral() << std::endl;
  */
  
  // *************************************
  // Other data/MC distributions
  // *************************************
  TH1D* h_flsTime_mc = (TH1D*)mc_bnbcosmic_file->Get("h_flsTime_wcut");
  TH1D* h_flsTime_bnbon = (TH1D*)bnbon_file->Get("h_flsTime_wcut");
  TH1D* h_flsTime_extbnb = (TH1D*)extbnb_file->Get("h_flsTime_wcut");
  h_flsTime_extbnb->Scale(scale_factor_extbnb);
  h_flsTime_bnbon->Scale(scale_factor_bnbon);
  h_flsTime_mc->Scale(scale_factor_mc_bnbcosmic);
  TH1D* h_flsTime_data = (TH1D*)h_flsTime_bnbon->Clone("h_flsTime_data");
  h_flsTime_data->Sumw2();
  h_flsTime_data->Add(h_flsTime_extbnb, -1.);
  
  new TCanvas();
  h_flsTime_mc->Draw("histo");
  h_flsTime_data->SetLineColor(kRed);
  h_flsTime_data->SetMarkerColor(kRed);
  DrawDataHisto(h_flsTime_data);
  TLegend* leg2;
  leg2 = new TLegend(0.13,0.69,0.45,0.87,NULL,"brNDC");
  leg2->AddEntry(h_flsTime_mc,"MC BNB+COSMIC","l");
  leg2->AddEntry(h_flsTime_data,"Data (Beam-on - Beam-off)","lep");
  leg2->Draw();
  DrawPOT(bnbon_pot_meas);

  new TCanvas();
  h_flsTime_mc->SetLineColor(kBlack);
  h_flsTime_bnbon->SetLineColor(kRed);
  h_flsTime_extbnb->SetLineColor(kBlue);
  h_flsTime_mc->Draw("histo");
  h_flsTime_bnbon->Draw("histo same");
  h_flsTime_extbnb->Draw("histo same");
  TLegend* leg3;
  leg3 = new TLegend(0.13,0.69,0.45,0.87,NULL,"brNDC");
  leg3->AddEntry(h_flsTime_mc,"MC BNB+COSMIC","l");
  leg3->AddEntry(h_flsTime_bnbon,"DATA BNBON","l");
  leg3->AddEntry(h_flsTime_extbnb,"DATA EXTBNB","l");
  leg3->Draw();
  DrawPOT(bnbon_pot_meas);

  
  new TCanvas();
  h_deltax_2d_mc->Draw("colz");
  new TCanvas();
  h_deltax_2d_data->Draw("colz");
  
  // Computing time
  clock_t end = clock();
  double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
  cout << endl << endl << "Computing time: " << elapsed_secs << " seconds = " << elapsed_secs/60. << " minutes." << endl << endl;
  
  rootapp->Run();
  //rootapp->Terminate(0);
  
  return 0;
}








