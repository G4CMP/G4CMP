void g4cmp_phonons_pairbreaking_summary(TString fileName, TString primaryType="electron", Int_t nShots=1) {
  gSystem->mkdir("figures", kTRUE);

  gStyle->SetOptStat(0);
  gStyle->SetTitleFontSize(0.045);
  gStyle->SetLabelSize(0.04, "XY");
  gStyle->SetTitleSize(0.045, "XY");
  gStyle->SetTitleOffset(1.1, "X");
  gStyle->SetTitleOffset(1.2, "Y");
  gStyle->SetPadGridX(true);
  gStyle->SetPadGridY(true);
  gStyle->SetCanvasColor(kWhite);
  gStyle->SetPadColor(kWhite);
  gStyle->SetFrameFillColor(kWhite);
  gStyle->SetTitleFillColor(kWhite);

  if (nShots <= 0) {
    std::cerr << "Error: nShots must be > 0" << std::endl;
    return;
  }

  TFile *f = TFile::Open(fileName + ".root", "read");
  if (!f || f->IsZombie()) {
    std::cerr << "Error: could not open " << fileName << ".root" << std::endl;
    return;
  }

  TTree *events = (TTree*)f->Get("PhononEventSummary");
  if (!events) {
    std::cerr << "Error: missing PhononEventSummary tree" << std::endl;
    return;
  }
  // ------------------------------------------------------------------
  // Compute summary statistics
  // ------------------------------------------------------------------
  Double_t meanEnergyAbove2Delta = 0.;
  Double_t meanCountAbove2Delta = 0.;

  Double_t totalEnergyPerShot = 0.0;

  // More robust total mean from explicit event sum
  Double_t eAbove = 0.0;
  events->SetBranchAddress("ePhononsAbove2DeltaWeighted_eV", &eAbove);
  Double_t nAbove = 0.;
  events->SetBranchAddress("nPhononsAbove2DeltaWeighted", &nAbove);

  Long64_t nEntries = events->GetEntries();
  Double_t totalEnergy = 0.0;
  for (Long64_t i = 0; i < nEntries; ++i) {
    events->GetEntry(i);
    totalEnergy += eAbove;
    meanEnergyAbove2Delta += eAbove;
    meanCountAbove2Delta += nAbove;
  }
  if (nEntries > 0) meanEnergyAbove2Delta /= nEntries;
  if (nEntries > 0) meanCountAbove2Delta /= nEntries;
  Double_t meanEnergyPerShot = meanEnergyAbove2Delta; // if 1 event = 1 shot

  if (nShots > 0) totalEnergyPerShot = totalEnergy / nShots;

  std::cout << "====================================================\n";
  std::cout << "Phonon pair-breaking summary\n";
  std::cout << "Primary type: " << primaryType << "\n";
  std::cout << "Number of shots: " << nShots << "\n";
  std::cout << "Number of events in tree: " << nEntries << "\n";
  std::cout << "Mean weighted phonon energy above 2Delta per event [eV]: "
            << meanEnergyAbove2Delta << "\n";
  std::cout << "Mean weighted phonon count above 2Delta per event: "
            << meanCountAbove2Delta << "\n";
  std::cout << "Total weighted phonon energy above 2Delta / shot [eV]: "
            << totalEnergyPerShot << "\n";
  std::cout << "====================================================\n";

  // ------------------------------------------------------------------
  // Plotting
  // ------------------------------------------------------------------
  TCanvas *c1 = new TCanvas("c1","Phonon Pair-Breaking Summary",1200,600);
  c1->SetFillColor(kWhite);
  c1->Divide(2,1,0.01,0.01);
  c1->cd(1);
  // ----------------------------------------------------------
  // Left panel: weighted phonon energy above 2Delta
  // ----------------------------------------------------------
  c1->cd(1);
  gPad->SetLeftMargin(0.14);
  gPad->SetBottomMargin(0.13);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);
  events->Draw("ePhononsAbove2DeltaWeighted_eV>>hEpb");
  TH1 *hEpb = (TH1*)gDirectory->Get("hEpb");
  hEpb->SetTitle(Form("Weighted Phonon Energy Above 2#Delta at Interface (%d shots, %s primary)",
                      nShots, primaryType.Data()));
  hEpb->GetXaxis()->SetTitle("Weighted phonon energy above 2#Delta at interface [eV/event]");
  hEpb->GetYaxis()->SetTitle("Events");
  hEpb->GetXaxis()->CenterTitle();
  hEpb->GetYaxis()->CenterTitle();
  hEpb->SetLineWidth(2);

  TLine *lineE = new TLine(meanEnergyAbove2Delta, 0,
                           meanEnergyAbove2Delta, hEpb->GetMaximum());
  lineE->SetLineColor(kRed+1);
  lineE->SetLineWidth(2);
  lineE->SetLineStyle(2);
  lineE->Draw("same");

  TLegend *leg1 = new TLegend(0.52,0.72,0.88,0.88);
  leg1->SetBorderSize(0);
  leg1->SetFillStyle(0);
  leg1->AddEntry(hEpb, "Event distribution", "l");
  leg1->AddEntry(lineE, Form("Mean = %.4g eV/event", meanEnergyAbove2Delta), "l");
  leg1->Draw();

  // ----------------------------------------------------------
  // Right panel: weighted phonon count above 2Delta
  // ----------------------------------------------------------
  c1->cd(2);
  gPad->SetLeftMargin(0.14);
  gPad->SetBottomMargin(0.13);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);

  events->Draw("nPhononsAbove2DeltaWeighted>>hNpb");
  TH1 *hNpb = (TH1*)gDirectory->Get("hNpb");
  hNpb->SetTitle(Form("Weighted Phonon Count Above 2#Delta at Interface (%d shots, %s primary)",
                      nShots, primaryType.Data()));
  hNpb->GetXaxis()->SetTitle("Weighted phonon count above 2#Delta at interface [per event]");
  hNpb->GetYaxis()->SetTitle("Events");
  hNpb->GetXaxis()->CenterTitle();
  hNpb->GetYaxis()->CenterTitle();
  hNpb->SetLineWidth(2);

  TLine *lineN = new TLine(meanCountAbove2Delta, 0,
                           meanCountAbove2Delta, hNpb->GetMaximum());
  lineN->SetLineColor(kBlue+1);
  lineN->SetLineWidth(2);
  lineN->SetLineStyle(2);
  lineN->Draw("same");

  TLegend *leg2 = new TLegend(0.50,0.72,0.88,0.88);
  leg2->SetBorderSize(0);
  leg2->SetFillStyle(0);
  leg2->AddEntry(hNpb, "Event distribution", "l");
  leg2->AddEntry(lineN, Form("Mean = %.4g /event", meanCountAbove2Delta), "l");
  leg2->Draw();

  c1->SaveAs(Form("figures/%s_phonons_pairbreaking_summary.png", fileName.Data()));
}
