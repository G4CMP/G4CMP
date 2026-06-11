#include "B1AnalysisConfig.hh"

#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

G4ThreadLocal B1AnalysisConfig* B1AnalysisConfig::fInstance = nullptr;

B1AnalysisConfig* B1AnalysisConfig::Instance()
{
  if (!fInstance) {
    fInstance = new B1AnalysisConfig();
  }
  return fInstance;
}

B1AnalysisConfig::B1AnalysisConfig()
: fSpecies("electrons"),
  fMessenger(nullptr)
{
  fMessenger = new G4GenericMessenger(this, "/analysis/", "Analysis control");

  auto& speciesCmd =
    fMessenger->DeclareMethod("setSpecies", &B1AnalysisConfig::SetSpecies,
                              "Set species to analyze: electrons or phonons");
  speciesCmd.SetParameterName("species", false);
}

B1AnalysisConfig::~B1AnalysisConfig()
{
  delete fMessenger;
}

void B1AnalysisConfig::SetSpecies(const G4String& species)
{
  if (species == "electrons" || species == "phonons") {
    fSpecies = species;
    G4cout << "Analysis species set to: " << fSpecies << G4endl;
  } else {
    G4cout << "Unknown species mode: " << species
           << ". Valid options are: electrons, phonons" << G4endl;
  }
}

const G4String& B1AnalysisConfig::GetSpecies() const
{
  return fSpecies;
}

G4bool B1AnalysisConfig::IsElectronMode() const
{
  return fSpecies == "electrons";
}

G4bool B1AnalysisConfig::IsPhononMode() const
{
  return fSpecies == "phonons";
}

G4String B1AnalysisConfig::GetOutputFileName() const
{
  if (fSpecies == "electrons") return "g4cmp_electrons.root";
  if (fSpecies == "phonons")   return "g4cmp_phonons.root";
  return "g4cmp_output.root";
}
