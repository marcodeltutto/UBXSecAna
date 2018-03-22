#ifndef ReweightingPlotter_CXX
#define ReweightingPlotter_CXX

#include "ReweightingPlotter.h"

namespace ubana {

  ReweightingPlotter::ReweightingPlotter()
  {
  }

  void ReweightingPlotter::SetEventBootstrapMap(std::map<std::string, ubana::BootstrapTH1D> map_bs)
  {
    _map_bs = map_bs;

    _configured = true;
  }

  void ReweightingPlotter::SetEfficiencyBootstraps(ubana::BootstrapTH1D eff_num, ubana::BootstrapTH1D eff_den)
  {
    _bs_eff_num = eff_num;
    _bs_eff_den = eff_den;
  }

  void ReweightingPlotter::SetXSecBootstrap(ubana::BootstrapTH1D bs_xsec) 
  {

    _bs_xsec = bs_xsec;

  }




  void ReweightingPlotter::MakePlots(int variable, bool normalised, bool makeLaTeX) 
  {

    if (!_configured) {
      std::cout << _name << "Not configured." << std::endl;
      throw std::exception();
    }

    TH1D *histo; // the nominal histogram
    std::map<std::string, std::vector<TH1D*>> histo_map; // a map from function name to 2 TH1D (+ and - 1 sigma)
    std::map<std::string, std::vector<TH1D*>> histo_map_den; // (only for eff plot) a map from function name to 2 TH1D (+ and - 1 sigma)
    TH1D *histo_p1; // for each function, this will reoresent the p1 histo
    TH1D *histo_m1; // for each function, this will reoresent the m1 histo

    double efficiency_nominal;
    double efficiency_p1;
    double efficiency_m1;

    if (variable == 2 || variable == 3) {

      BootstrapTH1D bs = _map_bs["total"];

      histo = bs.GetNominal();

      histo_map = bs.UnpackPMHisto();

    }

    if (variable == 0 || variable == 1) {

      // Construct nominal histogram
      //histo->Reset();
      histo = (TH1D*) _bs_eff_num.GetNominal()->Clone("nominal_eff");
      histo->Divide(_bs_eff_den.GetNominal());
      efficiency_nominal = _bs_eff_num.GetNominal()->Integral() / _bs_eff_den.GetNominal()->Integral();

      // Numerator for efficiency calculation 
      histo_map = _bs_eff_num.UnpackPMHisto();
      histo_map_den = _bs_eff_den.UnpackPMHisto();

    }



    // Make a directory to store the plots
    if (variable == 0) system("mkdir ./EvtWgtEfficiencyPlots");
    if (variable == 1) system("mkdir ./EvtWgtEfficiencyPlots");
    if (variable == 2) system("mkdir ./EvtWgtEventPlots");
    if (variable == 3) system("mkdir ./EvtWgtEventPlots");
  
    // Avoid root to dislay the canvases
    //gROOT->SetBatch(kTRUE);
  
    // Opening a text file to write the integrals
    std::ofstream outfile;
    if (variable == 0) outfile.open("./EvtWgtEfficiencyPlots/IntegralsPmu.txt");
    if (variable == 1) outfile.open("./EvtWgtEfficiencyPlots/IntegralsCosThetaMu.txt");
    if (variable == 2) outfile.open("./EvtWgtEventPlots/IntegralsPmu.txt");
    if (variable == 3) outfile.open("./EvtWgtEventPlots/IntegralsCosThetaMu.txt");
  
  
  
  
    // Open LaTeX file to write the table
    std::ofstream latexFile;
    std::ofstream latexFile3;
    if(makeLaTeX) {
      if (variable == 0) latexFile.open("./EvtWgtEfficiencyPlots/evtwgtEfficiencyPmu.tex");
      if (variable == 1) latexFile.open("./EvtWgtEfficiencyPlots/evtwgtEfficiencyCosThetaMu.tex");
      if (variable == 2) latexFile.open("./EvtWgtEventPlots/evtwgtEventPmu.tex");
      if (variable == 3) latexFile.open("./EvtWgtEventPlots/evtwgtEventCosThetaMu.tex");
      latexFile << "\\begin{table}[]" << std::endl;
      latexFile << "\\caption{}" << std::endl;
      latexFile << "\\captionsetup{format=hang,labelfont={sf,bf}}" << std::endl;
      latexFile << "\\label{tab:}" << std::endl;
      latexFile << "\\centering" << std::endl;
      latexFile << "\\begin{tabular}{ccc}" << std::endl;
      latexFile << "\\toprule" << std::endl;
      if (variable == 0 || variable == 1) latexFile << "  &  Efficiency  &  Difference (\\%) \\\\" << std::endl;
      else latexFile << "  &  Integral  &  Difference (\\%) \\\\" << std::endl;
      latexFile << "\\midrule" << std::endl;

      if (variable == 0) latexFile3.open("./EvtWgtEfficiencyPlots/evtwgtEfficiencyPmu_figures.tex", std::ofstream::out | std::ofstream::trunc);
      if (variable == 1) latexFile3.open("./EvtWgtEfficiencyPlots/evtwgtEfficiencyCosThetaMu_figures.tex", std::ofstream::out | std::ofstream::trunc);
      if (variable == 2) latexFile3.open("./EvtWgtEventPlots/evtwgtEventPmu_figures.tex", std::ofstream::out | std::ofstream::trunc);
      if (variable == 3) latexFile3.open("./EvtWgtEventPlots/evtwgtEventCosThetaMu_figures.tex", std::ofstream::out | std::ofstream::trunc);
      latexFile3 << "\\begin{figure}[t]" << std::endl;
      latexFile3 << "\\begin{adjustwidth}{-1cm}{-1cm}" << std::endl;
      latexFile3 << "\\centering" << std::endl;
    }


    bool is_first = true;

    int function_counter = 0;

    for(auto iter : histo_map) {

      function_counter++;

      std::string function_name = iter.first;

      //std::cout << "This is function name: " << function_name << std::endl;


      if (variable == 2 || variable == 3) {

        histo_p1 = iter.second.at(1);
        histo_m1 = iter.second.at(0);

      }

      if (variable == 0 || variable == 1) {

        //histo_p1->Reset();
        histo_p1 = (TH1D*) iter.second.at(1)->Clone("p1");
        histo_p1->Divide(histo_map_den[function_name].at(1));

        efficiency_p1 = iter.second.at(1)->Integral() / histo_map_den[function_name].at(1)->Integral();

        //histo_m1->Reset();
        histo_m1 = (TH1D*) iter.second.at(0)->Clone("m1");
        histo_m1->Divide(histo_map_den[function_name].at(0));

        efficiency_m1 = iter.second.at(0)->Integral() / histo_map_den[function_name].at(0)->Integral();

      }



      TString SaveName;
      if(variable == 0 || variable == 2) SaveName = "Pmu_"+function_name;
      if(variable == 1 || variable == 3) SaveName = "CosThetaMu_"+function_name;
      TString LegName  = GetLegendName(function_name);

      if(normalised) SaveName += "_normalised";


      double histo_Int = histo->Integral();
      double histo_p1_Int = histo_p1->Integral();
      double histo_m1_Int = histo_m1->Integral();


      if (normalised) {
        histo->Scale(1./histo_Int);
        histo_p1->Scale(1./histo_p1_Int);
        histo_m1->Scale(1./histo_m1_Int);
      }

      // Calculate integrals
      outfile << function_name << std::endl;
      outfile << "Integral Nominal:    " << histo->Integral() << std::endl;
      outfile << "Integral nominal_p1: " << histo_p1->Integral() << std::endl;
      outfile << "Integral nominal_m1: " << histo_m1->Integral() << std::endl;

      outfile << "Difference w.r.t. Nominal (%):" << std::endl;
      outfile << "nominal_p1: " << (histo_p1->Integral()-histo->Integral())/(histo->Integral())*100. << std::endl;
      outfile << "nominal_m1: " << (histo_m1->Integral()-histo->Integral())/(histo->Integral())*100. << std::endl;
      outfile << "--------------------------------------" << std::endl << std::endl;


      if(makeLaTeX) {
        if ( (variable == 2) || (variable == 3) ) {
          if (is_first) latexFile << "Nominal" << " & " << histo->Integral() << " & 0" << "\\\\" << std::endl;
          latexFile << "\\midrule" << std::endl;
          latexFile << "$" << GetLegendName(function_name) << " + 1\\sigma$ & " << histo_p1->Integral() << " & " << (histo_p1->Integral()-histo->Integral())/(histo->Integral())*100. << "\\\\" << std::endl;
          latexFile << "$" << GetLegendName(function_name) << " - 1\\sigma$ & " << histo_m1->Integral() << " & " << (histo_m1->Integral()-histo->Integral())/(histo->Integral())*100. << "\\\\" << std::endl;
        }

        if (variable == 0 || variable == 1) {  // save the value of the efficiency to the LaTeX file

          double eff_nom = efficiency_nominal * 100.;
          double eff_p1  = efficiency_p1      * 100.;
          double eff_m1  = efficiency_m1      * 100.;

          if (is_first) latexFile << "Nominal" << " & " << std::setprecision(4) << eff_nom << " & 0" << "\\\\" << std::endl;
          latexFile << "\\midrule" << std::endl;
        
          latexFile << "$" << GetLegendName(function_name) << " + 1\\sigma$ & "
          << std::setprecision(4) << eff_p1 << " & "
          << std::setprecision(4) << (eff_p1-eff_nom)/eff_nom*100. << "\\\\" << std::endl;
        
          latexFile << "$" << GetLegendName(function_name) << " - 1\\sigma$ & "
          << std::setprecision(4) << eff_m1 << " & "
          << std::setprecision(4) << (eff_m1-eff_nom)/eff_nom*100. << "\\\\" << std::endl;
        }

      }



      // Define the Canvas
      TCanvas *c = new TCanvas("c", "canvas", 800, 800);



      // Upper plot will be in pad1
      TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
      pad1->SetBottomMargin(0); // Upper and lower plot are joined
      pad1->SetGridx();         // Vertical grid
      pad1->Draw();             // Draw the upper pad: pad1
      pad1->cd();               // pad1 becomes the current pad
      if (variable == 0 || variable == 1) histo_p1->SetMaximum(1.);
      histo_p1->SetStats(0);          // No statistics on upper plot
      histo_m1->SetStats(0);          // No statistics on upper plot
      histo->SetStats(0);          // No statistics on upper plot
      histo_p1->Draw("histo");               // Draw h1
      histo->Draw("histo same");         // Draw h2 on top of h1
      histo_m1->Draw("histo same");
    
      //uBooNESimulation();
    
      if (normalised) {
        // TLatex
        double x;
        x = 0.87;
        if (variable == 3) x = 0.46;
        double y = 0.52;
        double size = 28;
        int color = 1;
        int font = 43;
        int align = 32;
        TLatex *latex = new TLatex( x, y, "Area Normalised" );
        latex->SetNDC();
        latex->SetTextSize(size);
        latex->SetTextColor(color);
        latex->SetTextFont(font);
        latex->SetTextAlign(align);
      
        latex->Draw();
      
      }

      // Do not draw the Y axis label on the upper plot and redraw a small
      // axis instead, in order to avoid the first label (0) to be clipped.
      histo->GetYaxis()->SetLabelSize(0.);
      TGaxis *axis = new TGaxis( -5, 20, -5, 220, 20,220,510,"");
      axis->SetLabelFont(43); // Absolute font size in pixel (precision 3)
      axis->SetLabelSize(15);
      axis->Draw();
      
      // Legend for the upper plot
      TLegend* leg;
      if (variable == 0 || variable == 2) leg = new TLegend(0.65,0.6,.85,0.87);
      if (variable == 1 || variable == 3) leg = new TLegend(0.216792,0.5723502,0.4172932,0.843318,NULL,"brNDC");
      leg->SetTextFont(42);
      leg->SetBorderSize(0);
      //leg->SetHeader("");
      //leg->SetTextFont(42);
      //leg->AddEntry(histo, "BBA2005 (Nominal)");
      //leg->AddEntry(histo_p1, "Dipole");
      leg->AddEntry(histo, "Nominal");
      leg->AddEntry(histo_p1, LegName + " + 1#sigma");
      leg->AddEntry(histo_m1, LegName + " - 1#sigma");
      leg->Draw();
    
      // lower plot will be in pad
      c->cd();          // Go back to the main canvas before defining pad2
      TPad *pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
      pad2->SetTopMargin(0);
      pad2->SetBottomMargin(0.5);
      pad2->SetGridx(); // vertical grid
      //pad2->SetGridy(); // orizontal grid
      pad2->Draw();
      pad2->cd();       // pad2 becomes the current pad
    
    
      // Define the first ratio plot
      TH1D *ratio_p1 = (TH1D*)histo_p1->Clone("ratio_p1");
      //ratio_p1->SetMinimum(0.92);  // Define Y ..
      //ratio_p1->SetMaximum(1.08); // .. range
      //ratio_p1->Sumw2();
      ratio_p1->SetStats(0);      // No statistics on lower plot
      ratio_p1->Divide(histo);
      ratio_p1->SetLineWidth(2);
      ratio_p1->SetLineColor(kRed+1);
      //ratio_p1->Draw("hist");       // Draw the ratio plot
    
      // Define the second ratio plot
      TH1D *ratio_m1 = (TH1D*)histo_m1->Clone("ratio_m1");
      //ratio_m1->SetMinimum(0.9);  // Define Y ..
      //ratio_m1->SetMaximum(1.1); // .. range
      //ratio_m1->Sumw2();
      ratio_m1->SetStats(0);      // No statistics on lower plot
      ratio_m1->Divide(histo);
      ratio_m1->SetLineWidth(2);
      ratio_m1->SetLineColor(kGreen+2);
      //ratio_m1->Draw("hist same");       // Draw the ratio plot
    
    
    
      // Try to set the Y range for the ratio plots
      double max = ratio_p1->GetMaximum();
      double min = ratio_p1->GetMinimum();
      if (ratio_m1->GetMaximum() > max) max = ratio_m1->GetMaximum();
      if (ratio_m1->GetMinimum() < min) min = ratio_m1->GetMinimum();
      //ratio_p1->GetYaxis()->SetRangeUser(min+0.1*min, max+0.1*max);
      //ratio_p1->SetMinimum(max);  // Define Y ..
      //ratio_p1->SetMaximum(min); // .. range
      //ratio_m1->SetMinimum(max);  // Define Y ..
      //ratio_m1->SetMaximum(min); // .. range

    
      // Draw the ratio plot
      //ratio_p1->Draw("hist");
      //ratio_m1->Draw("hist same");
    
      THStack *hs = new THStack("hs","");
      hs->Add(ratio_p1);
      hs->Add(ratio_m1);
      hs->SetMaximum(hs->GetMaximum("nostack")+0.01);
      hs->SetMinimum(hs->GetMinimum("nostack")-0.01);
      //std::cout << "hs->GetMinimum(): " << hs->GetMinimum("nostack") << std::endl;
      hs->Draw("NOSTACK histo");
    
    
      //ratio_p1->GetYaxis()->SetRangeUser(min+0.1*min, max+0.1*max);
    
       histo_p1->SetMinimum(0.0001); // Otherwise 0 label overlaps (need to do it after the THStack, otherwise sets the minimum)


      //**********************
      //
      // Settings
      //
      //**********************

      // h1 settings
      histo->SetLineColor(kBlack);
      histo->SetLineWidth(2);

      // Y axis h1 plot settings
      histo_p1->GetYaxis()->SetTitle("Selected Events");
      if (variable == 0 || variable == 1) histo_p1->GetYaxis()->SetTitle("Efficiency");
      histo_p1->GetYaxis()->CenterTitle();
      histo_p1->GetYaxis()->SetTitleSize(25);
      histo_p1->GetYaxis()->SetTitleFont(43);
      histo_p1->GetYaxis()->SetTitleOffset(1.55);

      // h2 settings
      histo_p1->SetLineColor(kRed+1);
      histo_p1->SetLineWidth(2);

      // h3 settings
      histo_m1->SetLineColor(kGreen+2);
      histo_m1->SetLineWidth(2);

      // Ratio plot (ratio_p1) settings
      ratio_p1->SetTitle(""); // Remove the ratio title
    
    
      hs->GetYaxis()->SetTitle("Ratio");
      hs->GetYaxis()->CenterTitle();
      hs->GetYaxis()->SetNdivisions(505);
      hs->GetYaxis()->SetTitleSize(25);
      hs->GetYaxis()->SetTitleFont(43);
      hs->GetYaxis()->SetTitleOffset(1.2);
      hs->GetYaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
      hs->GetYaxis()->SetLabelSize(15);
    
    
      if(variable == 0 || variable == 2) hs->GetXaxis()->SetTitle("p_{#mu} [GeV]");
      if(variable == 1 || variable == 3) hs->GetXaxis()->SetTitle("cos#theta_{#mu}");
      hs->GetXaxis()->CenterTitle();
      hs->GetXaxis()->SetTitleSize(25);
      hs->GetXaxis()->SetTitleFont(43);
      hs->GetXaxis()->SetTitleOffset(3.5);
      hs->GetXaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
      hs->GetXaxis()->SetLabelSize(20);
    
      // Draw linea at 1 in ratio plot
      TLine *line;
      if (variable == 0 || variable == 2) line = new TLine(0,1,2.5,1);
      if (variable == 1 || variable == 3) line = new TLine(-1,1,1,1);
      line->SetLineColor(kBlack);
      line->SetLineStyle(9); // dashed
      line->Draw();
    
      if (variable == 0 || variable == 1) {
        c->Print("./EvtWgtEfficiencyPlots/" + SaveName + ".C");
         c->Print("./EvtWgtEfficiencyPlots/" + SaveName + ".pdf");
      }
      if (variable == 2 || variable == 3) {
        c->Print("./EvtWgtEventPlots/" + SaveName + ".C");
        c->Print("./EvtWgtEventPlots/" + SaveName + ".pdf");
      }

      if (makeLaTeX) {
        latexFile3 << "\\subfloat[][]" << std::endl;
        latexFile3 << "   {\\includegraphics[width=.35\\textwidth]{images/EvtWgtEfficiencyPlots/" << SaveName << "}" << std::endl;
        latexFile3 << "   \\label{fig:" << "EvtWgtEfficiencyPlots_" << SaveName << "}} \\quad" << std::endl;
      }

      if (makeLaTeX && function_counter % 12 == 0) {
        latexFile3 << "\\caption{Efficiency Plots}" << std::endl;
        latexFile3 << "\\label{fig:EvtWgtEfficiencyPlots}" << std::endl;
        latexFile3 << "\\end{adjustwidth}" << std::endl;
        latexFile3 << "\\end{figure}" << std::endl;

        latexFile3 << "\\begin{figure}[t]" << std::endl;
        latexFile3 << "\\ContinuedFloat" << std::endl;
        latexFile3 << "\\begin{adjustwidth}{-1cm}{-1cm}" << std::endl;
        latexFile3 << "\\centering" << std::endl;

      }


      is_first = false;

    } // end loop functions
  
    if(makeLaTeX) {

      latexFile << "\\bottomrule" << std::endl;
      latexFile << "\\end{tabular}" << std::endl;
      latexFile << "\\end{table}" << std::endl;


      latexFile3 << "\\caption{Efficiency Plots}" << std::endl;
      latexFile3 << "\\label{fig:EvtWgtXsecDiffPlots}" << std::endl;
      latexFile3 << "\\end{adjustwidth}" << std::endl;
      latexFile3 << "\\end{figure}" << std::endl;
    }



  }

