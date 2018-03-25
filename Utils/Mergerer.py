#!/usr/bin/env python
#

import os,sys,string, time
from subprocess import call
import ROOT
from math import *
from ROOT import *
from array import array
from glob import glob

fname = glob("/data/t2k/lar/uboone/prodgenie_numi_nu_uboone_MCC7/prodgenie_numi_nu_cosmic_uboone_0*_gen_g4_detsim_reco1_reco2_ana.root")
#fname = '/data/t2k/lar/uboone/prodgenie_numi_nu_uboone_MCC7/prodgenie_numi_nu_cosmic_uboone_00817_gen_g4_detsim_reco1_reco2_ana.root', '/data/t2k/lar/uboone/prodgenie_numi_nu_uboone_MCC7/prodgenie_numi_nu_cosmic_uboone_00784_gen_g4_detsim_reco1_reco2_ana.root'
#print fname

chain   = TChain("analysistree/anatree")
chainPOT = TChain("analysistree/pottree")
fileCounter = 0
for f in fname:
  fileCounter += 1
  chain.Add(f)
  chainPOT.Add(f)

print "chain.GetNtrees()    = ", chain.GetNtrees()
print "chainPOT.GetNtrees() = ", chainPOT.GetNtrees()


# Merging files
mergedFile = ROOT.TFile("/data/t2k/lar/uboone/prodgenie_numi_nu_uboone_MCC7/prodgenie_numi_nu_cosmic_uboone_merged_gen_g4_detsim_reco1_reco2_ana.root", "RECREATE");
mergedFile.mkdir("analysistree").cd();

retVal = chain.Merge(mergedFile,-1)
if retVal < 0:
  print "Error in merging files. Exiting."
  exit(0) 
print fileCounter, " files merged (anatree)."

mergedFile = ROOT.TFile("/data/t2k/lar/uboone/prodgenie_numi_nu_uboone_MCC7/prodgenie_numi_nu_cosmic_uboone_merged_gen_g4_detsim_reco1_reco2_ana.root", "UPDATE");
mergedFile.cd("analysistree");
retVal2 = chainPOT.Merge(mergedFile,-1)
if retVal2 < 0:
  print "Error in merging files. Exiting."
  exit(0)
print fileCounter, " files merged (pottree)."


raw_input("Please press enter to exit.")



