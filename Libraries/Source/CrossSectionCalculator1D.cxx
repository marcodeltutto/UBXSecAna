#ifndef CROSSSECTIONCALCULATOR1D_CXX
#define CROSSSECTIONCALCULATOR1D_CXX

#include "CrossSectionCalculator1D.h"

namespace ubana {

  CrossSectionCalculator1D::CrossSectionCalculator1D()
  {
  }

  void CrossSectionCalculator1D::Reset() 
  {

    _configured = false;

    //_scale_factor_mc_bnbcosmic = -999;
    //_scale_factor_bnbon = -999;
    //_scale_factor_extbnb = -999;
    //_scale_factor_mc_intimecosmic = -999;

    //_pot = -1;

    _name = "Not configured!";
    _label = "Not configured!";

    //_outdir = "NotConfigured";

    _hmap_bnbcosmic.clear();
    _h_bnbon = NULL;
    _h_extbnb = NULL;
    _h_intimecosmic = NULL;

    _h_eff_mumom_num = NULL;
    _h_eff_mumom_den = NULL;

    _h_true_reco_mom = NULL;
  }

  void CrossSectionCalculator1D::SetScaleFactors(double bnbcosmic, double bnbon, double extbnb, double intimecosmic)
  {
    _scale_factor_mc_bnbcosmic = bnbcosmic;
    _scale_factor_bnbon = bnbon;
    _scale_factor_extbnb = extbnb;
    _scale_factor_mc_intimecosmic = intimecosmic;

    _configured = true;
  }

  void CrossSectionCalculator1D::SetPOT(double pot)
  {
    _pot = pot;
  }

  void CrossSectionCalculator1D::SetNameAndLabel(std::string name, std::string label)
  {
    _name = name;
    _label = label;
  }

  void CrossSectionCalculator1D::SetOutDir(std::string dir)
  {
    _outdir = dir;

    auto now = std::time(nullptr);
    char buf[sizeof("YYYY-MM-DD_HH-MM-SS")];
    std::string timestamp = std::string(buf,buf + std::strftime(buf,sizeof(buf),"%F_%H-%M-%S",std::gmtime(&now)));

    _folder = _outdir + "_" + timestamp + "/";

    system(("mkdir " + _folder).c_str());

  }

  void CrossSectionCalculator1D::SetMigrationMatrix(TMatrix s) 
  {
    _S.ResizeTo(s.GetNrows(), s.GetNcols());
    _S = s;
  }

  void CrossSectionCalculator1D::PrintConfig() {

    std::cout << "--- CrossSectionCalculator1D:" << std::endl;
    std::cout << "---   _scale_factor_mc_bnbcosmic     = " << _scale_factor_mc_bnbcosmic << std::endl;
    std::cout << "---   _scale_factor_bnbon            = " << _scale_factor_bnbon << std::endl;
    std::cout << "---   _scale_factor_extbnb           = " << _scale_factor_extbnb << std::endl;
    std::cout << "---   _scale_factor_mc_intimecosmic  = " << _scale_factor_mc_intimecosmic << std::endl;
    std::cout << "---   _pot                           = " << _pot << std::endl;

  }

  void CrossSectionCalculator1D::SetHistograms(std::map<std::string,TH1D*> bnbcosmic, TH1D* bnbon, TH1D* extbnb, TH1D* intimecosmic) 
  {

    _hmap_bnbcosmic = bnbcosmic;
    _h_bnbon = bnbon;
    _h_extbnb = extbnb;
    _h_intimecosmic = intimecosmic;

  }

  void CrossSectionCalculator1D::SetTruthHistograms(TH1D* num, TH1D* den, TH2D* h)
  {
    _h_eff_mumom_num = num;
    _h_eff_mumom_den = den;

    _h_true_reco_mom = h;

  }

  void CrossSectionCalculator1D::SetTruthXSec(TH1D* xsec) 
  {
    _truth_xsec = xsec;
  }

