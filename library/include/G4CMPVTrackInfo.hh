/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file library/include/G4CMPVTrackInfo.hh
/// \brief Definition of the G4CMPVTrackInfo class. Used as a base class
/// for CMP particles to store auxiliary information that a G4Track can't
/// store, but is necessary for physics processes to know.
///
//
// $Id$
//
// 20161111 Initial commit - R. Agnese
// 20260721 G4CMP-624 -- Move wavevector here from PhononTrackInfo.

#ifndef G4CMPVTrackInfo_hh
#define G4CMPVTrackInfo_hh 1

#include "G4VAuxiliaryTrackInformation.hh"
#include "G4ThreeVector.hh"

class G4LatticePhysical;


class G4CMPVTrackInfo: public G4VAuxiliaryTrackInformation {
public:
  G4CMPVTrackInfo() = delete;
  G4CMPVTrackInfo(const G4LatticePhysical* lat);
  G4CMPVTrackInfo(const G4LatticePhysical* lat, const G4ThreeVector& theK);

  size_t ReflectionCount() const { return reflCount; }
  void IncrementReflectionCount() { ++reflCount; }

  const G4LatticePhysical* Lattice() const     { return lattice; }
  void SetLattice(const G4LatticePhysical* lat) { lattice = lat; }

  // Phonon wavevectors need to be passed in the global coordinate system
  void SetK(const G4ThreeVector& theK)          { waveVec = theK; }
  void SetWaveVector(const G4ThreeVector& theK) { waveVec = theK; }

  // Phonon wavevectors will be returned in the global coordinate system
  G4ThreeVector k() const          { return waveVec; }
  G4ThreeVector WaveVector() const { return waveVec; }

  // Report contents of TrackInfo (subclasses may overload)
  virtual void Print() const override;

protected:
  size_t reflCount = 0; // Number of times track has been reflected
  const G4LatticePhysical* lattice; // The lattice the track is currently in
  G4ThreeVector waveVec;
};

#endif
