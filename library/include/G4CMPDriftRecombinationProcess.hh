/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file library/include/G4CMPDriftRecombinationProcess.hh
/// \brief Class definition for charge (electron-hole) recombination
//
// 20170802  Add EnergyPartition to handle phonon production
// 20260514  Add line breaks at 80 columns, for readability

#ifndef G4CMPDriftRecombinationProcess_h
#define G4CMPDriftRecombinationProcess_h 1

#include "G4CMPVDriftProcess.hh"
#include "globals.hh"

class G4CMPEnergyPartition;


class G4CMPDriftRecombinationProcess : public G4CMPVDriftProcess {
public:
  G4CMPDriftRecombinationProcess(const G4String& name = "G4CMPChargeRecombine",
                                 G4CMPProcessSubType type = fChargeRecombine);
  virtual ~G4CMPDriftRecombinationProcess();

  // No copying/moving
  G4CMPDriftRecombinationProcess(G4CMPDriftRecombinationProcess&) = delete;
  G4CMPDriftRecombinationProcess(G4CMPDriftRecombinationProcess&&) = delete;

  G4CMPDriftRecombinationProcess&
  operator=(const G4CMPDriftRecombinationProcess&) = delete;

  G4CMPDriftRecombinationProcess&
  operator=(const G4CMPDriftRecombinationProcess&&) = delete;

  virtual G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&)
    override;

protected:
  virtual G4double GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*)
    override;

  // Flag if track is eligible for recombination: stopped or below threshold
  G4bool ReadyToRecombine(const G4Track&) const;

  // Compute maximum energy gain along current trajectory toward surface
  // NOTE: Will include "turn around" if electric field is in use
  G4double EnergyGainToSurface(const G4Track&) const;

private:
  G4CMPEnergyPartition* partitioner;
};

#endif	/* G4CMPDriftRecombinationProcess_h */
