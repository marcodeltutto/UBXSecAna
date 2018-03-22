/**
 * \file MigrationMatrix2D.h
 *
 * 
 * \brief Class def header for a class MigrationMatrix2D
 *
 * @author Marco Del Tutto
 */

/** \addtogroup UBXSec

    @{*/
#ifndef MIGRATIONMATRIX2D_H
#define MIGRATIONMATRIX2D_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <map>
#include <time.h>

#include <TSystem.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLatex.h>
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
#include <TFile.h>
#include <TMath.h>
#include "Math/SMatrix.h"
#include "TMatrix.h"
#include "TLine.h"
#include "TGaxis.h"
#include "TMatrix.h"

#include "Types.h"


namespace ubana{
  
  /**
   \class MigrationMatrix2D
   User defined class MigrationMatrix2D ... these comments are used to generate
   doxygen documentation!
 */

  class MigrationMatrix2D {
    
  public:
    
    /// Default constructor
    MigrationMatrix2D();

    /// Default destructor
    ~MigrationMatrix2D(){}

    ///
    TMatrix CalculateMigrationMatrix(); 

    ///
    void SetTrueRecoHistogram(TH2D *h);

    ///
    void SetNBins(int n_var1_bins, int n_var2_bins);

    ///
    void SetOutputFileName(std::string name);

    ///
    void PrintSmearingMatrixLatex();

    /// Plots S as an histogram
    void PlotMatrix();

  
  protected:

    std::string _name = "[MigrationMatrix2D] ";

    bool _configured = false;

    TTree *_tree;

    //std::vector<std::pair<double, double>> _var1_bins;
    //std::vector<std::pair<double, double>> _var2_bins;
    int _n, _m;

    TH2D *_h_true_reco_mom;

    TMatrix _S; ///< The smearing matrix

    std::ofstream _f_out; ///< The output file

    std::string _folder = "MigrationMatrix2DPlots/";

    bool _verbose = false;

    
  };
}

#endif
/** @} */ // end of doxygen group 