  double CrossSectionCalculator1D::EstimateFlux() 
  {
    std::string flux_file = std::getenv("UBXSecAnaFluxFile");
    std::cout << "Using flux file: " << flux_file << std::endl;
    TFile * f = TFile::Open(flux_file.c_str());
    f->cd();
    TH1D * h_flux_numu = (TH1D*) f->Get("numu");
    h_flux_numu->Scale(_pot/1.e20);


    TCanvas * c_flux = new TCanvas();

    std::stringstream sstm2;
    sstm2 << "##nu / " << _pot << " POT / cm^{2}";
    std::string str = sstm2.str();
    h_flux_numu->GetYaxis()->SetTitle(str.c_str());
    h_flux_numu->GetXaxis()->SetTitle("E_{#nu} [GeV]");

    h_flux_numu->Draw("histo");

    double mean = h_flux_numu-> GetMean();
    std::cout << "The mean energy is: " << mean << std::endl;
    int binmean = h_flux_numu -> FindBin(mean);
    std::cout << "The bin of the mean is: " << binmean << std::endl;

    int n = h_flux_numu -> GetNbinsX();

    double lowerint = h_flux_numu -> Integral(1, binmean);
    std::cout << "Lower Integral: " << lowerint << std::endl;
    double lowerborder = lowerint * 0.32;
    std::cout << "Lower Border: " << lowerborder << std::endl;
    double lowersum = 0;
    int i = 0;
    while (lowersum < lowerborder) {
      i++;
      lowersum += h_flux_numu -> GetBinContent(i);
      std::cout << i << "\t" << lowersum << std::endl;
    }

    std::cout << "Lower Sum: " << lowersum << std::endl;
    double low = h_flux_numu -> GetBinCenter(i-1);
    std::cout << "The lower edge bin is: " << i-1 << std::endl;
    std::cout << "The lower edge center energy is: " << low << std::endl;
    std::cout << "The lower energy error is: " << mean - low << std::endl;

    double upperint = h_flux_numu -> Integral(binmean, n);
    std::cout << upperint << std::endl;
    double upperborder = upperint * 0.32;
    double uppersum = 0;
    i = 0;
    while (uppersum < upperborder) {
      uppersum += h_flux_numu -> GetBinContent(n+1 - i);
      i++;
    }

    double up = h_flux_numu -> GetBinCenter(n+1 - (i-1));
    std::cout << "The upper edge bin is: " << i-1 << std::endl;
    std::cout << "The upper edge center energy is: " << up << std::endl;
    std::cout << "The upper energy error is: " << up - mean << std::endl;

    TGraph *gmean = new TGraph();
    gmean -> SetPoint(0, mean, 0);
    gmean -> SetPoint(1, mean, 1e10);
    gmean -> SetLineWidth(2);
    gmean -> SetLineColor(kOrange+1);
    gmean -> Draw("same");

    TGraph *glow = new TGraph();
    glow -> SetPoint(0, low, 0);
    glow -> SetPoint(1, low, 1e10);
    glow -> SetLineWidth(2);
    glow -> SetLineColor(kOrange+1);
    glow -> SetLineStyle(7);
    glow -> Draw("same");

    TGraph *gup = new TGraph();
    gup -> SetPoint(0, up, 0);
    gup -> SetPoint(1, up, 1e10);
    gup -> SetLineWidth(2);
    gup -> SetLineColor(kOrange+1);
    gup -> SetLineStyle(7);
    gup -> Draw("same");

    TLegend *l = new TLegend(0.6, 0.7, 0.89, 0.89);
    l -> AddEntry(h_flux_numu, "BNB #nu_{#mu} flux, #nu-mode", "l");
    l -> AddEntry(gmean, "<E_{#nu}>", "l");
    l -> AddEntry(glow, "1#sigma Energy Range", "l");
    l -> Draw();



    TString name = _folder + "_flux";
    c_flux->SaveAs(name + ".pdf");
    c_flux->SaveAs(name + ".C","C");
    
    _flux = h_flux_numu->Integral();

    return _flux;
  }

