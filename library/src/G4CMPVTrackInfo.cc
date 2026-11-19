/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file library/src/G4CMPVTrackInfo.cc
/// \brief Implementation of the G4CMPVTrackInfo class. Used as a base class
/// for CMP particles to store auxiliary information that a G4Track can't
/// store, but is necessary for physics processes to know.
///
//
// $Id$
//
// 20161111 Initial commit - R. Agnese
// 20260721 G4CMP-624 -- Move wavevector here from PhononTrackInfo.

#include "G4CMPVTrackInfo.hh"
#include "G4SystemOfUnits.hh"


G4CMPVTrackInfo::G4CMPVTrackInfo(const G4LatticePhysical* lat) :
  G4VAuxiliaryTrackInformation(), lattice(lat), waveVec(0,0,0) {}

G4CMPVTrackInfo::G4CMPVTrackInfo(const G4LatticePhysical* lat,
				 const G4ThreeVector& theK) :
  G4VAuxiliaryTrackInformation(), lattice(lat), waveVec(theK) {}

void G4CMPVTrackInfo::Print() const {
  G4cout << "G4CMPVTrackInfo: lattice @ " << lattice << G4endl
	 << " K " << waveVec*m << " /m" << G4endl;
}
