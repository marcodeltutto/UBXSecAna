#ifndef MIGRATIONMATRIX2D_CXX
#define MIGRATIONMATRIX2D_CXX

#include "MigrationMatrix2D.h"

namespace ubana {

  MigrationMatrix2D::MigrationMatrix2D()
  {
  }

  void MigrationMatrix2D::SetOutputFileName(std::string name) 
  {
    _f_out.open(name, std::ios::out | std::ios::trunc);
  }

  void MigrationMatrix2D::SetTrueRecoHistogram(TH2D *h)
  {
    _h_true_reco_mom = h;
  }

  void MigrationMatrix2D::SetNBins(int n_var1_bins, int n_var2_bins)
  {

     _n = n_var1_bins;
     _m = n_var2_bins;
    
  }


  TMatrix MigrationMatrix2D::CalculateMigrationMatrix() 
  {
   
    // n x m matrix
    // n = number of rows
    // m = number of columns


    //std::vector<double> data_v;
    //data_v.resize(_n * _m);

    //TMatrix S;
    _S.Clear();
    _S.ResizeTo(_n, _m);



    for (int j = 1; j < _m + 1; j++) {    // True bin

      std::cout << "\tThis is true bin " << j << std::endl;

      std::vector<double> p_v;
      p_v.resize(_n + 1);

      double sum = 0;

      for (int i = 1; i < _n + 1; i++) {      // Reco bin

        std::cout << "This is reco bin " << i << std::endl;

        p_v.at(i) = _h_true_reco_mom->GetBinContent(j, i);
        sum += p_v.at(i);

        std::cout << "\tValue is " << p_v.at(i) << std::endl;


      } // reco bin

      std::cout << "\t>>> Sum is " << sum << std::endl;

      double tot_prob = 0;

      for (int i = 1; i < _n + 1; i++) {
        p_v.at(i) /= sum;

        std::cout << "\t\tProbability at " << i << " is " << p_v.at(i) << std::endl;
        tot_prob += p_v.at(i);

        //int row_offset = (i-1)*n;
        //data_v.at(row_offset + j-1) = p_v.at(j);
        _S[i - 1][j - 1] = p_v.at(i);
      }
      std::cout << "\t\t> Total Probability is " << tot_prob << std::endl;

    } // true bin
    

    std::cout << _name << "Migration Matrix: " << std::endl;
    _S.Print();

    return _S;

  }

  void MigrationMatrix2D::PlotMatrix()
  {

    TH2D * smearing_matrix_histo = new TH2D("smearing_matrix_histo", "", _m, 0, _m, _n, 0, _n);

    for (int i = 0; i < _n; i++) { 
      for (int j = 0; j < _m; j++) {
        smearing_matrix_histo->SetBinContent(j+1, i+1, _S[i][j]);
      }
    } 

    gStyle->SetPaintTextFormat("4.2f");

    TCanvas * c_smatrix = new TCanvas;
    smearing_matrix_histo->SetMarkerColor(kWhite);
    smearing_matrix_histo->SetMarkerSize(2.0);
    //smearing_matrix_histo->GetXaxis()->SetBinLabel(1, "test");
    smearing_matrix_histo->GetXaxis()->CenterTitle();
    smearing_matrix_histo->GetYaxis()->CenterTitle();
    smearing_matrix_histo->GetXaxis()->SetTitle("True Bin j");
    smearing_matrix_histo->GetYaxis()->SetTitle("Reco Bin i");

    smearing_matrix_histo->Draw("col TEXT");
    TString name = _folder +_name + "smearing_matrix_2d";
    c_smatrix->SaveAs(name + ".pdf");

  }



  void MigrationMatrix2D::PrintSmearingMatrixLatex()
  {


    if (!_f_out.is_open()) {
      std::cout << "File not opened." << std::endl;
      return;
    }

    _f_out << "\\begin{equation}" << std::endl;
    _f_out << "S_{ij} =" << std::endl;
    _f_out << "\\begin{bmatrix}" << std::endl;

    for (int i = 0; i < _n; i++) {
      for (int j = 0; j < _m; j++) {

        _f_out << _S[i][j] << "  &  ";

      }

      _f_out << " \\\\" << std::endl;
    }

    _f_out << "\\end{bmatrix}" << std::endl;
    _f_out << "\\end{equation}" << std::endl << std::endl;

  }



}


#endif