  void CrossSectionCalculator1D::Smear(int n, int m)
  {


    // Settings for true distributions

    _h_eff_mumom_den->SetTitle("");
    _h_eff_mumom_den->GetXaxis()->SetTitle("cos(#theta_{#mu}^{truth})");//->SetTitle("p_{#mu}^{truth} [GeV]");
    _h_eff_mumom_den->GetYaxis()->SetTitle("Events");
    _h_eff_mumom_den->SetFillColorAlpha(30, 0.35);
    _h_eff_mumom_den->SetLineColor(30);
    _h_eff_mumom_den->SetLineWidth(3);

    _h_eff_mumom_num->SetFillColorAlpha(9, 0.35);
    _h_eff_mumom_num->SetLineColor(9);
    _h_eff_mumom_num->SetLineWidth(3);


    //
    // Efficiency (true)
    //

    TEfficiency* teff_true = new TEfficiency(*_h_eff_mumom_num,*_h_eff_mumom_den);

    TCanvas * c_eff_true = new TCanvas;
    teff_true->SetTitle(";True Muon cos(#theta) [GeV];Efficiency");
    teff_true->SetLineColor(kGreen+3);
    teff_true->SetMarkerColor(kGreen+3);
    teff_true->SetMarkerStyle(20);
    teff_true->SetMarkerSize(0.5);
    teff_true->Draw("AP");

    gPad->Update(); 
    auto g = teff_true->GetPaintedGraph(); 
    g->SetMinimum(0);
    g->SetMaximum(1);
    gPad->Update(); 

    TString name = _folder +_name + "efficiecy_mumon_true";
    c_eff_true->SaveAs(name + ".pdf");

    // 
    // Do the smearing
    //

    TMatrix eff_num_true; eff_num_true.Clear(); eff_num_true.ResizeTo(m, 1);
    TMatrix eff_den_true; eff_den_true.Clear(); eff_den_true.ResizeTo(m, 1);

    for (int bin = 1; bin < m+1; bin++) {
      eff_num_true[bin-1] = _h_eff_mumom_num->GetBinContent(bin);
      eff_den_true[bin-1] = _h_eff_mumom_den->GetBinContent(bin);
    }

    TMatrix eff_num_smear = _S * eff_num_true;
    TMatrix eff_den_smear = _S * eff_den_true;

    TH1D* h_eff_mumom_num_smear = (TH1D*) _h_eff_mumom_num->Clone("h_eff_mumom_num_smear");
    TH1D* h_eff_mumom_den_smear = (TH1D*) _h_eff_mumom_den->Clone("h_eff_mumom_den_smear");

    for (int bin = 1; bin < n+1; bin++) {
      h_eff_mumom_num_smear->SetBinContent(bin, eff_num_smear[bin-1][0]);
      h_eff_mumom_den_smear->SetBinContent(bin, eff_den_smear[bin-1][0]);
    }




    // 
    // Plot without smearing
    //

    _h_eff_mumom_den->Scale(1., "width");
    _h_eff_mumom_num->Scale(1., "width");

    TCanvas * c = new TCanvas;
    _h_eff_mumom_den->SetTitle("");
    _h_eff_mumom_den->GetXaxis()->SetTitle("cos(#theta_{#mu}^{truth})");
    _h_eff_mumom_den->GetYaxis()->SetTitle("Events");
    _h_eff_mumom_den->SetFillColorAlpha(30, 0.35);
    _h_eff_mumom_den->SetLineColor(30);
    _h_eff_mumom_den->SetLineWidth(3);

    _h_eff_mumom_den->Draw("histo");

    _h_eff_mumom_num->SetFillColorAlpha(9, 0.35);
    _h_eff_mumom_num->SetLineColor(9);
    _h_eff_mumom_num->SetLineWidth(3);

    _h_eff_mumom_num->Draw("histo same");

    TLegend * ll = new TLegend(0.5315186,0.7515789,0.8696275,0.8821053,NULL,"brNDC");
    ll->AddEntry(_h_eff_mumom_den,"Generated #nu_{#mu} CC in FV","f");
    ll->AddEntry(_h_eff_mumom_num,"Selected #nu_{#mu} CC in FV","f");
    ll->Draw();

    name = _folder +_name + "all_selected";
    c->SaveAs(name + ".pdf");

    std::cout << "_h_eff_mumom_num->Integral(): " << _h_eff_mumom_num->Integral() << std::endl;
    std::cout << "_h_eff_mumom_den->Integral(): " << _h_eff_mumom_den->Integral() << std::endl;



    // 
    // Plot with smearing
    //

    h_eff_mumom_den_smear->Scale(1., "width");
    h_eff_mumom_num_smear->Scale(1., "width");

    TCanvas * c_smear = new TCanvas;
    h_eff_mumom_den_smear->SetTitle("");
    h_eff_mumom_den_smear->GetXaxis()->SetTitle("cos(#theta_{#mu}^{reco})");
    h_eff_mumom_den_smear->GetYaxis()->SetTitle("Events");
    h_eff_mumom_den_smear->Draw("histo");
    h_eff_mumom_num_smear->Draw("histo same");

    ll->Draw();

    name = _folder +_name + "_all_selected_smear";
    c_smear->SaveAs(name + ".pdf");

    std::cout << "h_eff_mumom_num_smear->Integral(): " << h_eff_mumom_num_smear->Integral() << std::endl;
    std::cout << "h_eff_mumom_den_smear->Integral(): " << h_eff_mumom_den_smear->Integral() << std::endl;


    //
    // Efficiency (reco)
    //

    TEfficiency* teff_reco = new TEfficiency(*h_eff_mumom_num_smear,*h_eff_mumom_den_smear);

    TCanvas * c_eff_reco = new TCanvas;
    teff_reco->SetTitle(";Reco Muon cos(#theta);Efficiency");
    teff_reco->SetLineColor(kGreen+3);
    teff_reco->SetMarkerColor(kGreen+3);
    teff_reco->SetMarkerStyle(20);
    teff_reco->SetMarkerSize(0.5);
    teff_reco->Draw("AP");

    gPad->Update(); 
    auto graph = teff_reco->GetPaintedGraph(); 
    graph->SetMinimum(0);
    graph->SetMaximum(1);
    gPad->Update(); 

    name = _folder +_name + "_efficiecy_reco";
    c_eff_reco->SaveAs(name + ".pdf");


    _eff = teff_reco;

  }



