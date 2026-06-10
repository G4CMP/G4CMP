void g4cmp_electrons_interface_hits(TString primaryType="electron", Int_t nShots=1) {
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

  TString yLabel = "Drift electron count / " + primaryType;

  TFile *f = TFile::Open("g4cmp_electrons.root");
  if (!f || f->IsZombie()) {
    std::cerr << "Error: could not open g4cmp_electrons.root" << std::endl;
    return;
  }

  TTree *hits = (TTree*)f->Get("InterfaceHits");
  TTree *events = (TTree*)f->Get("EventSummary");

  if (!hits || !events) {
    std::cerr << "Error: missing InterfaceHits or EventSummary tree" << std::endl;
    return;
  }

  // ---------- Canvas 1: interface-hit observables ----------
  TCanvas *c1 = new TCanvas("c1","Interface Hits",1400,900);
  c1->SetFillColor(kWhite);
  c1->Divide(2,2,0.01,0.01);

  // Panel 1: energy histogram
  c1->cd(1);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);
  gPad->SetLogy();

  hits->Draw("energy_eV>>hKE(100,0,2)", "weight*(speciesID==0)");
  TH1 *hKE = (TH1*)gDirectory->Get("hKE");
  hKE->Scale(1.0 / nShots);
  hKE->SetTitle(Form("Drift electron Energy at Si/Al Interface (%d shots, %s primary)", nShots, primaryType.Data()));
  hKE->GetXaxis()->SetTitle("Energy [eV]");
  hKE->GetYaxis()->SetTitle(yLabel);
  hKE->GetXaxis()->CenterTitle();
  hKE->GetYaxis()->CenterTitle();
  hKE->SetLineWidth(2);
  hKE->SetMinimum(5.0 / nShots);
  hKE->SetMaximum(200000.0 / nShots);

  // Panel 2: v_perp histogram
  c1->cd(2);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);

  hits->Draw("vperp_mm_ns>>hV(100,0,0.1)", "weight*(speciesID==0)");
  TH1 *hV = (TH1*)gDirectory->Get("hV");
  hV->Scale(1.0 / nShots);
  hV->SetTitle(Form("Drift electron Normal Velocity at Si/Al Interface (%d shots, %s primary)", nShots, primaryType.Data()));
  hV->GetXaxis()->SetTitle("Normal velocity v_{#perp} [mm/ns]");
  hV->GetYaxis()->SetTitle(yLabel);
  hV->GetXaxis()->CenterTitle();
  hV->GetYaxis()->CenterTitle();
  hV->SetLineWidth(2);
  hV->SetMinimum(0.);
  hV->SetMaximum(250000.0 / nShots);
  
  // Panel 3: arrival time histogram
  c1->cd(3);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);
  gPad->SetLogy();

  hits->Draw("time_ns>>hT(100,0,5)", "weight*(speciesID==0)");
  TH1 *hT = (TH1*)gDirectory->Get("hT");
  hT->Scale(1.0 / nShots);
  hT->SetTitle(Form("Drift electron Arrival Time at Si/Al Interface (%d shots, %s primary)", nShots, primaryType.Data()));
  hT->GetXaxis()->SetTitle("Arrival time [ns]");
  hT->GetYaxis()->SetTitle(yLabel);
  hT->GetXaxis()->CenterTitle();
  hT->GetYaxis()->CenterTitle();
  hT->SetLineWidth(2);
  hT->SetMinimum(5000.0 / nShots);
  hT->SetMaximum(2500000.0 / nShots);

  // Panel 4: weighted x-y hit map
  c1->cd(4);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);
  gPad->SetRightMargin(0.14);
  gPad->SetTopMargin(0.10);

  hits->Draw("y_mm:x_mm>>hXY(100,-0.5,0.5,100,-0.5,0.5)", "weight*(speciesID==0)", "colz");
  TH2 *hXY = (TH2*)gDirectory->Get("hXY");
  hXY->Scale(1.0 / nShots);
  hXY->SetTitle(Form("Drift electron Hit Positions at Si/Al Interface (%d shots, %s primary)", nShots, primaryType.Data()));
  hXY->GetXaxis()->SetTitle("x [mm]");
  hXY->GetYaxis()->SetTitle("y [mm]");
  hXY->GetZaxis()->SetTitle(yLabel);
  hXY->GetXaxis()->CenterTitle();
  hXY->GetYaxis()->CenterTitle();
  hXY->GetZaxis()->CenterTitle();
  hXY->GetXaxis()->SetLimits(-0.5, 0.5);
  hXY->GetYaxis()->SetRangeUser(-0.5, 0.5);

  c1->SaveAs("figures/interface_hits_summary.png");

  // ---------- Canvas 2: event summary ----------
  TCanvas *c2 = new TCanvas("c2","Event Summary",900,700);
  c2->SetFillColor(kWhite);
  c2->cd();
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);
  gPad->SetRightMargin(0.05);
  gPad->SetTopMargin(0.10);

  events->Draw("reachFraction>>hFrac(50,0,1)");
  TH1 *hFrac = (TH1*)gDirectory->Get("hFrac");
  hFrac->SetTitle(Form("Weighted Fraction of Drift electrons Reaching Si/Al Interface (%d shots, %s primary)", nShots, primaryType.Data()));
  hFrac->GetXaxis()->SetTitle("Reach fraction");
  hFrac->GetYaxis()->SetTitle("Events");
  hFrac->GetXaxis()->CenterTitle();
  hFrac->GetYaxis()->CenterTitle();
  hFrac->SetLineWidth(2);
  hFrac->SetMinimum(0.);
//hFrac->SetMaximum(240.);

  c2->SaveAs("figures/event_summary.png");
}

