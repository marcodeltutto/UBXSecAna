// To set this as default, you need a .rootrc file in your home directory,
// containing the following line:
// Rint.Logon: /full/path/to/rootlogon.C

void rootlogon()
{
  printf("Welcome to the ROOT of all evils \n");

  // Defaults to classic style, but that's OK, we can fix it
  TStyle* ubStyle = new TStyle("ubStyle", "MicroBooNE Style");

  // Centre title
  ubStyle->SetTitleAlign(22);
  ubStyle->SetTitleX(.5);
  ubStyle->SetTitleY(.95);
  ubStyle->SetTitleBorderSize(0);

  // No info box
  ubStyle->SetOptStat(0);
  //ubStyle->SetOptStat("emri");

  //set the background color to white
  ubStyle->SetFillColor(10);
  ubStyle->SetFrameFillColor(10);
  ubStyle->SetCanvasColor(10);
  ubStyle->SetPadColor(10);
  ubStyle->SetTitleFillColor(0);
  ubStyle->SetStatColor(10);

  // Don't put a colored frame around the plots
  ubStyle->SetFrameBorderMode(0);
  ubStyle->SetCanvasBorderMode(0);
  ubStyle->SetPadBorderMode(0);

  // Set the default line color for a fit function to be red
  ubStyle->SetFuncColor(kRed);

  // Marker settings
  //  ubStyle->SetMarkerStyle(kFullCircle);

  // No border on legends
  ubStyle->SetLegendBorderSize(0);

  // Scientific notation on axes
  //  TGaxis::SetMaxDigits(3);

  // Axis titles
  ubStyle->SetTitleSize(.055, "xyz");
  ubStyle->SetTitleOffset(.8, "xyz");
  // More space for y-axis to avoid clashing with big numbers
  ubStyle->SetTitleOffset(.9, "y");
  // This applies the same settings to the overall plot title
  ubStyle->SetTitleSize(.055, "");
  ubStyle->SetTitleOffset(.8, "");
  // Axis labels (numbering)
  ubStyle->SetLabelSize(.04, "xyz");
  ubStyle->SetLabelOffset(.005, "xyz");

  // Prevent ROOT from occasionally automatically zero-suppressing
  ubStyle->SetHistMinimumZero();

  // Thicker lines
  ubStyle->SetHistLineWidth(2);
  ubStyle->SetFrameLineWidth(2);
  ubStyle->SetFuncWidth(2);

  // Set the number of tick marks to show
  ubStyle->SetNdivisions(506, "xyz");

  // Set the tick mark style
  ubStyle->SetPadTickX(0);
  ubStyle->SetPadTickY(0);

  // Fonts
  const int kNovaFont = 42;
  ubStyle->SetStatFont(kNovaFont);
  ubStyle->SetLabelFont(kNovaFont, "xyz");
  ubStyle->SetTitleFont(kNovaFont, "xyz");
  ubStyle->SetTitleFont(kNovaFont, ""); // Apply same setting to plot titles
  ubStyle->SetTextFont(kNovaFont);
  ubStyle->SetLegendFont(kNovaFont);

  // Get moodier colours for colz
  const Int_t NRGBs = 5;
  const Int_t NCont = 255;
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  ubStyle->SetNumberContours(NCont);

  gROOT->SetStyle("ubStyle");

  // Uncomment this line if you want to force all plots loaded from files
  // to use this same style
  gROOT->ForceStyle();
}


// Put a "uBooNE Simulation" tag in the corner
void uBooNESimulation()
{
  TLatex* prelim = new TLatex(.9, .95, "MicroBooNE Simulation");
  prelim->SetTextColor(kGray+1);
  prelim->SetNDC();
  prelim->SetTextSize(2/30.);
  prelim->SetTextAlign(32);
  prelim->Draw();
}
void uBooNESimulation_2()
{
  TLatex* prelim = new TLatex(.95, .95, "MicroBooNE Simulation, Preliminary");
  prelim->SetTextColor(kGray+1);
  prelim->SetNDC();
  prelim->SetTextSize(1/30.);
  prelim->SetTextAlign(32);
  prelim->Draw();
}

// Put a "MicroBooNE Simulation" tag in the corner
void Simulation()
{
  TLatex* prelim = new TLatex(.9, .95, "MicroBooNE Simulation");
  prelim->SetTextColor(kGray+1);
  prelim->SetNDC();
  prelim->SetTextSize(2/30.);
  prelim->SetTextAlign(32);
  prelim->Draw();
}

void CenterTitles(TH1* histo)
{
  histo->GetXaxis()->CenterTitle();
  histo->GetYaxis()->CenterTitle();
  histo->GetZaxis()->CenterTitle();  
}
