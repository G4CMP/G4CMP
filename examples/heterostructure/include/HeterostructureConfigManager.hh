/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file  HeterostructureConfigManager.hh
/// \brief Definition of the HeterostructureConfigManager class
///   Singleton container class for user configuration of G4CMP
///   Heterostructure example. Looks for environment variables  at
///   initialization to set default values; active values may be
///   changed via macro commands (see HeterostructureConfigMessenger).

// 20260815  Selby Q. Dang (Stanford/SLAC)

#ifndef HeterostructureConfigManager_hh
#define HeterostructureConfigManager_hh 1

#include "globals.hh"

class HeterostructureConfigMessenger;


class HeterostructureConfigManager {
public:
  ~HeterostructureConfigManager();	// Must be public for end-of-job cleanup
  static HeterostructureConfigManager* Instance();   // Only needed by static accessors

  // Access current values
  static const G4String& GetHitOutput() { return Instance()->Hit_file; }
  static double GetQAbsProbGeSi() { return Instance()->fQAbsProbGeSi; }
  static double GetQReflProbGeSi() { return Instance()->fQReflProbGeSi; }
  static double GetQAbsProbSiGe() { return Instance()->fQAbsProbSiGe; }
  static double GetQReflProbSiGe() { return Instance()->fQReflProbSiGe; }
  static G4String GetTopMaterial() { return Instance()->fTopMaterial; }
  static G4String GetBottomMaterial() { return Instance()->fBottomMaterial; }
  static G4double GetTopThickness() { return Instance()->fTopThickness; }
  static G4double GetBottomThickness() { return Instance()->fBottomThickness; }

  // Change values (e.g., via Messenger)
  static void SetHitOutput(const G4String& name) { Instance()->Hit_file=name; UpdateGeometry(); }
  static void SetQAbsProbGeSi(const double qAbsProbGeSi) { Instance()->fQAbsProbGeSi=qAbsProbGeSi; UpdateGeometry(); }
  static void SetQReflProbGeSi(const double qReflProbGeSi) { Instance()->fQReflProbGeSi=qReflProbGeSi; UpdateGeometry(); }
  static void SetQAbsProbSiGe(const double qAbsProbSiGe) { Instance()->fQAbsProbSiGe=qAbsProbSiGe; UpdateGeometry(); }
  static void SetQReflProbSiGe(const double qReflProbSiGe) { Instance()->fQReflProbSiGe=qReflProbSiGe; UpdateGeometry(); }
  static void SetTopMaterial(const G4String& val) { Instance()->fTopMaterial=val; UpdateGeometry(); }
  static void SetBottomMaterial(const G4String& val) { Instance()->fBottomMaterial=val; UpdateGeometry(); }
  static void SetTopThickness(const G4double& val) { Instance()->fTopThickness=val; UpdateGeometry(); }
  static void SetBottomThickness(const G4double& val) { Instance()->fBottomThickness=val; UpdateGeometry(); }

  static void UpdateGeometry();

private:
  HeterostructureConfigManager();		// Singleton: only constructed on request
  HeterostructureConfigManager(const HeterostructureConfigManager&); // To clone from master
  HeterostructureConfigManager(HeterostructureConfigManager&&) = delete;
  HeterostructureConfigManager& operator=(const HeterostructureConfigManager&) = delete;
  HeterostructureConfigManager& operator=(HeterostructureConfigManager&&) = delete;

private:
  G4String Hit_file;	// Output file of e/h hits ($G4CMP_HIT_FILE)
  G4double fQAbsProbGeSi;
  G4double fQReflProbGeSi;
  G4double fQAbsProbSiGe;
  G4double fQReflProbSiGe;
  G4String fTopMaterial;
  G4String fBottomMaterial;
  G4double fTopThickness;
  G4double fBottomThickness;

  HeterostructureConfigMessenger* messenger;
};

#endif	/* HeterostructureConfigManager_hh */
