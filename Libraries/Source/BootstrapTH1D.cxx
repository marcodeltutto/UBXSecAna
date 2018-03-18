#ifndef BOOTSTRAPTH1D_CXX
#define BOOTSTRAPTH1D_CXX

#include "../Include/BootstrapTH1D.h"

//ClassImp(BootstrapTH1D)

namespace ubana {

  BootstrapTH1D::BootstrapTH1D() /*: TObject()*/
  {
  }

  BootstrapTH1D::BootstrapTH1D(std::string name, std::string title, int nbins, double *bins) /*: TObject()*/
  {
    _hmap["nominal"] = new TH1D(name.c_str(), title.c_str(), nbins, bins);
    _hname = name;
    _title = title;
    _nbins = nbins;
    _bins.resize(nbins);
    for (size_t i = 0; i < _bins.size(); i++) {
      _bins.at(i) = bins[i];
    }
  }

  BootstrapTH1D::~BootstrapTH1D()
  {
     
  }

  void BootstrapTH1D::SetWeightNames(std::vector<std::string> names)
  {

    _n_weights = names.size();
    _wnames = names;
    double* bins = &_bins[0];
    for (size_t i = 0; i < _n_weights; i++) {
      _hmap[names.at(i)] = new TH1D((_hname+names.at(i)).c_str(), _title.c_str(), _nbins, bins);
    }
    
  }

  void BootstrapTH1D::Fill(double value, double weight, std::vector<double> weights)
  {

    if (weights.size() != _n_weights) {
      std::cout << __PRETTY_FUNCTION__ << " Size mismatch, this weight vector has size " << weights.size() 
                << ", but " << _n_weights << " is expected." << std::endl;
      throw std::exception();
    }

    _hmap["nominal"]->Fill(value, weight);

    for (size_t i = 0; i < _n_weights; i++) {

     _hmap[_wnames.at(i)]->Fill(value, weight * weights.at(i));

   }
    
  }

  TH1D* BootstrapTH1D::GetNominal()
  {
    return _hmap["nominal"];
  }


  


}


#endif
