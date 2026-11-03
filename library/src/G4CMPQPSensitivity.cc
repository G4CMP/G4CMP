/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/
// 20261103 G4CMP-692 -- Initial implementation

#include "G4CMPQPSensitivity.hh"
#include "G4CMPQPHit.hh"
#include "G4CMPUtils.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"

#include "Randomize.hh"

G4CMPQPSensitivity::G4CMPQPSensitivity(G4String name)
  : G4VSensitiveDetector(name), hitsCollection(nullptr), waveform(nullptr), numSamples(1000) {
  collectionName.insert("G4CMPQPHit");
  collectionName.insert("G4CMPQPWave");
}

G4CMPQPSensitivity::G4CMPQPSensitivity(G4CMPQPSensitivity&& in) :
  G4VSensitiveDetector(std::move(in)),
  hitsCollection(in.hitsCollection), waveform(in.waveform),
  numSamples(in.numSamples) {
}

G4CMPQPSensitivity& G4CMPQPSensitivity::operator=(G4CMPQPSensitivity&& in) {
  // Base class members
  SensitiveDetectorName = std::move(in.SensitiveDetectorName);
  thePathName = std::move(in.thePathName);
  fullPathName = std::move(in.fullPathName);
  verboseLevel = in.verboseLevel;
  active = in.active;
  std::swap(ROgeometry, in.ROgeometry);
  std::swap(filter, in.filter);

  // Our members
  hitsCollection = in.hitsCollection;
  waveform = in.waveform;
  numSamples = in.numSamples;

  return *this;
}

void G4CMPQPSensitivity::Initialize(G4HCofThisEvent* HCE) {
  hitsCollection = new G4CMPQPHitsCollection(SensitiveDetectorName,
                                                    collectionName[0]);
  waveform = new G4CMPQPWaveform(SensitiveDetectorName,
                                                    collectionName[1]);
  G4SDManager* sdman = G4SDManager::GetSDMpointer();
  G4int HCID = sdman->GetCollectionID(hitsCollection);
  HCE->AddHitsCollection(HCID, hitsCollection);
  HCID = sdman->GetCollectionID(waveform);
  HCE->AddHitsCollection(HCID, waveform);
}

void G4CMPQPSensitivity::FillHitStart(const G4Step* step, G4CMPQPHit* hit)
{
  G4Track* track = step->GetTrack();
  hit->SetTrackID(track->GetTrackID());
  hit->SetStepID(track->GetCurrentStepNumber());
  hit->SetStartTime(track->GetGlobalTime());
  hit->SetStartPosition(track->GetPosition());
}

void G4CMPQPSensitivity::FillHitFinal(const G4Step* step, G4CMPQPHit* hit)
{
  G4Track* track = step->GetTrack();
  hit->SetStepID(track->GetCurrentStepNumber());
  hit->SetFinalTime(track->GetGlobalTime());
  hit->SetFinalPosition(track->GetPosition());
  hit->SetWeight(track->GetWeight());
}

void G4CMPQPSensitivity::FillHit(const G4Step* step, G4CMPQPHit* hit)
{
  G4CMPQPSensitivity::FillHitStart(step, hit);
  G4CMPQPSensitivity::FillHitFinal(step, hit);
}

G4bool G4CMPQPSensitivity::ProcessHits(G4Step* aStep,
                                              G4TouchableHistory* ROhist) {
  if (!IsHit(aStep, ROhist)) return false;
  G4CMPQPHit* hit = nullptr;
  G4Track* track = aStep->GetTrack();
  for (size_t i = 0; i < hitsCollection->entries(); ++i) {
    G4CMPQPHit* hi = (G4CMPQPHit*)(hitsCollection->GetHit(i));
    if (track->GetTrackID() != hi->GetTrackID()) continue;
    if (track->GetCurrentStepNumber() > hi->GetStepID() + 2) continue;
    hit = hi;
  }
  if (!hit) {
    hit = new G4CMPQPHit();
    G4CMPQPSensitivity::FillHit(aStep, hit);
    hitsCollection->insert(hit);
  }
  else {
    G4CMPQPSensitivity::FillHitFinal(aStep, hit);
  }
  return true;
}

G4bool G4CMPQPSensitivity::IsHit(const G4Step* step,
                                        const G4TouchableHistory*) const {
  // Only QPs allowed
  G4bool ans = G4CMP::IsQP(step->GetTrack());
  return ans;
}

void G4CMPQPSensitivity::EndOfEvent(G4HCofThisEvent* HCE) {
  const G4int nhc = HCE->GetNumberOfCollections();
  G4CMPQPHitsCollection* hc = nullptr;
  for (size_t i = 0; i < nhc; ++i)
  {
    auto *hci = HCE->GetHC(i);
    const G4String hnam = hci->GetName();
    if (hnam.find("G4CMPQPHit") == G4String::npos) continue;
    hc = (G4CMPQPHitsCollection*)hci;
    break;
  }
  if (!hc) return;
  const G4int nhits = hc->entries();
  std::vector<G4double> t0s(nhits, 0.), t1s(nhits, 0.);
  std::vector<G4double> ts(numSamples, 0.);
  G4double tmax = 0.;
  for (size_t i = 0; i < hc->entries(); ++i)
  {
    G4CMPQPHit* hit = (G4CMPQPHit*)(hc->GetHit(i));
    const G4double tf = hit->GetFinalTime();
    t0s[i] = hit->GetStartTime();
    t1s[i] = tf;

    // use the maximum QP time for sampling time window
    if (tf > tmax) tmax = tf;
  }
  for (size_t i = 0; i < numSamples; ++i)
  {
    const G4double ti = G4UniformRand() * tmax;
    G4double Nqp = 0.;
    for (size_t j = 0; j < nhits; ++j)
    {
      G4CMPQPHit* hit = (G4CMPQPHit*)(hc->GetHit(j));
      if (ti < hit->GetStartTime()) continue;
      if (ti >= hit->GetFinalTime()) continue;
      Nqp += hit->GetWeight();
    }
    if (Nqp <= 0.) continue;
    G4CMPQPHit* qp_wave = new G4CMPQPHit();
    qp_wave->SetTime(ti);
    qp_wave->SetNqp(Nqp);
    waveform->insert(qp_wave);
  }
}