  void ReweightingPlotter::MakeBackgroundPlots(int variable, bool normalised, bool makeLaTeX) {

      // Pmu: variable == 0
  // CosThetaMu: variable == 1


  

  
  // Create output directory
  system("mkdir ./EvtWgtBackgroundPlots");
  system("mkdir ./EvtWgtBackgroundPlots_reducedLegend"); 
 
  // Avoid root to dislay the canvases
  gROOT->SetBatch(kTRUE);
  
  
  double scaleFactor = 1.;//5.3e19/1.22e20;
  
  std::cout << "numu:   " << _map_bs["signal"].GetNominal()->Integral() * scaleFactor << std::endl;
  std::cout << "anumu:  " << _map_bs["anumu"].GetNominal()->Integral() * scaleFactor << std::endl;
  std::cout << "nue:    " << _map_bs["nue"].GetNominal()->Integral() * scaleFactor << std::endl;
  std::cout << "nc:     " << _map_bs["nc"].GetNominal()->Integral() * scaleFactor << std::endl;
  std::cout << "outfv:  " << _map_bs["outfv"].GetNominal()->Integral() * scaleFactor << std::endl;
  std::cout << "cosmic: " << _map_bs["cosmic"].GetNominal()->Integral() * scaleFactor << std::endl;

  std::ofstream latexFile;
  std::ofstream latexFile3;

  double numu_nominal = _map_bs["signal"].GetNominal()->Integral();
  double anumu_nominal = _map_bs["anumu"].GetNominal()->Integral();
  double nue_nominal = _map_bs["nue"].GetNominal()->Integral();
  double nc_nominal = _map_bs["nc"].GetNominal()->Integral();
  double outfv_nominal = _map_bs["outfv"].GetNominal()->Integral();
  double cosmic_nominal = _map_bs["cosmic"].GetNominal()->Integral();

  if(makeLaTeX) {
    if (variable == 0) latexFile.open("./EvtWgtBackgroundPlots/evtwgtBackgroundPmu.tex");
    if (variable == 1) latexFile.open("./EvtWgtBackgroundPlots/evtwgtBackgroundCosThetaMu.tex");
    latexFile << "\\begin{table}[]" << std::endl;
    latexFile << "\\begin{adjustwidth}{-2.1cm}{-1cm}" << std::endl;
    latexFile << "\\caption{}" << std::endl;
    latexFile << "\\captionsetup{format=hang,labelfont={sf,bf}}" << std::endl;
    latexFile << "\\label{tab:}" << std::endl;
    latexFile << "\\centering" << std::endl;
    latexFile << "\\tiny" << std::endl;
    latexFile << "\\begin{tabular}{c   c c  c c  c c  c c  c c  c c }" << std::endl;
    latexFile << "\\toprule" << std::endl;
    latexFile << "  &  \\multicolumn{2}{ c }{$\\nu_\\mu$ CC} &  \\multicolumn{2}{ c }{$\\bar{\\nu}_\\mu$ CC}   & \\multicolumn{2}{ c }{$\\nu_e$, $\\bar{\\nu}_e$ CC}   &  \\multicolumn{2}{ c }{NC} &  \\multicolumn{2}{ c }{OUTFV} &  \\multicolumn{2}{ c }{Cosmic} \\\\" << std::endl;
    latexFile << "  &  Events   &  Diff. (\\%) &  Events  &  Diff. (\\%) & Events  &  Diff. (\\%) &  Events &  Diff. (\\%)  &  Events &  Diff. (\\%)  &  Events &  Diff. (\\%) \\\\" << std::endl;
    latexFile << "\\midrule" << std::endl;
    latexFile << "Nominal & " << _map_bs["signal"].GetNominal()->Integral() << " & 0 "
    << " & " << _map_bs["anumu"].GetNominal()->Integral() << " & 0 "
    << " & " << _map_bs["nue"].GetNominal()->Integral() << " & 0 "
    << " & " << _map_bs["nc"].GetNominal()->Integral() << " & 0 "
    << " & " << _map_bs["outfv"].GetNominal()->Integral() << " & 0 "
    << " & " << _map_bs["cosmic"].GetNominal()->Integral() << " & 0 " << "\\\\" << std::endl;

    if (variable == 0) latexFile3.open("./EvtWgtBackgroundPlots/evtwgtBackgroundPmu_figures.tex", std::ofstream::out | std::ofstream::trunc);
    if (variable == 1) latexFile3.open("./EvtWgtBackgroundPlots/evtwgtBackgroundCosThetaMu_figures.tex", std::ofstream::out | std::ofstream::trunc);
    latexFile3 << "\\begin{figure}[t]" << std::endl;
    latexFile3 << "\\begin{adjustwidth}{-1cm}{-1cm}" << std::endl;
    latexFile3 << "\\centering" << std::endl;
  }
  
  
  
  
  
  TCanvas *c = new TCanvas("c", "canvas", 800, 800);
  
  std::map<std::string, std::vector<TH1D*>> map_bs_signal = _map_bs["signal"].UnpackPMHisto();
  std::map<std::string, std::vector<TH1D*>> map_bs_anumu = _map_bs["anumu"].UnpackPMHisto();
  std::map<std::string, std::vector<TH1D*>> map_bs_nue = _map_bs["nue"].UnpackPMHisto();
  std::map<std::string, std::vector<TH1D*>> map_bs_nc = _map_bs["nc"].UnpackPMHisto();
  std::map<std::string, std::vector<TH1D*>> map_bs_outfv = _map_bs["outfv"].UnpackPMHisto();
  std::map<std::string, std::vector<TH1D*>> map_bs_cosmic = _map_bs["cosmic"].UnpackPMHisto();

  int function_counter = 0;

  for (auto iter : map_bs_signal) {

    function_counter++;

    std::string function_name = iter.first;

    auto ii = map_bs_anumu.find(function_name);
    std::vector<TH1D*> anumu_v = ii->second;
    TH1D* anumu_nom = _map_bs["anumu"].GetNominal();
    ii = map_bs_nue.find(function_name);
    std::vector<TH1D*> nue_v = ii->second;
    TH1D* nue_nom = _map_bs["nue"].GetNominal();
    ii = map_bs_nc.find(function_name);
    std::vector<TH1D*> nc_v = ii->second;
    TH1D* nc_nom = _map_bs["nc"].GetNominal();
    ii = map_bs_outfv.find(function_name);
    std::vector<TH1D*> outfv_v = ii->second;
    TH1D* outfv_nom = _map_bs["outfv"].GetNominal();
    ii = map_bs_cosmic.find(function_name);
    std::vector<TH1D*> cosmic_v = ii->second;
    TH1D* cosmic_nom = _map_bs["cosmic"].GetNominal();

    
    TString SaveName;
    if (variable == 0) SaveName = "Pmu_" + function_name;
    if (variable == 1) SaveName = "CosThetaMu_" + function_name;
    
    c->SetLogy();                                 // Log scale
    //c->SetGridy();                                // Horizontal grid
    
    
    TH1D* histo_numu;
    TH1D* histo_numu_p1;
    TH1D* histo_numu_m1;
    TH1D* histo_anumu;
    TH1D* histo_anumu_p1;
    TH1D* histo_anumu_m1;
    TH1D* histo_nue;
    TH1D* histo_nue_p1;
    TH1D* histo_nue_m1;
    TH1D* histo_nc;
    TH1D* histo_nc_p1;
    TH1D* histo_nc_m1;
    TH1D* histo_outfv;
    TH1D* histo_outfv_p1;
    TH1D* histo_outfv_m1;
    TH1D* histo_cosmic;
    TH1D* histo_cosmic_p1;
    TH1D* histo_cosmic_m1;


    
    if (variable == 0) {
      histo_numu        = (TH1D*)_map_bs["signal"].GetNominal()->Clone("histo_numu");
      histo_numu_p1     = (TH1D*)iter.second.at(1)->Clone("histo_numu_p1");
      histo_numu_m1     = (TH1D*)iter.second.at(0)->Clone("histo_numu_m1");
      
      histo_anumu       = (TH1D*)anumu_nom->Clone("histo_anumu");
      histo_anumu_p1    = (TH1D*)anumu_v.at(1)->Clone("histo_anumu_p1");
      histo_anumu_m1    = (TH1D*)anumu_v.at(0)->Clone("histo_anumu_m1");
      
      histo_nue         = (TH1D*)nue_nom->Clone("histo_nue");
      histo_nue_p1      = (TH1D*)nue_v.at(1)->Clone("histo_nue_p1");
      histo_nue_m1      = (TH1D*)nue_v.at(0)->Clone("histo_nue_m1");
      
      histo_nc          = (TH1D*)nc_nom->Clone("histo_nc");
      histo_nc_p1       = (TH1D*)nc_v.at(1)->Clone("histo_nc_p1");
      histo_nc_m1       = (TH1D*)nc_v.at(0)->Clone("histo_nc_m1");

      histo_outfv       = (TH1D*)outfv_nom->Clone("histo_outfv");
      histo_outfv_p1    = (TH1D*)outfv_v.at(1)->Clone("histo_outfv_p1");
      histo_outfv_m1    = (TH1D*)outfv_v.at(0)->Clone("histo_outfv_m1");

      histo_cosmic      = (TH1D*)cosmic_nom->Clone("histo_cosmic");
      histo_cosmic_p1   = (TH1D*)cosmic_v.at(1)->Clone("histo_cosmic_p1");
      histo_cosmic_m1   = (TH1D*)cosmic_v.at(0)->Clone("histo_cosmic_m1");
    }
    else {
      std::cout << "Invalid option. Exit." << std::endl;
      exit(0);
    }
    
    
    // Settings
    histo_numu->SetStats(0);          // No statistics on upper plot
    if (variable == 1) histo_numu->SetMinimum(1);
    if (variable == 1) histo_numu->SetMaximum(1e5);

    if (variable == 0) histo_numu->GetXaxis()->SetTitle("Reconstructed p_{#mu} [GeV]");
    if (variable == 1) histo_numu->GetXaxis()->SetTitle("Reconstructed cos#theta_{#mu}");
    histo_numu->GetXaxis()->CenterTitle();
    histo_numu->GetXaxis()->SetTitleSize(25);
    histo_numu->GetXaxis()->SetTitleFont(43);
    histo_numu->GetXaxis()->SetTitleOffset(1.45);
    
    histo_numu->GetYaxis()->SetTitle("Events");
    histo_numu->GetYaxis()->CenterTitle();
    histo_numu->GetYaxis()->SetTitleSize(25);
    histo_numu->GetYaxis()->SetTitleFont(43);
    histo_numu->GetYaxis()->SetTitleOffset(1.55);
    
    double lineWidth = 1;
    
    // Nominal
    histo_numu   ->Draw("histo");
    histo_anumu  ->Draw("histo same");
    histo_nue    ->Draw("histo same");
    histo_nc     ->Draw("histo same");
    histo_outfv  ->Draw("histo same");
    histo_cosmic ->Draw("histo same");

    histo_numu   ->SetLineColor(kRed+1);
    histo_anumu  ->SetLineColor(kOrange+1);
    histo_nue    ->SetLineColor(kViolet+1);
    histo_nc     ->SetLineColor(kGray+1);
    histo_outfv  ->SetLineColor(kGreen+1);
    histo_cosmic ->SetLineColor(kBlue+1);
    
    histo_numu   ->SetLineWidth(lineWidth+1);
    histo_anumu  ->SetLineWidth(lineWidth+1);
    histo_nue    ->SetLineWidth(lineWidth+1);
    histo_nc     ->SetLineWidth(lineWidth+1);
    histo_outfv  ->SetLineWidth(lineWidth+1);
    histo_cosmic ->SetLineWidth(lineWidth+1);

    // +- 1 sigma
    histo_numu_p1   ->Draw("histo same");
    histo_anumu_p1  ->Draw("histo same");
    histo_nue_p1    ->Draw("histo same");
    histo_nc_p1     ->Draw("histo same");
    histo_outfv_p1  ->Draw("histo same");
    histo_cosmic_p1 ->Draw("histo same");

    histo_numu_m1   ->Draw("histo same");
    histo_anumu_m1  ->Draw("histo same");
    histo_nue_m1    ->Draw("histo same");
    histo_nc_m1     ->Draw("histo same");
    histo_outfv_m1  ->Draw("histo same");
    histo_cosmic_m1 ->Draw("histo same");

    histo_numu_p1  ->SetLineColor(kRed+1);
    histo_anumu_p1 ->SetLineColor(kOrange+1);
    histo_nue_p1   ->SetLineColor(kViolet+1);
    histo_nc_p1    ->SetLineColor(kGray+1);
    histo_outfv_p1 ->SetLineColor(kGreen+1);
    histo_cosmic_p1->SetLineColor(kBlue+1);

    histo_numu_m1  ->SetLineColor(kRed+1);
    histo_anumu_m1 ->SetLineColor(kOrange+1);
    histo_nue_m1   ->SetLineColor(kViolet+1);
    histo_nc_m1    ->SetLineColor(kGray+1);
    histo_outfv_m1 ->SetLineColor(kGreen+1);
    histo_cosmic_m1->SetLineColor(kBlue+1);

    histo_numu_p1  ->SetLineStyle(7);
    histo_anumu_p1 ->SetLineStyle(7);
    histo_nue_p1   ->SetLineStyle(7);
    histo_nc_p1    ->SetLineStyle(7);
    histo_outfv_p1 ->SetLineStyle(7);
    histo_cosmic_p1->SetLineStyle(7);

    histo_numu_m1  ->SetLineStyle(3);
    histo_anumu_m1 ->SetLineStyle(3);
    histo_nue_m1   ->SetLineStyle(3);
    histo_nc_m1    ->SetLineStyle(3);
    histo_outfv_m1 ->SetLineStyle(3);
    histo_cosmic_m1->SetLineStyle(3);

    histo_numu_p1  ->SetLineWidth(lineWidth);
    histo_anumu_p1 ->SetLineWidth(lineWidth);
    histo_nue_p1   ->SetLineWidth(lineWidth);
    histo_nc_p1    ->SetLineWidth(lineWidth);
    histo_outfv_p1 ->SetLineWidth(lineWidth);
    histo_cosmic_p1->SetLineWidth(lineWidth);

    histo_numu_m1  ->SetLineWidth(lineWidth);
    histo_anumu_m1 ->SetLineWidth(lineWidth);
    histo_nue_m1   ->SetLineWidth(lineWidth);
    histo_nc_m1    ->SetLineWidth(lineWidth);
    histo_outfv_m1 ->SetLineWidth(lineWidth);
    histo_cosmic_m1->SetLineWidth(lineWidth);

    
    // Legend
    TLegend* leg;
    if (variable == 0) leg = new TLegend(0.566416,0.5535484,0.8822055,0.8825806,NULL,"brNDC");
    if (variable == 1) leg = new TLegend(0.1679198,0.5367742,0.4837093,0.8658065,NULL,"brNDC");
    leg->SetTextFont(42);
    leg->SetBorderSize(0);
    
    leg->AddEntry(histo_numu,  "#nu_{#mu} CC (nominal)");
    leg->AddEntry(histo_anumu, "#bar{#nu}_{#mu} CC (nominal)");
    leg->AddEntry(histo_nue,   "#nu_{e} CC (nominal)");
    leg->AddEntry(histo_nc,    "NC all flavours (nominal)");
    leg->AddEntry(histo_nue,   "OUTFV (nominal)");
    leg->AddEntry(histo_nc,    "Cosmic (nominal)");

    leg->AddEntry(histo_numu_p1,   "#nu_{#mu} CC (" + GetLegendName(function_name) + " + 1 #sigma)");
    leg->AddEntry(histo_anumu_p1,  "#bar{#nu}_{#mu} CC (" + GetLegendName(function_name) + " + 1 #sigma)");
    leg->AddEntry(histo_nue_p1,    "#nu_{e} CC (" + GetLegendName(function_name) + " + 1 #sigma)");
    leg->AddEntry(histo_nc_p1,     "NC all flavours (" + GetLegendName(function_name) + " + 1 #sigma)");
    leg->AddEntry(histo_outfv_p1,  "OUTFV (" + GetLegendName(function_name) + " + 1 #sigma)");
    leg->AddEntry(histo_cosmic_p1, "Cosmic (" + GetLegendName(function_name) + " + 1 #sigma)");

    leg->AddEntry(histo_numu_m1,   "#nu_{#mu} CC (" + GetLegendName(function_name) + " - 1 #sigma)");
    leg->AddEntry(histo_anumu_m1,  "#bar{#nu}_{#mu} CC (" + GetLegendName(function_name) + " - 1 #sigma)");
    leg->AddEntry(histo_nue_m1,    "#nu_{e} CC (" + GetLegendName(function_name) + " - 1 #sigma)");
    leg->AddEntry(histo_nc_m1,     "NC all flavours (" + GetLegendName(function_name) + " - 1 #sigma)");
    leg->AddEntry(histo_outfv_m1,  "OUTFV (" + GetLegendName(function_name) + " - 1 #sigma)");
    leg->AddEntry(histo_cosmic_m1, "Cosmic (" + GetLegendName(function_name) + " - 1 #sigma)");

    leg->Draw();
    
    
    c->Print("./EvtWgtBackgroundPlots/" + SaveName + ".C");
    c->Print("./EvtWgtBackgroundPlots/" + SaveName + ".pdf");
    

    // Reduced Legend Plots
    histo_numu->GetXaxis()->SetTitleOffset(1.10);
    histo_numu->GetYaxis()->SetTitleOffset(1.30);
    histo_numu->GetXaxis()->SetTitleSize(30);
    histo_numu->GetYaxis()->SetTitleSize(30);

    histo_numu  ->Draw("histo");
    histo_anumu ->Draw("histo same");
    histo_nue   ->Draw("histo same");
    histo_nc    ->Draw("histo same");
    histo_outfv ->Draw("histo same");
    histo_cosmic->Draw("histo same");

    histo_numu_p1  ->Draw("histo same");
    histo_anumu_p1 ->Draw("histo same");
    histo_nue_p1   ->Draw("histo same");
    histo_nc_p1    ->Draw("histo same");
    histo_outfv_p1 ->Draw("histo same");
    histo_cosmic_p1->Draw("histo same");

    histo_numu_m1  ->Draw("histo same");
    histo_anumu_m1 ->Draw("histo same");
    histo_nue_m1   ->Draw("histo same");
    histo_nc_m1    ->Draw("histo same");
    histo_outfv_m1 ->Draw("histo same");
    histo_cosmic_m1->Draw("histo same");

    TLegend* leg2;
    if (variable == 0) leg2 = new TLegend(0.566416,0.5535484,0.8822055,0.8825806,NULL,"brNDC");
    if (variable == 1) leg2 = new TLegend(0.1679198,0.5367742,0.4837093,0.8658065,NULL,"brNDC");
    leg2->SetTextFont(42);
    leg2->SetBorderSize(0);

    leg2->AddEntry(histo_numu,  "#nu_{#mu} CC");
    leg2->AddEntry(histo_anumu, "#bar{#nu}_{#mu} CC");
    leg2->AddEntry(histo_nue,   "#nu_{e}, #bar{#nu}_{e} CC");
    leg2->AddEntry(histo_nc,    "NC all flavours");
    leg2->AddEntry(histo_outfv, "OUTFV");
    leg2->AddEntry(histo_cosmic,"Cosmic");

    leg2->Draw();

    c->Print("./EvtWgtBackgroundPlots_reducedLegend/" + SaveName + ".C");
    c->Print("./EvtWgtBackgroundPlots_reducedLegend/" + SaveName + ".pdf");

    if (makeLaTeX) {
        latexFile3 << "\\subfloat[][]" << std::endl;
        latexFile3 << "   {\\includegraphics[width=.35\\textwidth]{images/EvtWgtBackgroundPlots/" << SaveName << "}" << std::endl;
        latexFile3 << "   \\label{fig:" << "EvtWgtEfficiencyPlots_" << SaveName << "}} \\quad" << std::endl;
    }

    if (makeLaTeX && function_counter % 12 == 0) {
      latexFile3 << "\\caption{Signal and Background Plots}" << std::endl;
      latexFile3 << "\\label{fig:EvtWgtBackgroundPlotss}" << std::endl;
      latexFile3 << "\\end{adjustwidth}" << std::endl;
      latexFile3 << "\\end{figure}" << std::endl;

      latexFile3 << "\\begin{figure}[t]" << std::endl;
      latexFile3 << "\\ContinuedFloat" << std::endl;
      latexFile3 << "\\begin{adjustwidth}{-1cm}{-1cm}" << std::endl;
      latexFile3 << "\\centering" << std::endl;

    }




    double numu_p1;
    double numu_m1;
    double anumu_p1;
    double anumu_m1;
    double nue_p1;
    double nue_m1;
    double nc_p1;
    double nc_m1;
    double outfv_p1;
    double outfv_m1;    
    double cosmic_p1;
    double cosmic_m1;

    if (variable == 0) {
      numu_p1   = histo_numu_p1   ->Integral();
      numu_m1   = histo_numu_m1   ->Integral();
      anumu_p1  = histo_anumu_p1  ->Integral();
      anumu_m1  = histo_anumu_m1  ->Integral();
      nue_p1    = histo_nue_p1    ->Integral();
      nue_m1    = histo_nue_p1    ->Integral();
      nc_p1     = histo_nc_p1     ->Integral();
      nc_m1     = histo_nc_p1     ->Integral();
      outfv_p1  = histo_outfv_p1  ->Integral();
      outfv_m1  = histo_outfv_p1  ->Integral();
      cosmic_p1 = histo_cosmic_p1 ->Integral();
      cosmic_m1 = histo_cosmic_p1 ->Integral();
    } 
    
    if(makeLaTeX) {
      double numu_diff_p1 = (numu_p1 -numu_nominal)/numu_nominal * 100.;
      double numu_diff_m1 = (numu_p1 -numu_nominal)/numu_nominal * 100.;
      
      double anumu_diff_p1 = (anumu_p1 -anumu_nominal)/anumu_nominal * 100.;
      double anumu_diff_m1 = (anumu_m1 -anumu_nominal)/anumu_nominal * 100.;

      double nue_diff_p1 = (nue_p1 -nue_nominal)/nue_nominal * 100.;
      double nue_diff_m1 = (nue_m1 -nue_nominal)/nue_nominal * 100.;
      
      double nc_diff_p1 = (nc_p1 -nc_nominal)/nc_nominal * 100.;
      double nc_diff_m1 = (nc_p1 -nc_nominal)/nc_nominal * 100.;

      double outfv_diff_p1 = (outfv_p1 -outfv_nominal)/outfv_nominal * 100.;
      double outfv_diff_m1 = (outfv_p1 -outfv_nominal)/outfv_nominal * 100.;

      double cosmic_diff_p1 = (cosmic_p1 -cosmic_nominal)/cosmic_nominal * 100.;
      double cosmic_diff_m1 = (cosmic_p1 -cosmic_nominal)/cosmic_nominal * 100.;

      latexFile << "\\midrule" << std::endl;
      latexFile << "$" << GetLegendName(function_name) << " + 1\\sigma$ "
      << " & " << std::setprecision(5) << numu_p1  << " & " << std::setprecision(2) << numu_diff_p1
      << " & " << std::setprecision(5) << anumu_p1 << " & " << std::setprecision(2) << anumu_diff_p1
      << " & " << std::setprecision(5) << nue_p1   << " & " << std::setprecision(2) << nue_diff_p1
      << " & " << std::setprecision(5) << nc_p1    << " & " << std::setprecision(2) << nc_diff_p1
      << " & " << std::setprecision(5) << outfv_p1 << " & " << std::setprecision(2) << outfv_diff_p1
      << " & " << std::setprecision(5) << cosmic_p1<< " & " << std::setprecision(2) << cosmic_diff_p1 << "\\\\" << std::endl;
      
      latexFile << "$" << GetLegendName(function_name) << " - 1\\sigma$ "
      << " & " << std::setprecision(5) << numu_m1  << " & " << std::setprecision(2) << numu_diff_m1
      << " & " << std::setprecision(5) << anumu_m1 << " & " << std::setprecision(2) << anumu_diff_m1
      << " & " << std::setprecision(5) << nue_m1   << " & " << std::setprecision(2) << nue_diff_m1
      << " & " << std::setprecision(5) << nc_m1    << " & " << std::setprecision(2) << nc_diff_m1
      << " & " << std::setprecision(5) << outfv_m1 << " & " << std::setprecision(2) << outfv_diff_m1
      << " & " << std::setprecision(5) << cosmic_m1<< " & " << std::setprecision(2) << cosmic_diff_m1 << "\\\\" << std::endl;
    }
  }
  
  
  
  
  if(makeLaTeX) {
    latexFile << "\\bottomrule" << std::endl;
    latexFile << "\\end{tabular}" << std::endl;
    latexFile << "\\end{adjustwidth}" << std::endl;
    latexFile << "\\end{table}" << std::endl;

    latexFile3 << "\\caption{Signal and Background Plots}" << std::endl;
    latexFile3 << "\\label{fig:EvtWgtBackgroundPlots}" << std::endl;
    latexFile3 << "\\end{adjustwidth}" << std::endl;
    latexFile3 << "\\end{figure}" << std::endl;
  }
 

  }


