/**
 * \file CorrelationCalculator2D.h
 *
 * 
 * \brief Class def header for a class CorrelationCalculator2D
 *
 * @author Marco Del Tutto
 */

/** \addtogroup UBXSec

    @{*/
#ifndef CorrelationCalculator2D_H
#define CorrelationCalculator2D_H

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
#include <TLegend.h>
#include <TStyle.h>
#include <TString.h>
#include <TMatrix.h>
#include <TObject.h>



#include "BootstrapTH1D.h"

namespace ubana{
  
  /**
   \class CorrelationCalculator2D
   User defined class CorrelationCalculator2D ... these comments are used to generate
   doxygen documentation!
 */

  class CorrelationCalculator2D /*: public TObject*/ {
    
  public:
    
    /// Default constructor
    CorrelationCalculator2D();

    virtual ~CorrelationCalculator2D();

    ///
    void SetBootstrap(ubana::BootstrapTH1D);

    ///
    void CalculateCovarianceMatrix();

    ///
    void PlotMatrices();


  protected:

    std::string _name = "[CorrelationCalculator2D] ";

    ubana::BootstrapTH1D _bs;

    TMatrix _M; ///< The covariance matrix
    TMatrix _M_frac; ///< The fractional covariance matrix

    bool _verbose = true;

    
  };
}

#endif
/** @} */ // end of doxygen group 

