/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// /file  HeterostructureConfigMessenger.cc
/// /brief Implementation of the HeterostructureConfigMessenger class
///   Macro command defitions to set user configuration in
///		HeterostructureConfigManager.

// 20260815  Selby Q. Dang (Stanford/SLAC)

#include "HeterostructureConfigMessenger.hh"
#include "HeterostructureConfigManager.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"


// Constructor and destructor

HeterostructureConfigMessenger::HeterostructureConfigMessenger(HeterostructureConfigManager* mgr)
  : G4UImessenger("/g4cmp/", "User configuration for G4CMP Heterostructure example"),
    theManager(mgr), hitsCmd(0) {
  hitsCmd = CreateCommand<G4UIcmdWithAString>("HitsFile",
			      "Set filename for output of Heterostructure hit locations");

  qAbsProbBotTopCmd = CreateCommand<G4UIcmdWithADouble> ("qAbsProbBotTop",
            "Set the qAbsProb parameter from bottom -> top, between 0 and 1");
  qAbsProbBotTopCmd->SetParameterName("qAbsProbBotTop", false);
  qAbsProbBotTopCmd->SetRange("qAbsProbBotTop >= 0. && qAbsProbBotTop <= 1.");

  qAbsProbTopBotCmd = CreateCommand<G4UIcmdWithADouble> ("qAbsProbTopBot",
            "Set the qAbsProb parameter from top -> bottom, between 0 and 1");
  qAbsProbTopBotCmd->SetParameterName("qAbsProbTopBot", false);
  qAbsProbTopBotCmd->SetRange("qAbsProbTopBot >= 0. && qAbsProbTopBot <= 1.");

  qReflProbBotTopCmd = CreateCommand<G4UIcmdWithADouble> ("qReflProbBotTop",
            "Set the pReflProb parameter from bottom -> top, between 0 and 1");
  qReflProbBotTopCmd->SetParameterName("value", false);
  qReflProbBotTopCmd->SetRange("value >= 0. && value <= 1.");

  qReflProbTopBotCmd = CreateCommand<G4UIcmdWithADouble> ("qReflProbTopBot",
            "Set the pReflProb parameter from top -> bottom, between 0 and 1");
  qReflProbTopBotCmd->SetParameterName("value", false);
  qReflProbTopBotCmd->SetRange("value >= 0. && value <= 1.");

  CreateDirectory("/geometry/top/", "top volume in geometry");
  CreateDirectory("/geometry/bottom/", "bottom volume in geometry");
  topMatCmd = CreateCommand<G4UIcmdWithAString>("/material/top", "Set top volume material");
  topMatCmd->SetDefaultValue("Si");
  topMatCmd->SetGuidance("Only materials whose NIST manager suffix (ie everything after \'G4_\') ");
  topMatCmd->SetGuidance("corresponds to the name of a subdirectory accessible from ");
  topMatCmd->SetGuidance("G4CMPConfigManager::GetLatticeDir() can be used.");
  botMatCmd = CreateCommand<G4UIcmdWithAString>("/material/bottom", "Set bottom volume material");
  botMatCmd->SetDefaultValue("Ge");
  topThickCmd = CreateCommand<G4UIcmdWithADoubleAndUnit>( "/geometry/top/thickness", "Set top volume thickness");
  topThickCmd->SetParameterName("value", true, false);
  topThickCmd->SetDefaultValue(5.e-03);
  topThickCmd->SetDefaultUnit("mm");
  topThickCmd->SetUnitCategory("Length");
  botThickCmd = CreateCommand<G4UIcmdWithADoubleAndUnit>( "/geometry/bottom/thickness", "Set bottom volume thickness");
  botThickCmd->SetParameterName("value", true, false);
  botThickCmd->SetDefaultValue(5.e-03);
  botThickCmd->SetDefaultUnit("mm");
  botThickCmd->SetUnitCategory("Length");
}


HeterostructureConfigMessenger::~HeterostructureConfigMessenger() {
  delete hitsCmd; hitsCmd=0;
  delete qAbsProbBotTopCmd; qAbsProbBotTopCmd=0;
  delete qAbsProbTopBotCmd; qAbsProbTopBotCmd=0;
  delete qReflProbBotTopCmd; qReflProbBotTopCmd=0;
  delete qReflProbTopBotCmd; qReflProbTopBotCmd=0;
  delete topMatCmd; topMatCmd=0;
  delete botMatCmd; botMatCmd=0;
  delete topThickCmd; topThickCmd=0;
  delete botThickCmd; botThickCmd=0;
}


// Parse user input and add to configuration

void HeterostructureConfigMessenger::SetNewValue(G4UIcommand* cmd, G4String value) {
  if (cmd == hitsCmd) theManager->SetHitOutput(value);
  if (cmd == qAbsProbBotTopCmd) theManager->SetQAbsProbBotTop(StoD(value));
  if (cmd == qAbsProbTopBotCmd) theManager->SetQAbsProbTopBot(StoD(value));
  if (cmd == qReflProbBotTopCmd) theManager->SetQReflProbBotTop(StoD(value));
  if (cmd == qReflProbTopBotCmd) theManager->SetQReflProbTopBot(StoD(value));
  if (cmd == topMatCmd) theManager->SetTopMaterial(value);
  if (cmd == botMatCmd) theManager->SetBottomMaterial(value);
  if (cmd == topThickCmd) theManager->SetTopThickness(StoD(value));
  if (cmd == botThickCmd) theManager->SetBottomThickness(StoD(value));
}