  void ReweightingPlotter::MakeXsecDiffPlots(bool makeLaTeX) {


    std::map<std::string, std::vector<TH1D*>> xsec_histo_map = _bs_xsec.UnpackPMHisto();
    TH1D* xsec_nominal = _bs_xsec.GetNominal();



    system("mkdir ./EvtWgtXsecDiffPlots");

    // Open LaTeX file
    std::ofstream latexFile;
    std::ofstream latexFile2;
    std::ofstream latexFile3;

    if (makeLaTeX){
      latexFile.open("./EvtWgtXsecDiffPlots/evtwgtXsecDiffPmu.tex");
      latexFile << "\\begin{table}[]" << std::endl;
      latexFile << "\\caption{evtwgtXsecDiffPmu}" << std::endl;
      latexFile << "\\captionsetup{format=hang,labelfont={sf,bf}}" << std::endl;
      latexFile << "\\label{tab:}" << std::endl;
      latexFile << "\\centering" << std::endl;
      latexFile << "\\begin{tabular}{c c c c}" << std::endl;
      latexFile << "\\toprule" << std::endl;
      latexFile << "  &  Cross-section percental   &    $\\sqrt{S+B}/S$             &   $\\sqrt{S+B}/S$ \\\\"            << std::endl;
      latexFile << "  &  difference (\\%)          &    @ $6.6\\times 10^{20}$ POT  &   @ $0.5\\times 10^{20}$ POT \\\\" << std::endl;
      latexFile << "\\midrule" << std::endl;

      latexFile2.open("./EvtWgtXsecDiffPlots/evtwgtXsecDiffPmu_technote.tex");
      latexFile2 << "\\begin{table}[]" << std::endl;
      latexFile2 << "\\caption{evtwgtXsecDiffPmu}" << std::endl;
      latexFile2 << "\\captionsetup{format=hang,labelfont={sf,bf}}" << std::endl;
      latexFile2 << "\\label{tab:}" << std::endl;
      latexFile2 << "\\centering" << std::endl;
      latexFile2 << "\\begin{tabular}{c c c}" << std::endl;
      latexFile2 << "\\toprule" << std::endl;
      latexFile2 << "  &  \\multicolumn{2}{c}{Cross-section percental difference}  \\\\"            << std::endl;
      latexFile2 << "  &  at maximum point  &  at cross-section peak               \\\\"            << std::endl;
      latexFile2 << "\\midrule" << std::endl;

      latexFile3.open("./EvtWgtXsecDiffPlots/evtwgtXsecDiffPmu_figures.tex", std::ofstream::out | std::ofstream::trunc);
      latexFile3 << "\\begin{figure}[t]" << std::endl;
      latexFile3 << "\\begin{adjustwidth}{-1cm}{-1cm}" << std::endl;
      latexFile3 << "\\centering" << std::endl;
    }

    // Avoid root to dislay the canvases
    gROOT->SetBatch(kTRUE);




    std::map<std::string,TH1D*> XSec_pmu_percDiff_max;
    TString nameBase;
    for (auto iter : xsec_histo_map) {
      nameBase = "xsec_pmu_percDiff_max_";
      XSec_pmu_percDiff_max[iter.first] = (TH1D*) xsec_nominal->Clone(nameBase+iter.first+"_max");
      XSec_pmu_percDiff_max[iter.first]->Reset();
    }

    std::vector<bool> binMax; // This vector contains bin by bin info
                              // true: in this bin Plus1 is bigger
                              // flase: in this bin Minus1 is bigger

    int function_counter = 0;

    for (auto iter : xsec_histo_map) {

      function_counter++;

      std::string function_name = iter.first;


      // Calculate xsec perc difference

      TH1D* XSec_pmu_percDiff_p1;
      TH1D* XSec_pmu_percDiff_m1;

      XSec_pmu_percDiff_p1 = (TH1D*) xsec_nominal->Clone("XSec_pmu_percDiff_p1");
      XSec_pmu_percDiff_p1->Add(iter.second.at(1), -1);
      XSec_pmu_percDiff_p1->Divide(xsec_nominal);
      XSec_pmu_percDiff_p1->Scale(100.);

      XSec_pmu_percDiff_m1 = (TH1D*) xsec_nominal->Clone("XSec_pmu_percDiff_m1");
      XSec_pmu_percDiff_m1->Add(iter.second.at(1), -1);
      XSec_pmu_percDiff_m1->Divide(xsec_nominal);
      XSec_pmu_percDiff_m1->Scale(100.);








      binMax.resize(iter.second.at(1)->GetNbinsX());

      // First set the histograms so that I am plotting absolute values only
      for (int bin = 1; bin < iter.second.at(1)->GetNbinsX()+1; bin++) {

        // First set the histograms so that I am plotting absolute values only
        XSec_pmu_percDiff_p1->SetBinContent(bin, std::abs(XSec_pmu_percDiff_p1->GetBinContent(bin)));
        XSec_pmu_percDiff_m1->SetBinContent(bin, std::abs(XSec_pmu_percDiff_m1->GetBinContent(bin)));
     
        // Now fill XSec_pmu_percDiff_max with the maximum between the two previous histograms
        if (XSec_pmu_percDiff_p1->GetBinContent(bin) > XSec_pmu_percDiff_m1->GetBinContent(bin)){
          binMax[bin] = true;
          XSec_pmu_percDiff_max[function_name]->SetBinContent(bin, XSec_pmu_percDiff_p1->GetBinContent(bin));
        } else {
          binMax[bin] = false;
          XSec_pmu_percDiff_max[function_name]->SetBinContent(bin, XSec_pmu_percDiff_m1->GetBinContent(bin));
        }
      }

      // Find the bin where the cross-section in bigger
      int xsec_max_point_bin = xsec_nominal->GetMaximumBin();

      //XSec_pmu_percDiff_max[function_name]->GetXaxis()->SetRange(2,8);
      //double uncert = XSec_pmu_percDiff_max[function_name]->GetMaximum();
      //double uncertAtXSecPeak = XSec_pmu_percDiff_max[function_name]->GetBinContent(xsec_max_point_bin);
      //double maxBin = XSec_pmu_percDiff_max[function_name]->GetMaximumBin();
      //XSec_pmu_percDiff_max[function_name]->GetXaxis()->SetRangeUser(1,10);

      // Calulate sqrt(S+B)/S
      /*

      double POTscale = 5.3e19/2.43000e20;
      POTscale = 6.6e20/2.43000e20;
      std::cout << "******************* You are using 6.6e20 to scale. ===> Scale Factor: " << POTscale << std::endl;
      double S = pmu_numu_cc_reco_histo->Integral() * POTscale;
      double B = background_pmu_nominal->Integral() * POTscale;
    
      double POTscale_2 = 0.5e20/2.43000e20;
      double S_2 = pmu_numu_cc_reco_histo->Integral() * POTscale;
      double B_2 = background_pmu_nominal->Integral() * POTscale;
  
      TH1D * scoreFunction   = new TH1D("scoreFunctionAT6Point6e20POT",  ";p_{#mu};#sqrt{S+B}/S", 10, 0, 2);
      TH1D * scoreFunction_2 = new TH1D("scoreFunctionAT0Point5e20POT",  ";p_{#mu};#sqrt{S+B}/S", 10, 0, 2);    

      for (int bin = 1; bin < pmu_numu_cc_reco_histo->GetNbinsX()+1; bin++) {
        S = pmu_numu_cc_reco_histo->GetBinContent(bin) * POTscale;
        B = background_pmu_nominal->GetBinContent(bin) * POTscale;
        S_2 = pmu_numu_cc_reco_histo->GetBinContent(bin) * POTscale_2;
        B_2 = background_pmu_nominal->GetBinContent(bin) * POTscale_2;
        if (S == 0) {
          scoreFunction->SetBinContent(bin,0);
        }
        else
          scoreFunction->SetBinContent(bin,sqrt(S+B)/S*100);
          if (S_2 == 0) {
          scoreFunction_2->SetBinContent(bin,0);
        }
        else
          scoreFunction_2->SetBinContent(bin,sqrt(S_2+B_2)/S_2*100);
      }
    
      TFile tempFile = TFile("temp.root", "RECREATE");
      scoreFunction->Write();
      scoreFunction_2->Write();
      //t.Close();
    */

    

/*
      // Make the table now
      if (makeLaTeX){
        latexFile << " $ " << GetLegendName(function_name) 
                  << " $ & " << uncert 
                  << " &   " << scoreFunction->GetBinContent(maxBin) 
                  << " &   " << scoreFunction_2->GetBinContent(maxBin) << " \\\\ " << std::endl;
  
        latexFile2 << " $ " << GetLegendName(function_name)
                   << " $ & " << uncert
                   << " & " << uncertAtXSecPeak << " \\\\ " << std::endl;
      }
   

*/


      // Make the plots now
      TCanvas *c = new TCanvas("c", "canvas", 800, 700);

      c->SetBottomMargin(0.15);
      c->SetLeftMargin(0.15);
      c->SetLogy();

      XSec_pmu_percDiff_max[function_name]->SetMinimum(0.001);
      XSec_pmu_percDiff_max[function_name]->SetMaximum(100.);

      XSec_pmu_percDiff_max[function_name]->SetStats(0);   
      XSec_pmu_percDiff_max[function_name]->SetLineColor(kRed+1);
      XSec_pmu_percDiff_max[function_name]->GetXaxis()->SetTitle("p_{#mu} [GeV]");
      XSec_pmu_percDiff_max[function_name]->GetXaxis()->CenterTitle();
      //XSec_pmu_percDiff_max.at(function)->GetXaxis()->SetTitleSize(25);
      //XSec_pmu_percDiff_max.at(function)->GetXaxis()->SetTitleFont(43);
      XSec_pmu_percDiff_max[function_name]->GetXaxis()->SetTitleOffset(1.1);
      XSec_pmu_percDiff_max[function_name]->GetYaxis()->SetTitle("Percentage [%]");//("|Percental difference| [%]");
      XSec_pmu_percDiff_max[function_name]->GetYaxis()->CenterTitle();
      //XSec_pmu_percDiff_max.at(function)->GetYaxis()->SetTitleSize(25);
      //XSec_pmu_percDiff_max.at(function)->GetYaxis()->SetTitleFont(43);
      XSec_pmu_percDiff_max[function_name]->GetYaxis()->SetTitleOffset(1.1);
      XSec_pmu_percDiff_max[function_name]->GetYaxis()->SetNdivisions(516);
      XSec_pmu_percDiff_max[function_name]->Draw();
    
    /*
      scoreFunction->SetLineColor(9);
      scoreFunction->Draw("same");

      scoreFunction_2->SetLineColor(8);
      scoreFunction_2->Draw("same");
*/



      double x = 0.9;
      double y = 0.95;
      double size = 35;//28;
      int color = kRed+1;
      int font = 43;
      int align = 32;
      TString LaTeXname = GetLegendName(function_name);
      TLatex *latex = new TLatex( x, y, LaTeXname);
      latex->SetNDC();
      latex->SetTextSize(size);
      latex->SetTextColor(color);
      latex->SetTextFont(font);
      latex->SetTextAlign(align);

      latex->Draw();

/*
      TLegend* leg= new TLegend(0.1829574,0.648368,0.5639098,0.8486647,NULL,"brNDC");
 
      leg->SetTextFont(42);
      leg->SetBorderSize(0);
      leg->SetTextSize(0.02967359);

      leg->SetX1(0.179198);
      leg->SetX2(0.4924812);
      leg->SetY1(0.7091988);
      leg->SetY2(0.8531157);

      //leg->SetNColumns(3);

      leg->AddEntry(XSec_pmu_percDiff_max[function_name],  "|(#sigma-#sigma^{#pm})/#sigma|");

      //leg->AddEntry(scoreFunction,                       "Stat. @ 6.6e20 POT");
      //leg->AddEntry(scoreFunction_2,                     "Stat. @ 0.5e20 POT");
      //leg->SetNColumns(2);


      leg->Draw();
      */
      //leg->SetNColumns(2);

      //leg->SetX1(0.1829574);
      //leg->SetX2(0.5639098);
      //leg->SetY1(0.648368);
      //leg->SetY2(0.8486647);

      TString SaveName;
      SaveName = "Pmu_"+function_name;

      c->SetGridy();

      c->Print("./EvtWgtXsecDiffPlots/" + SaveName + ".C");
      c->Print("./EvtWgtXsecDiffPlots/" + SaveName + ".pdf");

      if (makeLaTeX) {
        latexFile3 << "\\subfloat[][]" << std::endl;
        latexFile3 << "   {\\includegraphics[width=.35\\textwidth]{images/EvtWgtXsecDiffPlots/" << SaveName << "}" << std::endl;
        latexFile3 << "   \\label{fig:" << "EvtWgtXsecDiffPlots_" << SaveName << "}} \\quad" << std::endl;
      }

      if (makeLaTeX && function_counter % 12 ==0) {
        latexFile3 << "\\caption{Cross Section Percental Difference Plots}" << std::endl;
        latexFile3 << "\\label{fig:EvtWgtXsecDiffPlots}" << std::endl;
        latexFile3 << "\\end{adjustwidth}" << std::endl;
        latexFile3 << "\\end{figure}" << std::endl;

        latexFile3 << "\\begin{figure}[t]" << std::endl;
        latexFile3 << "\\ContinuedFloat" << std::endl;
        latexFile3 << "\\begin{adjustwidth}{-1cm}{-1cm}" << std::endl;
        latexFile3 << "\\centering" << std::endl;

      }

      //if (function == loopMax-1) { // If we are at the end of the loop save stats.
        //latexFile2 << "Stat. @ 0.5e20 POT & - & " << scoreFunction_2->GetBinContent(xsec_max_point_bin) << " \\\\" << std::endl;
        //latexFile2 << "Stat. @ 6.6e20 POT & - & " << scoreFunction->GetBinContent(xsec_max_point_bin) << " \\\\" << std::endl;
      //}

    } // end loop functions

    if (makeLaTeX) {
      latexFile << "\\bottomrule" << std::endl;
      latexFile << "\\end{tabular}" << std::endl;
      latexFile << "\\end{table}" << std::endl;

      latexFile2 << "\\bottomrule" << std::endl;
      latexFile2 << "\\end{tabular}" << std::endl;
      latexFile2 << "\\end{table}" << std::endl;

      latexFile3 << "\\caption{Cross Section Percental Difference Plots}" << std::endl;
      latexFile3 << "\\label{fig:EvtWgtXsecDiffPlots}" << std::endl;
      latexFile3 << "\\end{adjustwidth}" << std::endl;
      latexFile3 << "\\end{figure}" << std::endl;
    }


  }