  void CrossSectionCalculator1D::ProcessPlots() 
  {

    // Scale mc histograms
    for (auto iter : _hmap_bnbcosmic) {
      if (iter.second == NULL || iter.first == "intimecosmic" || iter.first == "beam-off") continue;
      iter.second->Sumw2();
      iter.second->Scale(_scale_factor_mc_bnbcosmic);
    }

    // Scale data histograms
    _h_extbnb->Sumw2();
    _h_bnbon->Sumw2();
    _h_extbnb->Scale(_scale_factor_extbnb);
    _h_bnbon->Scale(_scale_factor_bnbon);

    // Get the beam-on - beam-off histogram
    _h_data_sub = (TH1D*)_h_bnbon->Clone("_h_data_sub");
    _h_data_sub->Sumw2();
    _h_data_sub->Add(_h_extbnb, -1.);

    // Save beam off in the MC backgrounds
    _hmap_bnbcosmic["beam-off"] = _h_extbnb;

    // And update the total histogram
    _hmap_bnbcosmic["total"]->Add(_h_extbnb);

  }


  TH1D* CrossSectionCalculator1D::ExtractCrossSection(std::string xaxis_label, std::string yaxis_label) 
  {

    //
    // The two histograms we acually need: MC and data (bkg subtracted)
    //

    TH1D* h_mc = _hmap_bnbcosmic["signal"];
    TH1D* h_data = (TH1D*)_h_bnbon->Clone("h_data");
    h_mc->SetTitle(_label.c_str());
    h_data->Sumw2();

    std::vector<std::string> bkg_names = {"beam-off", "cosmic", "outfv", "nc", "nue", "anumu"};

    for (auto name : bkg_names) 
    {
      h_data->Add(_hmap_bnbcosmic[name], -1.);
    }

    //
    // Create efficiency histogram
    //

    std::cout << "HERE" << std::endl;
    //TEfficiency* teff_true = new TEfficiency(*_h_eff_mumom_num,*_h_eff_mumom_den);
    //_eff = teff_true;

    TH1D * h_eff = (TH1D*)h_mc->Clone("h_eff");
    h_eff->Sumw2();
    for (int b = 1; b < h_mc->GetNbinsX() + 1; b++)
    {
      h_eff->SetBinContent(b, _eff->GetEfficiency(_eff->GetGlobalBin(b)));
      double unc = 0.;
      unc += _eff->GetEfficiencyErrorLow(_eff->GetGlobalBin(b));
      unc += _eff->GetEfficiencyErrorUp(_eff->GetGlobalBin(b));
      unc /= 2.;
      h_eff->SetBinError(b, unc);

      //if (b == 1) h_eff->SetBinContent(b,0.5);
      //if (b == 2) h_eff->SetBinContent(b,0.55);

      //std::cout << "Efficiency at bin " << b << ": " << h_eff->GetBinContent(b) << " =- " << unc << std::endl;
    }

    //
    // Divide by efficiency
    //

    h_mc->Divide(h_eff);
    h_data->Divide(h_eff);

    //
    // Divide by flux, and N_target and bin width
    //

    std::cout << "FLUX: " << _flux
    << "\nN_target: " << _n_target
    << "\nFLUX x N_target: " << _flux*_n_target << std::endl;
    double den = _flux * _n_target * 1e-38;

    h_mc->Scale(1. / den, "width");
    h_data->Scale(1. / den, "width");

    // Do it also for the truth xsec
    _truth_xsec->Scale(1. / den, "width");


    std::cout << "MC Integral: " << h_mc->Integral() << std::endl;
    std::cout << "Data Integral: " << h_data->Integral() << std::endl;


    // Plot the cross section

    TCanvas * c = new TCanvas();
    h_mc->GetXaxis()->SetTitle(xaxis_label.c_str());
    h_mc->GetYaxis()->SetTitle(yaxis_label.c_str());
    h_mc->GetYaxis()->SetTitleOffset(0.77);

    h_mc->SetLineColor(kGreen+2);
    h_mc->SetFillColor(29);
    h_mc->Draw("E2");

    TH1D* h_mc_main = (TH1D*) h_mc->Clone("h_mc_main");
    h_mc_main->SetLineColor(kGreen+2);
    h_mc_main->SetFillColor(0); // fully transparent
    h_mc_main->Draw("histo same");

    _truth_xsec->SetLineColor(kGreen+2);
    //_truth_xsec->Draw("hist same");

    h_data->SetMarkerStyle(kFullCircle);
    h_data->SetMarkerSize(0.6);
    h_data->Draw("E1 same");

    TLegend *l = new TLegend(0.44,0.74, 0.87,0.85,NULL,"brNDC");
    l->AddEntry(h_mc, "MC (Stat. Uncertainty)");
    //l->AddEntry(_truth_xsec, "Monte Carlo (Truth)", "l");
    l->AddEntry(h_data, "Measured (Stat. Uncertainty)", "lep");
    l->Draw();

    TLatex* prelim = new TLatex(0.9,0.93, "MicroBooNE Preliminary");
    prelim->SetTextColor(kGray+1);
    prelim->SetNDC();
    prelim->SetTextSize(2/30.);
    prelim->SetTextAlign(32);
    prelim->SetTextSize(0.04631579);
    prelim->Draw();


    TString name = _folder +_name + "_xsec";
    c->SaveAs(name + ".pdf");
    c->SaveAs(name + ".C","C");

    return h_data;

  }



