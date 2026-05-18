/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file library/src/G4CMPDriftRecombinationProcess.cc
/// \brief Charge (electron-hole) recombination
//
// 20170620  M. Kelsey -- Follow interface changes in G4CMPSecondaryUtils
// 20170802  M. Kelsey -- Replace phonon production with G4CMPEnergyPartition
// 20180827  M. Kelsey -- Prevent partitioner from recomputing sampling factors
// 20210328  Modify above; compute direct-phonon sampling factor here
// 20250929  M. Kelsey -- Include residual kinetic energy in phonon release
// 20260514  G4CMP-517 -- Register NTL (Luke) rate model to use in computing
//	       threshold and energy-gain estimation for killing in flight.

#include "G4CMPDriftRecombinationProcess.hh"
#include "G4CMPConfigManager.hh"
#include "G4CMPDriftElectron.hh"
#include "G4CMPDriftHole.hh"
#include "G4CMPEnergyPartition.hh"
#include "G4CMPFieldUtils.hh"
#include "G4CMPLukeEmissionRate.hh"
#include "G4CMPSecondaryUtils.hh"
#include "G4CMPSolidUtils.hh"
#include "G4CMPUtils.hh"
#include "G4LatticePhysical.hh"
#include "G4RandomDirection.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include <vector>


// Constructor and destructor

G4CMPDriftRecombinationProcess::
G4CMPDriftRecombinationProcess(const G4String &name, G4CMPProcessSubType type)
  : G4CMPVDriftProcess(name, type), partitioner(new G4CMPEnergyPartition) {
  partitioner->UseDownsampling(false);		// Apply preset scaling factors
  UseRateModel(new G4CMPLukeEmissionRate);	// For threshold estimation
}

G4CMPDriftRecombinationProcess::~G4CMPDriftRecombinationProcess() {
  delete partitioner;
}


// Process actions

G4double 
G4CMPDriftRecombinationProcess::GetMeanFreePath(const G4Track& aTrack, G4double,
						G4ForceCondition* cond) {
  UpdateMeanFreePathForLatticeChangeover(aTrack);
  *cond = Forced;
  return DBL_MAX;
}

G4VParticleChange* 
G4CMPDriftRecombinationProcess::PostStepDoIt(const G4Track& aTrack,
					     const G4Step& aStep) {
  InitializeParticleChange(aTrack);

  // If the particle has not come to rest, do nothing
  if (!ReadyToRecombine(aTrack)) {
    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  if (verboseLevel) {
    G4cout << "G4CMPDriftRecombinationProcess::PostStepDoIt: " << G4endl
           << aTrack.GetDefinition()->GetParticleName()
           << " reabsorbed by the lattice " << G4endl
	   << " @ " << aTrack.GetPosition()
           << G4endl;
  }

  *(G4CMPProcessUtils*)partitioner = *(G4CMPProcessUtils*)this;
  partitioner->SetVerboseLevel(verboseLevel);
  partitioner->UseVolume(aTrack.GetVolume());

  // Each charge carrier is independent, so it only gives back 0.5 times
  // the band gap. When the charge recombines, it may be in a different
  // location than its partner hole (due to a bias voltage), so combining
  // the two tracks is neither reasonable, nor The Geant4 Way.
  G4double Erecomb = (0.5*theLattice->GetBandGapEnergy()
		      + aTrack.GetKineticEnergy());

  partitioner->ComputePhononSampling(Erecomb);
  partitioner->DoPartition(0., Erecomb);
  partitioner->GetSecondaries(&aParticleChange);

  if (aParticleChange.GetNumberOfSecondaries() == 0) {	// Record energy release
    aParticleChange.ProposeNonIonizingEnergyDeposit(Erecomb);
  }

  aParticleChange.ProposeEnergy(0.);
  aParticleChange.ProposeTrackStatus(fStopAndKill);

  ClearNumberOfInteractionLengthLeft();		// All processes should do this!
  return &aParticleChange;
}


// Tracks should recombine if stopped, or if permanently below NTL threshold

G4bool G4CMPDriftRecombinationProcess::
ReadyToRecombine(const G4Track& aTrack) const {
  if (aTrack.GetStepLength() <= 0.) return false;	// Avoid reflections

  if (aTrack.GetTrackStatus() == fStopButAlive) return true;

  // Minimum energy for NTL emission is kinetic energy at Vsound
  G4double Esound = GetRateModel()->Threshold(1e-9*eV);
  G4double Etrack = GetKineticEnergy(aTrack);

  // In field, track can "regain" energy even if below threshold now
  G4double Egain = EnergyGainToSurface(aTrack);
  if (Etrack+Egain < Esound) return true;

  // If none of the conditions were satisfied, let the track keep going
  return false;
}


// Compute maximum energy gain along current trajectory toward surface
// NOTE: Will include "turn around" if electric field is in use

G4double G4CMPDriftRecombinationProcess::
EnergyGainToSurface(const G4Track& aTrack) const {
  G4ThreeVector Efield = G4CMP::GetFieldAtPosition(aTrack);
  Efield *= aTrack.GetDynamicParticle()->GetCharge();

  if (Efield.mag() <= 0.) return 0.;	// No field, no energy gain

  // Use track touchable to create wrapper for G4VSolid interface
  G4CMPSolidUtils sutil(aTrack.GetTouchable(),verboseLevel,"Recombination");

  // Get distance to nearest surface along current trajectory
  G4double tDist = sutil.GetDistanceToSolid(aTrack.GetPosition(),
					    GetGlobalMomentum(aTrack).unit());
  G4double Vtrack = tDist*Efield.mag();		// Energy gain along track

  // Get distance to volume surface along direction field accelerates
  RotateToLocalDirection(Efield);
  G4double fDist = sutil.GetDistanceToSolid(aTrack.GetPosition(),
					    Efield.unit());
  G4double Vfield = fDist*Efield.mag();		// Energy gain induced by field

  return std::max(Vtrack,Vfield);
}

