/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// /file  HeterostructureConfigManager.cc
/// /brief Implementation of the HeterostructureConfigManager class
///   Singleton container class for user configuration of G4CMP
///		Heterostructure example. Looks for environment variables	at
///		initialization to set default values; active values may be
///		changed via macro commands (see HeterostructureConfigMessenger).

// 20260815  Selby Q. Dang (Stanford/SLAC)

#include "HeterostructureConfigManager.hh"
#include "HeterostructureConfigMessenger.hh"
#include "G4RunManager.hh"
#include "G4Threading.hh"
#include <stdlib.h>

// Singleton Initializer
HeterostructureConfigManager* HeterostructureConfigManager::Instance() {
  static const HeterostructureConfigManager* masterInstance = 0;
  static G4ThreadLocal HeterostructureConfigManager* theInstance = 0;

  if (!theInstance) {
    if (!G4Threading::IsWorkerThread()) { // Master or sequential
      theInstance = new HeterostructureConfigManager;
      masterInstance = theInstance;
    } else {          // Workers copy from master
      theInstance = new HeterostructureConfigManager(*masterInstance);
    }
  }

  return theInstance;
}

HeterostructureConfigManager::HeterostructureConfigManager()
  : Hit_file(getenv("G4CMP_HIT_FILE")?getenv("G4CMP_HIT_FILE"):"Heterostructure_hits.txt"),
    fQAbsProbGeSi(0), fQAbsProbSiGe(0), fQReflProbGeSi(0), fQReflProbSiGe(0),
    fTopMaterial("Si"), fBottomMaterial("Ge"),
    fTopThickness(5.e-03), fBottomThickness(5.e-03),
    messenger(new HeterostructureConfigMessenger(this)) {;}

HeterostructureConfigManager::HeterostructureConfigManager(const HeterostructureConfigManager& master)
  : Hit_file(master.GetHitOutput()),
    fQAbsProbGeSi(master.GetQAbsProbGeSi()), fQAbsProbSiGe(master.GetQAbsProbSiGe()),
    fQReflProbGeSi(master.GetQReflProbGeSi()), fQReflProbSiGe(master.GetQReflProbSiGe()),
    fTopMaterial(master.GetTopMaterial()), fBottomMaterial(master.GetBottomMaterial()),
    fTopThickness(master.GetTopThickness()), fBottomThickness(master.GetBottomThickness()),
    messenger(new HeterostructureConfigMessenger(this)) {;}

HeterostructureConfigManager::~HeterostructureConfigManager() {
  delete messenger; messenger=0;
}


// Trigger rebuild of geometry if parameters change

void HeterostructureConfigManager::UpdateGeometry() {
  G4RunManager::GetRunManager()->ReinitializeGeometry(true);
}