  void CrossSectionCalculator1D::Draw(std::vector<std::string> histos_to_subtract)
  {

    TH1D* _h_data_subtracted = (TH1D*)_h_bnbon->Clone("_h_data_subtracted");
    _h_data_subtracted->Sumw2();

    for (auto hname : histos_to_subtract) 
    {
      std::cout << "[CrossSectionCalculator1D] Going to subtract histogram " << hname << std::endl;
      // Need to remove from the data histogram
      _h_data_subtracted->Add(_hmap_bnbcosmic[hname], -1.);
      // But also form the total MC one, to properly propagate unc
      _hmap_bnbcosmic["total"]->Add(_hmap_bnbcosmic[hname], -1.);
    }


    TLegend* leg = new TLegend(0.56,0.37,0.82,0.82,NULL,"brNDC");;

    TCanvas* canvas = new TCanvas();

    THStack *hs_mc = this->ProcessTHStack(_hmap_bnbcosmic, leg, histos_to_subtract);

    TH1D* data = ProcessDataHisto(_h_data_subtracted);

    hs_mc->Draw("hist");
    _hmap_bnbcosmic["total"]->Draw("E2 same"); // errors
    data->Draw("E1 same");

    leg->AddEntry(data, "Data (Background subtracted)", "lep");
    leg->Draw();


    TLatex* l = this->GetPOTLatex(_pot);
    l->Draw();

    TString name = _folder +_name + "_test2";
    canvas->SaveAs(name + ".pdf");
    canvas->SaveAs(name + ".C","C");


  }

