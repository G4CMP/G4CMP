/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file library/src/G4CMPQPRecombinationRate.cc
/// \brief Output a rate for an artificial time step for QPs undergoing
//  diffusion
//
#include "G4CMPQPDiffusionTimeStepperRate.hh"
#include "G4LatticePhysical.hh"
#include "G4PhysicalConstants.hh"
#include "G4Track.hh"
#include <vector>
#include <map>

G4double G4CMPQPDiffusionTimeStepperRate::Rate(const G4Track& /*aTrack*/) const {
  //Compute tau for time stepper, and invert for rate
  G4double tau_nextScatter = theLattice->GetSCQPDiffusionStepTau();
  return (1.0/tau_nextScatter);
}

void G4CMPQPDiffusionTimeStepperRate::
UpdateLookupTable(const G4LatticePhysical * /*theLat*/) {
  return;
}