  TString ReweightingPlotter::GetLegendName(std::string fName) {
    
    TString legName = "null";
    if (fName.find("qema") != std::string::npos) legName = "M_{A}^{CCQE}";
    if (fName.find("ncelEta") != std::string::npos) legName = "#eta^{NCEL}";
    if (fName.find("ncelAxial") != std::string::npos) legName = "M_{A}^{NCEL}";
    if (fName.find("qevec") != std::string::npos) legName = "VecFF";
    if (fName.find("ccresAxial") != std::string::npos) legName = "M_{A}^{CCRES}";
    if (fName.find("ccresVector") != std::string::npos) legName = "M_{V}^{CCRES}";
    if (fName.find("ncresAxial") != std::string::npos) legName = "M_{A}^{NCRES}";
    if (fName.find("ncresVector") != std::string::npos) legName = "M_{V}^{NCRES}";
    if (fName.find("cohMA") != std::string::npos) legName = "M_{A}^{COH#pi}";
    if (fName.find("cohR0") != std::string::npos) legName = "R_{0}^{COH#pi}";
    if (fName.find("NonResRvp1pi") != std::string::npos) legName = "NonResRvp1pi";
    if (fName.find("NonResRvbarp1pi") != std::string::npos) legName = "NonResRvbarp1pi";
    if (fName.find("NonResRvp2pi") != std::string::npos) legName = "NonResRvp2pi";
    if (fName.find("NonResRvbarp2pi") != std::string::npos) legName = "NonResRvbarp2pi";
    if (fName.find("ResDecayGamma") != std::string::npos) legName = "BR(#gamma)";
    if (fName.find("ResDecayEta") != std::string::npos) legName = "BR(#eta)";
    if (fName.find("ResDecayTheta") != std::string::npos) legName = "Ang. distr.";
    if (fName.find("DISAth") != std::string::npos) legName = "A_{HT}^{BY}";
    if (fName.find("DISBth") != std::string::npos) legName = "B_{HT}^{BY}";
    if (fName.find("DISCv1u") != std::string::npos) legName = "C_{V1u}^{BY}";
    if (fName.find("DISCv2u") != std::string::npos) legName = "C_{V2u}^{BY}";
    if (fName.find("AGKYxF") != std::string::npos) legName = "x_{F}";
    if (fName.find("AGKYpT") != std::string::npos) legName = "p_{T}";
    if (fName.find("FormZone") != std::string::npos) legName = "FZ";
    if (fName.find("FermiGasModelKf") != std::string::npos) legName = "CCQE-PauliSup";
    if (fName.find("FermiGasModelSf") != std::string::npos) legName = "Fermi Gas to SF";
    if (fName.find("IntraNukeNmfp") != std::string::npos) legName = "x_{mfp}^{N}";
    if (fName.find("IntraNukeNcex") != std::string::npos) legName = "x_{cex}^{N}";
    if (fName.find("ntraNukeNel") != std::string::npos) legName = "x_{el}^{N}";
    if (fName.find("ntraNukeNinel") != std::string::npos) legName = "x_{inel}^{N}";
    if (fName.find("ntraNukeNabs") != std::string::npos) legName = "x_{abs}^{N}";
    if (fName.find("ntraNukeNpi") != std::string::npos) legName = "x_{pi}^{N}";
    if (fName.find("IntraNukePImfp") != std::string::npos) legName = "x_{mfp}^{PI}";
    if (fName.find("IntraNukePIcex") != std::string::npos) legName = "x_{cex}^{PI}";
    if (fName.find("ntraNukePIel") != std::string::npos) legName = "x_{el}^{PI}";
    if (fName.find("ntraNukePIinel") != std::string::npos) legName = "x_{inel}^{PI}";
    if (fName.find("ntraNukePIabs") != std::string::npos) legName = "x_{abs}^{PI}";
    if (fName.find("ntraNukePIpi") != std::string::npos) legName = "x_{pi}^{PI}";
  
    return legName;
  }




}


#endif