  void CrossSectionCalculator1D::Draw() 
  {

    TLegend* leg = new TLegend(0.56,0.37,0.82,0.82,NULL,"brNDC");;

    TCanvas* canvas = new TCanvas();

    std::vector<std::string> histos_to_subtract; histos_to_subtract.clear();
    THStack *hs_mc = this->ProcessTHStack(_hmap_bnbcosmic, leg, histos_to_subtract);

    TH1D* data = ProcessDataHisto(_h_bnbon);

    hs_mc->Draw("hist");
    _hmap_bnbcosmic["total"]->Draw("E2 same"); // errors
    data->Draw("same");

    leg->AddEntry(data, "Data (Beam-on)", "lep");
    leg->Draw();


    TLatex* l = this->GetPOTLatex(_pot);
    l->Draw();



    /*
    THStack *hs_mc = new THStack("hs",";Test [cm]; Selected Events");
    //hmap_trklen_mc["beam-off"] = h_trklen_total_extbnb;
    leg = this->DrawTHStack(hs_mc, 1, true, _hmap_bnbcosmic);
    std::cout << "\t             MC BNBCOSMIC: " << _hmap_bnbcosmic["total"]->Integral() << std::endl;
    //DrawDataHisto(h_trklen_total_bnbon);
    //leg->AddEntry(hmap_trklen_mc["beam-off"],"Data (Beam-off)","f");
    //leg->AddEntry(h_trklen_total_bnbon,"Data (Beam-on)","lep");
    this->DrawDataHisto(_h_data_sub);
    leg->AddEntry(_h_data_sub,"Data (Beam-on - Beam-off)","lep");
    //DrawPOT(_pot);
*/

    TString name = _folder +_name + "_test";
    canvas->SaveAs(name + ".pdf");
    canvas->SaveAs(name + ".C","C");


  } 




