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

  void BootstrapTH1D::SetAllHistograms(std::map<std::string,TH1D*> input_map) 
  {

    _hmap = input_map;

    _nbins = _hmap["nominal"]->GetNbinsX();

  }

  int BootstrapTH1D::GetNbinsX()
  {
    return _nbins;
  }

  int BootstrapTH1D::GetNUniverses()
  {
    return _hmap.size() - 1;
  }

  void BootstrapTH1D::ResetIterator()
  {
    _current_iterator = _hmap.begin();
  }

  bool BootstrapTH1D::NextUniverse(std::string & uni_name, TH1D & uni_histo) {

    if (_current_iterator == _hmap.end()) {
      std::cout << "NextUniverse false" << std::endl;
      return false;
    }

    if (_current_iterator->first == "nominal") {
      _current_iterator++;
    }

    uni_name = _current_iterator->first;
    uni_histo = *_current_iterator->second;

    _current_iterator++;
    
    return true;
  }

  std::map<std::string, std::vector<TH1D*>> BootstrapTH1D::UnpackPMHisto()
  {

    std::map<std::string, std::vector<TH1D*>> output_map;

    for (auto iter : _hmap) {
      if (iter.first == "nominal") continue;
      //std::cout << "iter.first = " << iter.first << std::endl;

      // Minus 1 sigma
      std::size_t pos = iter.first.find("_m1");
      if (pos != std::string::npos) {  
        std::string function_name = iter.first.substr (0, pos);

        //std::cout << "Minus 1, substrig is " << function_name << std::endl;

        std::vector<TH1D*> vec;
        vec.resize(2);
        vec.at(0) = iter.second;

        output_map[function_name] = vec;
      }

      // Plus 1 sigma
      pos = iter.first.find("_p1");
      if (pos != std::string::npos) {  
        std::string function_name = iter.first.substr (0, pos);

        //std::cout << "Plus 1, substrig is " << function_name << std::endl;

        output_map[function_name].at(1) = iter.second;
      }
    }

    return output_map;
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
