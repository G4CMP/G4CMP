/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file library/include/G4CMPParticleChangeForCharge.hh
/// \brief Definition of the G4CMPParticleChangeForCharge class
///   Concrete class for ParticleChange for handling charge interactions
//
// 20261030 G4CMP-610 -- Implement ParticleChange for charges

#ifndef G4CMPParticleChangeForCharge_hh
#define G4CMPParticleChangeForCharge_hh 1

#include "G4TouchableHandle.hh"
#include "G4ParticleChange.hh"

class G4LatticePhysical;

class G4CMPParticleChangeForCharge : public G4ParticleChange {
public:
  G4CMPParticleChangeForCharge() : G4ParticleChange() {;}
  virtual ~G4CMPParticleChangeForCharge() override = default;

  // ParticleChange cannot be copied
  G4CMPParticleChangeForCharge(const G4CMPParticleChangeForCharge& right) = delete;
  G4CMPParticleChangeForCharge& operator=(const G4CMPParticleChangeForCharge& right) = delete;

  // Ensure that local flags are cleared between steps
  virtual void Initialize(const G4Track& track) override;
  
  // --- Methods for updating G4Step ---
  virtual G4Step* UpdateStepForPostStep(G4Step* pStep) override;

  virtual void DumpInfo() const override;

  // Get/Set theLattice: it is the outgoing lattice
  void ProposeLattice(G4LatticePhysical* lattice);
  const G4LatticePhysical* GetLattice() const;
  const G4LatticePhysical* GetProposedLattice() const;
  void SetProposedLattice(G4LatticePhysical* lattice);

  // Get/Set theWaveVector vector: it is the outgoing wavevector
  void ProposeWaveVector(const G4ThreeVector& Kfinal);
  void ProposeWaveVector(G4double Kx, G4double Ky, G4double Kz);
  const G4ThreeVector* GetWaveVector() const;
  const G4ThreeVector* GetProposedWaveVector() const;
  void SetProposedWaveVector(const G4ThreeVector& Kfinal);

  // Get/Set theValley: it is the outgoing charge valley
  void ProposeValley(const G4int& Vfinal);
  const G4int* GetValley() const;
  const G4int* GetProposedValley() const;
  void SetProposedValley(const G4int& Vfinal);

private:
  G4LatticePhysical* theLattice;
  G4ThreeVector theWaveVector;
  G4int theValley;
}; // class G4CMPParticleChangeForCharge
#endif /* G4CMPParticleChangeForCharge_hh */