  THStack * CrossSectionCalculator1D::ProcessTHStack(std::map<std::string,TH1D*> themap, TLegend* leg, std::vector<std::string> histos_to_subtract){

    THStack *hs_trklen = new THStack("hs",_label.c_str());

    bool _breakdownPlots = false;

    bool _draw_beamoff = true, _draw_cosmic = true, _draw_outfv = true, _draw_nue = true, _draw_nc = true, _draw_anumu = true;

    for (auto name: histos_to_subtract)
    {
      if (name == "beam-off") _draw_beamoff = false;
      if (name == "cosmic") _draw_cosmic = false;
      if (name == "outfv") _draw_outfv = false;
      if (name == "nue") _draw_nue = false;
      if (name == "nc") _draw_nc = false;
      if (name == "anumu") _draw_anumu = false;

    }

    if (themap["beam-off"] != NULL && _draw_beamoff) {
      themap["beam-off"]->SetLineColor(kBlue+2);
      themap["beam-off"]->SetFillColor(kBlue+2);
      themap["beam-off"]->SetFillStyle(3004);
      hs_trklen->Add(themap["beam-off"]);
    }

    if (themap["intimecosmic"] != NULL) {
      themap["intimecosmic"]->SetLineColor(kBlue+2);
      themap["intimecosmic"]->SetFillColor(kBlue+2);
      themap["intimecosmic"]->SetFillStyle(3004);
      hs_trklen->Add(themap["intimecosmic"]);
    }

    if (_breakdownPlots) {
      themap["cosmic_nostopmu"]->SetLineColor(kBlue+2);
      themap["cosmic_nostopmu"]->SetFillColor(kBlue+2);
      hs_trklen->Add(themap["cosmic_nostopmu"]);
      themap["cosmic_stopmu"]->SetLineColor(kBlue);
      themap["cosmic_stopmu"]->SetFillColor(kBlue);
      hs_trklen->Add(themap["cosmic_stopmu"]);
      themap["outfv_nostopmu"]->SetLineColor(kGreen+3);
      themap["outfv_nostopmu"]->SetFillColor(kGreen+3);
      hs_trklen->Add(themap["outfv_nostopmu"]);
      themap["outfv_stopmu"]->SetLineColor(kGreen+2);
      themap["outfv_stopmu"]->SetFillColor(kGreen+2);
      hs_trklen->Add(themap["outfv_stopmu"]);
      themap["nc_proton"]->SetLineColor(kGray+2);
      themap["nc_proton"]->SetFillColor(kGray+2);
      hs_trklen->Add(themap["nc_proton"]);
      themap["nc_pion"]->SetLineColor(kGray+1);
      themap["nc_pion"]->SetFillColor(kGray+1);
      hs_trklen->Add(themap["nc_pion"]);
      themap["nc_other"]->SetLineColor(kGray);
      themap["nc_other"]->SetFillColor(kGray);
      hs_trklen->Add(themap["nc_other"]);
    }
    else {
      if (_draw_cosmic) {
        themap["cosmic"]->SetLineColor(kBlue+2);
        themap["cosmic"]->SetFillColor(kBlue+2);
        hs_trklen->Add(themap["cosmic"]);
      }
      if (_draw_outfv) {
        themap["outfv"]->SetLineColor(kGreen+2);
        themap["outfv"]->SetFillColor(kGreen+2);
        hs_trklen->Add(themap["outfv"]);
      }
      if (_draw_nc) {
        themap["nc"]->SetLineColor(kGray);
        themap["nc"]->SetFillColor(kGray);
        hs_trklen->Add(themap["nc"]);
      }
    }

    if (_draw_anumu) {
      themap["anumu"]->SetLineColor(kOrange-3);
      themap["anumu"]->SetFillColor(kOrange-3);
      hs_trklen->Add(themap["anumu"]);
    }
    if (_draw_nue) {
      themap["nue"]->SetLineColor(kMagenta+1);
      themap["nue"]->SetFillColor(kMagenta+1);
      hs_trklen->Add(themap["nue"]);
    }

    if (_breakdownPlots) {
      themap["signal_nostopmu"]->SetLineColor(kRed+2);
      themap["signal_nostopmu"]->SetFillColor(kRed+2);
      hs_trklen->Add(themap["signal_nostopmu"]);
      themap["signal_stopmu"]->SetLineColor(kRed);
      themap["signal_stopmu"]->SetFillColor(kRed);
      hs_trklen->Add(themap["signal_stopmu"]);
    }
    else {
      themap["signal"]->SetLineColor(kRed);
      themap["signal"]->SetFillColor(kRed);
      hs_trklen->Add(themap["signal"]);
    }
    hs_trklen->Draw();

  //h_trklen_total->DrawCopy("hist");

  //gStyle->SetHatchesLineWidth(1);
    themap["total"]->SetFillColor(kBlack);
    themap["total"]->SetFillStyle(3005);
  //themap["total"]->Draw("E2 same");







    if (_breakdownPlots){
      //leg = new TLegend(0.56,0.37,0.82,0.82,NULL,"brNDC");
    } else {
      //leg = new TLegend(0.56,0.54,0.82,0.82,NULL,"brNDC");
    }
    std::stringstream sstm;
  // numu
    if (_breakdownPlots) {
      leg->AddEntry(themap["signal_stopmu"],"#nu_{#mu} CC (stopping #mu)","f");
      leg->AddEntry(themap["signal_nostopmu"],"#nu_{#mu} CC (other)","f");
    } else {
      sstm << "#nu_{#mu} CC (signal), " << std::setprecision(2)  << themap["signal"]->Integral() / themap["total"]->Integral()*100. << "%";
      leg->AddEntry(themap["signal"],sstm.str().c_str(),"f");
      sstm.str("");
    }

  // nue
    sstm << "#nu_{e}, #bar{#nu}_{e} CC, " << std::setprecision(2)  << themap["nue"]->Integral() / themap["total"]->Integral()*100. << "%";
    if (_draw_nue) leg->AddEntry(themap["nue"],sstm.str().c_str(),"f");
    sstm.str("");

  // anumu
    sstm << "#bar{#nu}_{#mu} CC, " << std::setprecision(2)  << themap["anumu"]->Integral() / themap["total"]->Integral()*100. << "%";
    if (_draw_anumu)leg->AddEntry(themap["anumu"],sstm.str().c_str(),"f");
    sstm.str("");

  // nc, outfv, cosmic
    if (_breakdownPlots) {
      leg->AddEntry(themap["nc_other"],"NC (other)","f");
      leg->AddEntry(themap["nc_pion"],"NC (pion)","f");
      leg->AddEntry(themap["nc_proton"],"NC (proton)","f");
      leg->AddEntry(themap["outfv_stopmu"],"OUTFV (stopping #mu)","f");
      leg->AddEntry(themap["outfv_nostopmu"],"OUTFV (other)","f");
      leg->AddEntry(themap["cosmic_stopmu"],"Cosmic (stopping #mu)","f");
      leg->AddEntry(themap["cosmic_nostopmu"],"Cosmic (other)","f");
      if (themap["intimecosmic"] != NULL) {
        leg->AddEntry(themap["intimecosmic"],"In-time cosmics","f");
      }
    } else {
      sstm << "NC, " << std::setprecision(2)  << themap["nc"]->Integral() / themap["total"]->Integral()*100. << "%";
      if (_draw_nc) leg->AddEntry(themap["nc"],sstm.str().c_str(),"f");
      sstm.str("");

      sstm << "OUTFV, " << std::setprecision(2)  << themap["outfv"]->Integral() / themap["total"]->Integral()*100. << "%";
      if (_draw_outfv) leg->AddEntry(themap["outfv"],sstm.str().c_str(),"f");
      sstm.str("");

      sstm << "Cosmic, " << std::setprecision(2)  << themap["cosmic"]->Integral() / themap["total"]->Integral()*100. << "%";
      if (_draw_cosmic) leg->AddEntry(themap["cosmic"],sstm.str().c_str(),"f");
      sstm.str("");
    }
    leg->AddEntry(themap["total"],"MC Stat Unc.","f");
    

    return hs_trklen;

  }

  TH1D* CrossSectionCalculator1D::ProcessDataHisto(TH1D* histo) {

    histo->SetMarkerStyle(kFullCircle);
    histo->SetMarkerSize(0.6);

    return histo;

  }


  TLatex* CrossSectionCalculator1D::GetPOTLatex(double pot) {

    std::stringstream sstm2;
    sstm2 << "Accumulated POT: " << pot;
    std::string str = sstm2.str();

    TLatex* pot_latex_2 = new TLatex(.10, .92, str.c_str());
    pot_latex_2->SetTextFont(62);
    pot_latex_2->SetTextColor(kGray+2);
    pot_latex_2->SetNDC();
    pot_latex_2->SetTextSize(1/30.);
    pot_latex_2->SetTextAlign(10);//left adjusted
    //pot_latex_2->Draw();

    return pot_latex_2;
  
  }


}


#endif
