/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file library/src/G4CMPParticleChangeForCharge.cc
/// \brief Implementation of the G4CMPParticleChangeForCharge class
//
// 20261030 G4CMP-610 -- Implement ParticleChange for charges

#include "G4CMPParticleChangeForCharge.hh"
#include "G4CMPDriftTrackInfo.hh"
#include "G4CMPTrackUtils.hh"
#include "G4LatticePhysical.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VTouchable.hh"

// Ensure that local flags are initialized at each step

void G4CMPParticleChangeForCharge::Initialize(const G4Track& track) {
  // Use base class's method first.
  G4ParticleChange::Initialize(track);
  // initialize derived class attributes
  G4CMPDriftTrackInfo* trackInfo = G4CMP::GetTrackInfo<G4CMPDriftTrackInfo>(track);
  theLattice = const_cast<G4LatticePhysical*>(trackInfo->Lattice());
  theWaveVector = trackInfo->k();
  theValley = trackInfo->ValleyIndex();
}

// Populate G4Step with results of process' PostStepDoIt()
  
G4Step* G4CMPParticleChangeForCharge::UpdateStepForPostStep(G4Step* pStep) {
  // Call base class function
  G4Step* newStep = G4ParticleChange::UpdateStepForPostStep(pStep);
  // propagate values to DriftTrackInfo
  G4Track* track = newStep->GetTrack();
  G4CMPDriftTrackInfo* trackInfo = G4CMP::GetTrackInfo<G4CMPDriftTrackInfo>(*track);
  if (theLattice) trackInfo->SetLattice(theLattice);
  trackInfo->SetWaveVector(theWaveVector);
  trackInfo->SetValleyIndex(theValley);
  return newStep;
}


// Include local information in printout

void G4CMPParticleChangeForCharge::DumpInfo() const {
  G4ParticleChange::DumpInfo();
  G4cout << "        theLattice : " << theLattice->GetLattice()->GetName() << G4endl;
  G4cout << "        theWaveVector : " << theWaveVector << G4endl;
  G4cout << "        theValley : " << theValley << G4endl;
}

void G4CMPParticleChangeForCharge::ProposeValley(const G4int& Vfinal) {
  theValley = Vfinal;
}

const G4int* G4CMPParticleChangeForCharge::GetValley() const {
  return &theValley;
}

const G4int* G4CMPParticleChangeForCharge::GetProposedValley() const {
  return &theValley;
}

void G4CMPParticleChangeForCharge::SetProposedValley(const G4int& Vfinal) {
  theValley = Vfinal;
}

void G4CMPParticleChangeForCharge::ProposeLattice(G4LatticePhysical* lattice) {
  theLattice = lattice;
}

const G4LatticePhysical* G4CMPParticleChangeForCharge::GetLattice() const {
  return theLattice;
}

const G4LatticePhysical* G4CMPParticleChangeForCharge::GetProposedLattice() const {
  return theLattice;
}

void G4CMPParticleChangeForCharge::SetProposedLattice(
                                                      G4LatticePhysical* lattice) {
  theLattice = lattice;
}

void G4CMPParticleChangeForCharge::ProposeWaveVector(const G4ThreeVector& Kfinal) {
  theWaveVector = Kfinal;
}

void G4CMPParticleChangeForCharge::ProposeWaveVector(G4double Kx, G4double Ky,
                                                     G4double Kz) {
  theWaveVector = G4ThreeVector(Kx, Ky, Kz);
}

const G4ThreeVector* G4CMPParticleChangeForCharge::GetWaveVector() const {
  return &theWaveVector;
}

const G4ThreeVector* G4CMPParticleChangeForCharge::GetProposedWaveVector() const {
  return &theWaveVector;
}

void G4CMPParticleChangeForCharge::SetProposedWaveVector(
                                                         const G4ThreeVector& Kfinal) {
  theWaveVector = Kfinal;
}

void G4CMPParticleChangeForCharge::CorrectValleyForLattice() {
  G4int iv = theValley;
  if (theLattice && iv >= theLattice->NumberOfValleys()) {
    iv = theLattice->NumberOfValleys() - 1;
  }
  ProposeValley(iv);
}
