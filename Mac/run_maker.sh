#! /bin/bash

Maker --filename ../Files/ubxsec_output_mc_bnbcosmic_mcc8.7_test1.root --maxentries -1 --flashstart 3.2 --flashend 4.8 -g 198 -p
mv ubxsecana_output.root ubxsecana_output_bnbcosmic_mcc8.7_test1.root

Maker --filename ../Files/ubxsec_output_data_bnbon_mcc8.7_test1.root --maxentries -1 --flashstart 3.3 --flashend 4.9 -g 243 -d
mv ubxsecana_output.root ubxsecana_output_bnbon_mcc8.7_test1.root

Maker --filename ../Files/ubxsec_output_data_extbnb_mcc8.7_test1.root --maxentries -1 --flashstart 3.65 --flashend 5.25 -s 0.406 -g 243 -d
mv ubxsecana_output.root ubxsecana_output_extbnb_mcc8.7_test1.root




#Maker --filename ../evt_sel/files/mcc8.7_test/ubxsec_output_mc_bnbnuecosmic_mcc8.6_stopmu_test16.root --maxentries -1 --flashstart 3.2 --flashend 4.8 -g 198 -p
#mv ubxsecana_output.root ubxsecana_output_bnbnuecosmic_mcc8.7_test15.root

#Maker --filename ../evt_sel/files/mcc8.6_v2.0.2/ubxsec_output_mc_intimecosmic_mcc8.6_v2.0.2.root --maxentries -1 --flashstart 3.65 --flashend 5.25 -g 198 -s 0.406
#mv ubxsecana_output.root ubxsecana_output_intimecosmic_mcc8.6_v2.0.2.root
