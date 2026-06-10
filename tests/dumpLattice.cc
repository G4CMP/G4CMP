/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

// Usage: dumpLattice <material> <latticeName>
//
// Creates specified Geant4 material, then reads in requested G4CMP lattice
// and associates with the material.  After reading, the G4LatticeLogical
// is printed to standard output.
//
// 20260609  Michael Kelsey

#include "globals.hh"
#include "G4CMPConfigManager.hh"
#include "G4LatticeLogical.hh"
#include "G4LatticeReader.hh"
#include "G4LatticeManager.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"

int main(int argc, char* argv[]) {
  if (argc < 3) {
    G4cerr << "Usage: " << argv[0] << " <Material> <Lattice>" << G4endl;
    ::exit(1);
  }

  G4String matName = argv[1];
  G4String latName = argv[2];

  if (!G4CMPConfigManager::Instance()) {
    G4cerr << "ERROR: Unable to create G4CMPConfigManager" << G4endl;
    ::exit(1);
  }

  G4cout << "Loading lattice " << latName << " for " << matName << G4endl;

  G4Material* mat = G4NistManager::Instance()->FindOrBuildMaterial(matName);
  if (!mat) {
    G4cerr << "ERROR: Unable to find material " << matName << G4endl;
    ::exit(2);
  }

  G4LatticeManager* latMan = G4LatticeManager::Instance();
  G4LatticeLogical* lat = latMan->LoadLattice(mat, latName);
  if (!lat) {
    G4cerr << "ERROR: Unable to read lattice " << latName << G4endl;
    ::exit(2);
  }

  lat->Dump(G4cout);
}
