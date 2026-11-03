/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/
// 20261103 G4CMP-692 -- Initial implementation

#ifndef G4CMPQPSensitivity_hh
#define G4CMPQPSensitivity_hh 1

#include "G4VSensitiveDetector.hh"
#include "G4CMPQPHit.hh"

class G4HCofThisEvent;

class G4CMPQPSensitivity : public G4VSensitiveDetector {
public:
  explicit G4CMPQPSensitivity(G4String);
  // No copies
  G4CMPQPSensitivity(const G4CMPQPSensitivity&) = delete;
  G4CMPQPSensitivity& operator=(const G4CMPQPSensitivity&) = delete;
  // Move OK
  G4CMPQPSensitivity(G4CMPQPSensitivity&&);
  G4CMPQPSensitivity& operator=(G4CMPQPSensitivity&&);

  virtual void Initialize(G4HCofThisEvent*) override;
  virtual void EndOfEvent(G4HCofThisEvent*) override;

  // Get/Set for number of samples
  void SetNumSamples(G4int val) { numSamples = val; }
  G4int GetNumSamples() const { return numSamples; }
  
protected:
  virtual G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
  virtual G4bool IsHit(const G4Step*, const G4TouchableHistory*) const;

  static void FillHitStart(const G4Step*, G4CMPQPHit* hit);
  static void FillHitFinal(const G4Step*, G4CMPQPHit* hit);
  static void FillHit(const G4Step*, G4CMPQPHit* hit);

  G4CMPQPHitsCollection* hitsCollection;
  G4CMPQPWaveform* waveform;
  G4int numSamples;
}; // class G4CMPQPSensitivity
#endif /* G4CMPQPSensitivity_hh */
