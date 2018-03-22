#ifndef CorrelationCalculator2D_CXX
#define CorrelationCalculator2D_CXX

#include "../Include/CorrelationCalculator2D.h"

//ClassImp(CorrelationCalculator2D)

namespace ubana {

  CorrelationCalculator2D::CorrelationCalculator2D() /*: TObject()*/
  {
  }

  CorrelationCalculator2D::~CorrelationCalculator2D()
  {   
  }

  

  void CorrelationCalculator2D::SetBootstrap(ubana::BootstrapTH1D bs)
  {

    _bs = bs;
    
  }

  void CorrelationCalculator2D::CalculateCovarianceMatrix() 
  {

    std::cout << "herea" << std::endl;

    _M.Clear();
    _M.ResizeTo(_bs.GetNbinsX(), _bs.GetNbinsX());

    _M_frac.Clear();
    _M_frac.ResizeTo(_bs.GetNbinsX(), _bs.GetNbinsX());

    for (int i = 0; i < _bs.GetNbinsX(); i++) {
      for (int j = 0; j < _bs.GetNbinsX(); j++) {
       _M[i][j] = 0.;
       _M_frac[i][j] = 0.;
      }
    }


    _bs.ResetIterator();

    for (int i = 0; i < _bs.GetNbinsX(); i++) {

      for (int j = 0; j < _bs.GetNbinsX(); j++) {

        _bs.ResetIterator();

        for (int s = 0; s < _bs.GetNUniverses(); s++) {

          std::string uni_name;
          TH1D uni_histo;
          _bs.NextUniverse(uni_name, uni_histo);

          double N_i_s = uni_histo.GetBinContent(i+1);
          double N_i_cv = _bs.GetNominal()->GetBinContent(i+1);
          double N_j_s = uni_histo.GetBinContent(j+1);
          double N_j_cv = _bs.GetNominal()->GetBinContent(i+1);

          _M[i][j] += (N_i_s - N_i_cv) * (N_j_s - N_j_cv) / (double)_bs.GetNUniverses();

          _M_frac[i][j] += (N_i_s - N_i_cv) * (N_j_s - N_j_cv) / ((double)_bs.GetNUniverses() * N_i_cv * N_j_cv);

        } // universe loop
      } // bin j loop
    } // bin i loop

    if (_verbose) {
      std::cout << _name << "Printing Covariance Matrix M = " << std::endl;
      _M.Print();
      std::cout << _name << "Printing Fractional Covariance Matrix M = " << std::endl;
      _M_frac.Print();
    }

  }


  void CorrelationCalculator2D::PlotMatrices()
  {

    TH2D * cov_matrix_histo = new TH2D("cov_matrix_histo", "", _bs.GetNbinsX(), 0, _bs.GetNbinsX(), _bs.GetNbinsX(), 0, _bs.GetNbinsX());

    for (int i = 0; i < _bs.GetNbinsX(); i++) { 
      for (int j = 0; j < _bs.GetNbinsX(); j++) {
        cov_matrix_histo->SetBinContent(i+1, j+1, _M_frac[i][j]);
      }
    } 

    gStyle->SetPaintTextFormat("4.5f");

    TCanvas * c_smatrix = new TCanvas;
    cov_matrix_histo->SetMarkerColor(kWhite);
    cov_matrix_histo->SetMarkerSize(1.8);
    //cov_matrix_histo->GetXaxis()->SetBinLabel(1, "test");
    cov_matrix_histo->GetXaxis()->CenterTitle();
    cov_matrix_histo->GetYaxis()->CenterTitle();
    cov_matrix_histo->GetXaxis()->SetTitle("Bin i");
    cov_matrix_histo->GetYaxis()->SetTitle("Bin j");

    cov_matrix_histo->Draw("colz TEXT");
    TString name = "cov_frac_matrix_2d";
    c_smatrix->SaveAs(name + ".pdf");

  }

 


}


#endif
