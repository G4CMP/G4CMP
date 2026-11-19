/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file tests/testTrappingMFP.cc
/// \brief Unit test for lattice-dependent charge trapping and ionization MFPs.
//
// 20261118  G4CMP-687 -- Unit test for lattice-dependent trapping MFPs.

#include "G4CMPConfigManager.hh"
#include "G4CMPDriftElectron.hh"
#include "G4CMPDriftHole.hh"
#include "G4CMPDriftTrapIonization.hh"
#include "G4CMPDriftTrappingProcess.hh"
#include "G4CMPTrackUtils.hh"

#include "G4Box.hh"
#include "G4DynamicParticle.hh"
#include "G4ForceCondition.hh"
#include "G4LatticeLogical.hh"
#include "G4LatticeManager.hh"
#include "G4LatticePhysical.hh"
#include "G4LatticeReader.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NavigationHistory.hh"
#include "G4PVPlacement.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4TouchableHandle.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4Types.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ios.hh"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

namespace {

// Relative tolerance used when comparing floating-point mean free paths.
constexpr G4double relativeTolerance = 1.0e-9;

// Temporary config file written to exercise G4LatticeReader parsing.
constexpr char tempConfigPath[] = "/tmp/g4cmp_test_trapping_mfp.txt";

/// All six trapping and trap-ionization MFPs for one configuration tier
/// (global fallback, logical lattice config, or per-volume physical override).
struct MfpValues {
  G4double electronTrapping;
  G4double holeTrapping;
  G4double electronDonorIon;
  G4double electronAcceptorIon;
  G4double holeDonorIon;
  G4double holeAcceptorIon;
};

// Distinct values for each tier ensure fallback and override checks cannot pass
// accidentally by reading the wrong level of the MFP hierarchy.
const MfpValues globalFallbackMfps = {
    /*electronTrapping=*/10.0 * mm,
    /*holeTrapping=*/20.0 * mm,
    /*electronDonorIon=*/30.0 * mm,
    /*electronAcceptorIon=*/40.0 * mm,
    /*holeDonorIon=*/50.0 * mm,
    /*holeAcceptorIon=*/60.0 * mm,
};

const MfpValues logicalConfigMfps = {
    /*electronTrapping=*/100.0 * um,
    /*holeTrapping=*/200.0 * um,
    /*electronDonorIon=*/300.0 * um,
    /*electronAcceptorIon=*/400.0 * um,
    /*holeDonorIon=*/500.0 * um,
    /*holeAcceptorIon=*/600.0 * um,
};

const MfpValues physicalOverrideMfps = {
    /*electronTrapping=*/1.0 * mm,
    /*holeTrapping=*/2.0 * mm,
    /*electronDonorIon=*/3.0 * mm,
    /*electronAcceptorIon=*/4.0 * mm,
    /*holeDonorIon=*/5.0 * mm,
    /*holeAcceptorIon=*/6.0 * mm,
};

/// Records assertion failures across test functions without using global state.
class TestRecorder {
public:
  /// Checks that \p actual matches \p expected within relativeTolerance.
  /// Exact equality is tested first so the DBL_MAX sentinel compares cleanly.
  void CheckNear(G4double actual, G4double expected, const char* label) {
    G4bool isNear =
        (actual == expected) || (std::fabs(actual - expected) <=
                                 relativeTolerance * std::fabs(expected));
    if (!isNear) {
      G4cerr << "FAIL " << label << ": got " << actual << ", expected "
             << expected << G4endl;
      ++failureCount;
    }
  }

  /// Checks that \p condition is true, recording a failure with \p label if
  /// not.
  void CheckTrue(G4bool condition, const char* label) {
    if (!condition) {
      G4cerr << "FAIL " << label << G4endl;
      ++failureCount;
    }
  }

  /// Returns the total number of failed assertions recorded so far.
  G4int GetFailureCount() const { return failureCount; }

private:
  G4int failureCount = 0;
};

/// Subclass that exposes the protected GetMeanFreePath() of
/// G4CMPDriftTrappingProcess so tests can query the active MFP directly without
/// running a full Geant4 stepping loop.
class TrappingProcessProbe final : public G4CMPDriftTrappingProcess {
public:
  G4double EvaluateMeanFreePath(const G4Track& track) {
    G4ForceCondition condition = NotForced;
    return GetMeanFreePath(track, /*previousStepSize=*/0.0, &condition);
  }
};

/// Subclass that exposes the protected GetMeanFreePath() of
/// G4CMPDriftTrapIonization for a given (impact carrier, trapped carrier) pair.
class TrapIonizationProbe final : public G4CMPDriftTrapIonization {
public:
  TrapIonizationProbe(G4ParticleDefinition* impactParticle,
                      G4ParticleDefinition* trapParticle)
      : G4CMPDriftTrapIonization(impactParticle, trapParticle) {}

