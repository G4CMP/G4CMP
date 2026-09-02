void g4cmp_phonons_interface_hits(TString fileName, TString primaryType="electron", Int_t nShots=1) {
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

  TString yLabel = "Phonon count / " + primaryType;

  TFile *f = TFile::Open(fileName + ".root", "read");
  if (!f || f->IsZombie()) {
    std::cerr << "Error: could not open " << fileName << ".root" << std::endl;
    return;
  }

  TTree *hits = (TTree*)f->Get("InterfaceHits");
  if (!hits) {
    std::cerr << "Error: missing InterfaceHits tree" << std::endl;
    return;
  }

  const Int_t idL  = 1;
  const Int_t idTF = 2;
  const Int_t idTS = 3;

  const Color_t cL  = kRed;
  const Color_t cTF = kBlue;
  const Color_t cTS = kGreen+2;

  TCanvas *c1 = new TCanvas("c1","Phonon Interface Hits",1400,900);
  c1->SetFillColor(kWhite);
  c1->Divide(2,2,0.01,0.01);

  // ------------------------------------------------------------------
  // Top-left: phonon energy
  // ------------------------------------------------------------------
  c1->cd(1);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);
  gPad->SetLogy();

  hits->Draw("energy_eV>>hEL(120,0,0.04)", Form("weight*(speciesID==%d)", idL));
  hits->Draw("energy_eV>>hETF(120,0,0.04)", Form("weight*(speciesID==%d)", idTF), "same");
  hits->Draw("energy_eV>>hETS(120,0,0.04)", Form("weight*(speciesID==%d)", idTS), "same");

  TH1 *hEL  = (TH1*)gDirectory->Get("hEL");
  TH1 *hETF = (TH1*)gDirectory->Get("hETF");
  TH1 *hETS = (TH1*)gDirectory->Get("hETS");

  hEL->Scale(1.0 / nShots);
  hETF->Scale(1.0 / nShots);
  hETS->Scale(1.0 / nShots);

  hEL->SetTitle(Form("Phonon Energy at Si/Al Interface (%d shots, %s primary)", nShots, primaryType.Data()));
  hEL->GetXaxis()->SetTitle("Energy [eV]");
  hEL->GetYaxis()->SetTitle(yLabel);
  hEL->GetXaxis()->CenterTitle();
  hEL->GetYaxis()->CenterTitle();
  hEL->SetLineColor(cL);
  hETF->SetLineColor(cTF);
  hETS->SetLineColor(cTS);
  hEL->SetLineWidth(2);
  hETF->SetLineWidth(2);
  hETS->SetLineWidth(2);
  hEL->GetXaxis()->SetRangeUser(0, 0.05);
  hEL->GetYaxis()->SetRangeUser(1e-2, 1e5);

  TLegend *leg1 = new TLegend(0.65,0.70,0.88,0.88);
  leg1->SetBorderSize(0);
  leg1->SetFillStyle(0);
  leg1->AddEntry(hEL,  "phononL",  "l");
  leg1->AddEntry(hETF, "phononTF", "l");
  leg1->AddEntry(hETS, "phononTS", "l");
  leg1->Draw();

  // ------------------------------------------------------------------
  // Top-right: normal velocity
  // ------------------------------------------------------------------
  c1->cd(2);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);
  gPad->SetLogy();

  hits->Draw("vperp_mm_ns>>hVL(120,0,0.006)", Form("weight*(speciesID==%d)", idL));
  hits->Draw("vperp_mm_ns>>hVTF(120,0,0.006)", Form("weight*(speciesID==%d)", idTF), "same");
  hits->Draw("vperp_mm_ns>>hVTS(120,0,0.006)", Form("weight*(speciesID==%d)", idTS), "same");

  TH1 *hVL  = (TH1*)gDirectory->Get("hVL");
  TH1 *hVTF = (TH1*)gDirectory->Get("hVTF");
  TH1 *hVTS = (TH1*)gDirectory->Get("hVTS");

  hVL->Scale(1.0 / nShots);
  hVTF->Scale(1.0 / nShots);
  hVTS->Scale(1.0 / nShots);

  hVL->SetTitle(Form("Phonon Normal Velocity at Si/Al Interface (%d shots, %s primary)", nShots, primaryType.Data()));
  hVL->GetXaxis()->SetTitle("Normal velocity v_{#perp} [mm/ns]");
  hVL->GetYaxis()->SetTitle(yLabel);
  hVL->GetXaxis()->CenterTitle();
  hVL->GetYaxis()->CenterTitle();
  hVL->SetLineColor(cL);
  hVTF->SetLineColor(cTF);
  hVTS->SetLineColor(cTS);
  hVL->SetLineWidth(2);
  hVTF->SetLineWidth(2);
  hVTS->SetLineWidth(2);
  hVL->GetXaxis()->SetRangeUser(0, 0.006);
  hVL->GetYaxis()->SetRangeUser(10, 1e5);

  TLegend *leg2 = new TLegend(0.65,0.70,0.88,0.88);
  leg2->SetBorderSize(0);
  leg2->SetFillStyle(0);
  leg2->AddEntry(hVL,  "phononL",  "l");
  leg2->AddEntry(hVTF, "phononTF", "l");
  leg2->AddEntry(hVTS, "phononTS", "l");
  leg2->Draw();

  // ------------------------------------------------------------------
  // Bottom-left: arrival time
  // ------------------------------------------------------------------
  c1->cd(3);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);
  gPad->SetLogy();

  hits->Draw("time_ns>>hTL(120,0,1.5)", Form("weight*(speciesID==%d)", idL));
  hits->Draw("time_ns>>hTTF(120,0,1.5)", Form("weight*(speciesID==%d)", idTF), "same");
  hits->Draw("time_ns>>hTTS(120,0,1.5)", Form("weight*(speciesID==%d)", idTS), "same");

  TH1 *hTL  = (TH1*)gDirectory->Get("hTL");
  TH1 *hTTF = (TH1*)gDirectory->Get("hTTF");
  TH1 *hTTS = (TH1*)gDirectory->Get("hTTS");

  hTL->Scale(1.0 / nShots);
  hTTF->Scale(1.0 / nShots);
  hTTS->Scale(1.0 / nShots);

  TF1 *fL  = new TF1("fL",  "[0]*exp(-x/[1])", 0.1, 0.5);
  TF1 *fTF = new TF1("fTF", "[0]*exp(-x/[1])", 0.1, 0.5);
  TF1 *fTS = new TF1("fTS", "[0]*exp(-x/[1])", 0.1, 0.5);

  fL->SetLineColor(kRed);
  fTF->SetLineColor(kBlue);
  fTS->SetLineColor(kGreen+2);

  fL->SetLineWidth(2);
  fTF->SetLineWidth(2);
  fTS->SetLineWidth(2);

  // Initial guesses help
  fL->SetParameters(hTL->GetMaximum(), 0.5);
  fTF->SetParameters(hTTF->GetMaximum(), 0.5);
  fTS->SetParameters(hTTS->GetMaximum(), 0.5);

  hTL->Fit(fL, "R");
  hTTF->Fit(fTF, "R+");
  hTTS->Fit(fTS, "R+");
  
  hTL->SetTitle(Form("Phonon Arrival Time at Si/Al Interface (%d shots, %s primary)", nShots, primaryType.Data()));
  hTL->GetXaxis()->SetTitle("Arrival time [ns]");
  hTL->GetYaxis()->SetTitle(yLabel);
  hTL->GetXaxis()->CenterTitle();
  hTL->GetYaxis()->CenterTitle();
  hTL->SetLineColor(cL);
  hTTF->SetLineColor(cTF);
  hTTS->SetLineColor(cTS);
  hTL->SetLineWidth(2);
  hTTF->SetLineWidth(2);
  hTTS->SetLineWidth(2);

  hTL->GetXaxis()->SetRangeUser(0, 1.5);
  hTL->GetYaxis()->SetRangeUser(1, 1e5);

  TLegend *leg3 = new TLegend(0.65,0.70,0.88,0.88);
  leg3->SetBorderSize(0);
  leg3->SetFillStyle(0);
  leg3->AddEntry(hTL,  "phononL",  "l");
  leg3->AddEntry(hTTF, "phononTF", "l");
  leg3->AddEntry(hTTS, "phononTS", "l");
  leg3->Draw();

  // ------------------------------------------------------------------
  // Bottom-right: hit positions
  // ------------------------------------------------------------------
  c1->cd(4);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);

  TH2F *frame = new TH2F("frame","Phonon Hit Positions at Si/Al Interface",
                         100,-.5,.5,100,-.5,.5);
  frame->SetTitle(Form("Phonon Hit Positions at Si/Al Interface (%d shots, %s primary)", nShots, primaryType.Data()));
  frame->GetXaxis()->SetTitle("x [mm]");
  frame->GetYaxis()->SetTitle("y [mm]");
  frame->GetXaxis()->CenterTitle();
  frame->GetYaxis()->CenterTitle();
  frame->Draw();

  hits->SetMarkerStyle(20);
  hits->SetMarkerSize(0.005);

  hits->SetMarkerColor(cL);
  hits->Draw("y_mm:x_mm", Form("speciesID==%d", idL), "same");

  hits->SetMarkerColor(cTF);
  hits->Draw("y_mm:x_mm", Form("speciesID==%d", idTF), "same");

  hits->SetMarkerColor(cTS);
  hits->Draw("y_mm:x_mm", Form("speciesID==%d", idTS), "same");

  TLegend *leg4 = new TLegend(0.65,0.70,0.88,0.88);
  leg4->SetBorderSize(0);
  leg4->SetFillStyle(0);
  leg4->AddEntry((TObject*)0, "phononL: red", "");
  leg4->AddEntry((TObject*)0, "phononTF: blue", "");
  leg4->AddEntry((TObject*)0, "phononTS: green", "");
  leg4->Draw();

  c1->SaveAs(Form("figures/%s_interface_hits_summary.png", fileName.Data()));
}

