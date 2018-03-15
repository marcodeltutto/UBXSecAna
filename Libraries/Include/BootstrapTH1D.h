/**
 * \file BootstrapTH1D.h
 *
 * 
 * \brief Class def header for a class BootstrapTH1D
 *
 * @author Marco Del Tutto
 */

/** \addtogroup UBXSec

    @{*/
#ifndef BOOTSTRAPTH1D_H
#define BOOTSTRAPTH1D_H

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
   \class BootstrapTH1D
   User defined class BootstrapTH1D ... these comments are used to generate
   doxygen documentation!
 */

  class BootstrapTH1D {
    
  public:
    
    /// Default constructor
    BootstrapTH1D();

    /// Constructor
    BootstrapTH1D(std::string, std::string, int, double*);

    /// Default destructor
    ~BootstrapTH1D(){}

    ///
    void SetWeightNames(std::vector<std::string>);

    /// Fill the value bin with a general weight and the vector of weights
    void Fill(double, double, std::vector<double>);

  
  protected:

    std::string _name = "[BootstrapTH1D] ";

    std::string _hname; ///< Histogram name
    std::string _title; ///< Histogram title
    int _nbins; ///< Histogram n bins
    double *_bins; ///< Histogram bins array

    std::map<std::string, TH1D*> _hmap; ///< Histogram map ("nominal", "universe1"...)
    size_t _n_weights; ///< Number of weigths to use
    std::vector<double> _weights; ///< Weigths to use
    std::vector<std::string>_wnames; ///< Weight names

    
  };
}

#endif
/** @} */ // end of doxygen group 