  G4double EvaluateMeanFreePath(const G4Track& track) {
    G4ForceCondition condition = NotForced;
    return GetMeanFreePath(track, /*previousStepSize=*/0.0, &condition);
  }
};

/// Builds a minimal touchable pointing to \p volume so tracks can be placed in
/// a volume without initializing a full G4RunManager or geometry navigator.
G4TouchableHandle MakeTouchable(G4VPhysicalVolume* volume) {
  G4NavigationHistory history;
  history.SetFirstEntry(volume);
  return new G4TouchableHistory(history);
}

/// Writes \p mfps to a temporary config file, parses it with G4LatticeReader,
/// and deletes the temporary file.
/// Callers receive a heap-allocated G4LatticeLogical because its internal
/// fKVMap table is ~7 MB and risks overflowing the stack if allocated locally.
///
/// \returns The parsed G4LatticeLogical, or nullptr if parsing fails.
std::unique_ptr<G4LatticeLogical> LoadLogicalLatticeFromConfig(
    const MfpValues& mfps) {
  {
    std::ofstream configOut(tempConfigPath);
    configOut << "eTrappingMFP " << (mfps.electronTrapping / um) << " um\n"
              << "hTrappingMFP " << (mfps.holeTrapping / um) << " um\n"
              << "eDTrapIonizationMFP " << (mfps.electronDonorIon / um)
              << " um\n"
              << "eATrapIonizationMFP " << (mfps.electronAcceptorIon / um)
              << " um\n"
              << "hDTrapIonizationMFP " << (mfps.holeDonorIon / um) << " um\n"
              << "hATrapIonizationMFP " << (mfps.holeAcceptorIon / um)
              << " um\n";
  }

  G4LatticeReader reader;
  std::unique_ptr<G4LatticeLogical> lattice(reader.MakeLattice(tempConfigPath));
  std::remove(tempConfigPath);
  return lattice;
}

/// Applies all six MFP overrides in \p mfps to \p physicalLattice.
void SetPhysicalLatticeMfps(const MfpValues& mfps,
                            G4LatticePhysical& physicalLattice) {
  physicalLattice.SetETrappingMFP(mfps.electronTrapping);
  physicalLattice.SetHTrappingMFP(mfps.holeTrapping);
  physicalLattice.SetEDTrapIonMFP(mfps.electronDonorIon);
  physicalLattice.SetEATrapIonMFP(mfps.electronAcceptorIon);
  physicalLattice.SetHDTrapIonMFP(mfps.holeDonorIon);
  physicalLattice.SetHATrapIonMFP(mfps.holeAcceptorIon);
}

/// Sets all six global fallback MFPs in the G4CMPConfigManager singleton.
void SetGlobalConfigMfps(const MfpValues& mfps) {
  G4CMPConfigManager::SetETrappingMFP(mfps.electronTrapping);
  G4CMPConfigManager::SetHTrappingMFP(mfps.holeTrapping);
  G4CMPConfigManager::SetEDTrapIonMFP(mfps.electronDonorIon);
  G4CMPConfigManager::SetEATrapIonMFP(mfps.electronAcceptorIon);
  G4CMPConfigManager::SetHDTrapIonMFP(mfps.holeDonorIon);
  G4CMPConfigManager::SetHATrapIonMFP(mfps.holeAcceptorIon);
}

/// Creates a 1 eV charge-carrier track positioned in \p initialTouchable with
/// G4CMP auxiliary track info attached (required by G4CMP drift processes).
std::unique_ptr<G4Track> MakeTestTrack(
    G4ParticleDefinition* particleDef,
    const G4TouchableHandle& initialTouchable) {
  // G4Track takes ownership of the heap-allocated G4DynamicParticle.
  G4DynamicParticle* dynamicParticle = new G4DynamicParticle(
      particleDef, G4ThreeVector(1.0, 0.0, 0.0), 1.0 * eV);
  std::unique_ptr<G4Track> track(
      new G4Track(dynamicParticle, /*aTime=*/0.0, G4ThreeVector()));
  track->SetTouchableHandle(initialTouchable);
  G4CMP::AttachTrackInfo(track.get());
  return track;
}

/// Verifies that unconfigured lattices default to the DBL_MAX sentinel and fall
/// back to G4CMPConfigManager global settings when physical and logical values
/// are unset.
void TestDefaultAndGlobalFallback(TestRecorder& results) {
  // Heap-allocate G4LatticeLogical to avoid stack overflow (~7 MB fKVMap).
  std::unique_ptr<G4LatticeLogical> emptyLogical(new G4LatticeLogical());
  G4LatticePhysical emptyPhysical(emptyLogical.get());

  results.CheckNear(emptyLogical->GetETrappingMFP(), DBL_MAX,
                    "default logical eTrap");
  results.CheckNear(emptyPhysical.GetHATrapIonMFP(), DBL_MAX,
                    "default physical hATrap");

  SetGlobalConfigMfps(globalFallbackMfps);
  results.CheckNear(emptyPhysical.GetETrappingMFP(),
                    globalFallbackMfps.electronTrapping,
                    "global fallback eTrap");
  results.CheckNear(emptyPhysical.GetHTrappingMFP(),
                    globalFallbackMfps.holeTrapping, "global fallback hTrap");
  results.CheckNear(emptyPhysical.GetEDTrapIonMFP(),
                    globalFallbackMfps.electronDonorIon, "global fallback eD");
  results.CheckNear(emptyPhysical.GetEATrapIonMFP(),
                    globalFallbackMfps.electronAcceptorIon,
                    "global fallback eA");
  results.CheckNear(emptyPhysical.GetHDTrapIonMFP(),
                    globalFallbackMfps.holeDonorIon, "global fallback hD");
  results.CheckNear(emptyPhysical.GetHATrapIonMFP(),
                    globalFallbackMfps.holeAcceptorIon, "global fallback hA");

  // Restore global defaults so singleton state does not leak into other tests.
  const MfpValues unsetMfps = {DBL_MAX, DBL_MAX, DBL_MAX,
                               DBL_MAX, DBL_MAX, DBL_MAX};
  SetGlobalConfigMfps(unsetMfps);
}

/// Verifies that G4LatticeReader parses all six MFP directives from a crystal
/// config file, G4LatticeLogical's copy constructor preserves them, and Dump()
/// prints configured trapping MFPs.
void TestLatticeReaderCopyAndDump(TestRecorder& results) {
  std::unique_ptr<G4LatticeLogical> loadedLattice =
      LoadLogicalLatticeFromConfig(logicalConfigMfps);
  results.CheckTrue(loadedLattice != nullptr, "reader loaded config");
  if (!loadedLattice) {
    return;
  }

  std::unique_ptr<G4LatticeLogical> copiedLattice(
      new G4LatticeLogical(*loadedLattice));
  results.CheckNear(copiedLattice->GetETrappingMFP(),
                    logicalConfigMfps.electronTrapping, "reader/copy eTrap");
  results.CheckNear(copiedLattice->GetHTrappingMFP(),
                    logicalConfigMfps.holeTrapping, "reader/copy hTrap");
  results.CheckNear(copiedLattice->GetEDTrapIonMFP(),
                    logicalConfigMfps.electronDonorIon, "reader/copy eD");
  results.CheckNear(copiedLattice->GetEATrapIonMFP(),
                    logicalConfigMfps.electronAcceptorIon, "reader/copy eA");
  results.CheckNear(copiedLattice->GetHDTrapIonMFP(),
                    logicalConfigMfps.holeDonorIon, "reader/copy hD");
  results.CheckNear(copiedLattice->GetHATrapIonMFP(),
                    logicalConfigMfps.holeAcceptorIon, "reader/copy hA");

  std::ostringstream dumpStream;
  loadedLattice->Dump(dumpStream);
  results.CheckTrue(dumpStream.str().find("eTrappingMFP") != std::string::npos,
                    "configured lattice Dump includes eTrappingMFP");
}

/// Verifies that per-volume G4LatticePhysical overrides supersede the
/// underlying G4LatticeLogical values, and that resetting an override to
/// DBL_MAX restores fallback to the logical lattice.
void TestPhysicalLatticeOverrides(TestRecorder& results) {
  std::unique_ptr<G4LatticeLogical> logicalLattice =
      LoadLogicalLatticeFromConfig(logicalConfigMfps);
  results.CheckTrue(logicalLattice != nullptr,
                    "reader loaded config for physical override test");
  if (!logicalLattice) {
    return;
  }

  G4LatticePhysical physicalLattice(logicalLattice.get());
  SetPhysicalLatticeMfps(physicalOverrideMfps, physicalLattice);

  results.CheckNear(physicalLattice.GetETrappingMFP(),
                    physicalOverrideMfps.electronTrapping,
                    "phys override eTrap");
  results.CheckNear(physicalLattice.GetHTrappingMFP(),
                    physicalOverrideMfps.holeTrapping, "phys override hTrap");
  results.CheckNear(physicalLattice.GetEDTrapIonMFP(),
                    physicalOverrideMfps.electronDonorIon, "phys override eD");
  results.CheckNear(physicalLattice.GetEATrapIonMFP(),
                    physicalOverrideMfps.electronAcceptorIon,
                    "phys override eA");
  results.CheckNear(physicalLattice.GetHDTrapIonMFP(),
                    physicalOverrideMfps.holeDonorIon, "phys override hD");
  results.CheckNear(physicalLattice.GetHATrapIonMFP(),
                    physicalOverrideMfps.holeAcceptorIon, "phys override hA");

  physicalLattice.SetETrappingMFP(DBL_MAX);
  results.CheckNear(physicalLattice.GetETrappingMFP(),
                    logicalConfigMfps.electronTrapping,
                    "phys DBL_MAX reset falls back to logical lattice");
}

/// Verifies that trapping and trap-ionization processes read MFPs from the
/// track's current volume lattice and refresh their cached MFPs when a track
/// crosses into a volume with a different physical lattice.
void TestProcessLookupAndLatticeChangeover(TestRecorder& results) {
  std::unique_ptr<G4LatticeLogical> logicalLattice =
      LoadLogicalLatticeFromConfig(logicalConfigMfps);
  results.CheckTrue(logicalLattice != nullptr,
                    "reader loaded config for process lookup test");
  if (!logicalLattice) {
    return;
  }

  // Electron track initialization requires at least one conduction valley.
  logicalLattice->AddValley(G4ThreeVector(1.0, 0.0, 0.0));

  // Build two volumes sharing the same logical lattice. Volume 1 inherits the
  // logical MFPs; volume 2 applies per-volume physical overrides.
  // Geant4's geometry stores take ownership of material, solid, logical
  // volumes, and physical volumes.
  G4Material* siliconMaterial =
      new G4Material("TestSi", 14.0, 28.09 * g / mole, 2.33 * g / cm3);
  G4Box* solidBox = new G4Box("Box", 1.0 * cm, 1.0 * cm, 1.0 * cm);
  G4LogicalVolume* logicalVol1 =
      new G4LogicalVolume(solidBox, siliconMaterial, "LV1");
  G4LogicalVolume* logicalVol2 =
      new G4LogicalVolume(solidBox, siliconMaterial, "LV2");
  G4VPhysicalVolume* physicalVol1 =
      new G4PVPlacement(/*pRot=*/nullptr, G4ThreeVector(), logicalVol1, "PV1",
                        /*pMotherLogical=*/nullptr, /*pMany=*/false,
                        /*pCopyNo=*/0);
  G4VPhysicalVolume* physicalVol2 =
      new G4PVPlacement(/*pRot=*/nullptr, G4ThreeVector(), logicalVol2, "PV2",
                        /*pMotherLogical=*/nullptr, /*pMany=*/false,
                        /*pCopyNo=*/0);

  // G4LatticeManager takes ownership of registered G4LatticePhysical pointers.
  G4LatticePhysical* latticeVol1 = new G4LatticePhysical(logicalLattice.get());
  G4LatticePhysical* latticeVol2 = new G4LatticePhysical(logicalLattice.get());
  SetPhysicalLatticeMfps(physicalOverrideMfps, *latticeVol2);

  G4LatticeManager* latticeManager = G4LatticeManager::GetLatticeManager();
  latticeManager->RegisterLattice(physicalVol1, latticeVol1);
  latticeManager->RegisterLattice(physicalVol2, latticeVol2);

  G4ParticleDefinition* electronDef = G4CMPDriftElectron::Definition();
  G4ParticleDefinition* holeDef = G4CMPDriftHole::Definition();
  G4TouchableHandle touchableVol1 = MakeTouchable(physicalVol1);
  G4TouchableHandle touchableVol2 = MakeTouchable(physicalVol2);

  std::unique_ptr<G4Track> electronTrack =
      MakeTestTrack(electronDef, touchableVol1);
  std::unique_ptr<G4Track> holeTrack = MakeTestTrack(holeDef, touchableVol1);

  TrappingProcessProbe trappingProbe;
  TrapIonizationProbe electronDonorProbe(electronDef, electronDef);
  TrapIonizationProbe electronAcceptorProbe(electronDef, holeDef);
  TrapIonizationProbe holeDonorProbe(holeDef, electronDef);
  TrapIonizationProbe holeAcceptorProbe(holeDef, holeDef);

  // Verify initial electron MFPs in volume 1 (logical lattice values).
  trappingProbe.StartTracking(electronTrack.get());
  electronDonorProbe.StartTracking(electronTrack.get());
  electronAcceptorProbe.StartTracking(electronTrack.get());
  results.CheckNear(trappingProbe.EvaluateMeanFreePath(*electronTrack),
                    logicalConfigMfps.electronTrapping, "proc eTrap vol1");
  results.CheckNear(electronDonorProbe.EvaluateMeanFreePath(*electronTrack),
                    logicalConfigMfps.electronDonorIon, "proc eD vol1");
  results.CheckNear(electronAcceptorProbe.EvaluateMeanFreePath(*electronTrack),
                    logicalConfigMfps.electronAcceptorIon, "proc eA vol1");

  // Advance track length before switching touchables so GetMeanFreePath()
  // exercises the trackLength > 0 lattice-changeover branch rather than the
  // trackLength == 0 start-of-track initialization branch.
  electronTrack->AddTrackLength(1.0 * mm);
  electronTrack->SetTouchableHandle(touchableVol2);
  results.CheckNear(trappingProbe.EvaluateMeanFreePath(*electronTrack),
                    physicalOverrideMfps.electronTrapping,
                    "proc eTrap vol2 changeover");
  results.CheckNear(electronDonorProbe.EvaluateMeanFreePath(*electronTrack),
                    physicalOverrideMfps.electronDonorIon,
                    "proc eD vol2 changeover");
  results.CheckNear(electronAcceptorProbe.EvaluateMeanFreePath(*electronTrack),
                    physicalOverrideMfps.electronAcceptorIon,
                    "proc eA vol2 changeover");

  // Verify initial hole MFPs in volume 1 (logical lattice values).
  trappingProbe.StartTracking(holeTrack.get());
  holeDonorProbe.StartTracking(holeTrack.get());
  holeAcceptorProbe.StartTracking(holeTrack.get());
  results.CheckNear(trappingProbe.EvaluateMeanFreePath(*holeTrack),
                    logicalConfigMfps.holeTrapping, "proc hTrap vol1");
  results.CheckNear(holeDonorProbe.EvaluateMeanFreePath(*holeTrack),
                    logicalConfigMfps.holeDonorIon, "proc hD vol1");
  results.CheckNear(holeAcceptorProbe.EvaluateMeanFreePath(*holeTrack),
                    logicalConfigMfps.holeAcceptorIon, "proc hA vol1");

  // Advance track length and move the hole into volume 2 to verify changeover.
  holeTrack->AddTrackLength(1.0 * mm);
  holeTrack->SetTouchableHandle(touchableVol2);
  results.CheckNear(trappingProbe.EvaluateMeanFreePath(*holeTrack),
                    physicalOverrideMfps.holeTrapping,
                    "proc hTrap vol2 changeover");
  results.CheckNear(holeDonorProbe.EvaluateMeanFreePath(*holeTrack),
                    physicalOverrideMfps.holeDonorIon,
                    "proc hD vol2 changeover");
  results.CheckNear(holeAcceptorProbe.EvaluateMeanFreePath(*holeTrack),
                    physicalOverrideMfps.holeAcceptorIon,
                    "proc hA vol2 changeover");
}

}  // namespace

int main() {
  TestRecorder results;
  TestDefaultAndGlobalFallback(results);
  TestLatticeReaderCopyAndDump(results);
  TestPhysicalLatticeOverrides(results);
  TestProcessLookupAndLatticeChangeover(results);

  if (results.GetFailureCount() == 0) {
    G4cout << "testTrappingMFP PASSED" << G4endl;
    return 0;
  }
  return 1;
}
