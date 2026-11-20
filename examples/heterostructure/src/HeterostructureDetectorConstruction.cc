/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file HeterostructureDetectorConstruction.cc 
/// \brief Implementation of the HeterostructureDetectorConstruction class
/// Constructs heterostructure toy geometry with germanium on silicon.

// 20260815  Selby Q. Dang (Stanford/SLAC)

#include "HeterostructureDetectorConstruction.hh"
#include "HeterostructureConfigManager.hh"
#include "HeterostructureSensitivity.hh"
#include "G4CMPLogicalBorderSurface.hh"
#include "G4CMPLogicalSkinSurface.hh"
#include "G4CMPPhononElectrode.hh"
#include "G4CMPSurfaceProperty.hh"
#include "G4CMPUtils.hh"
#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4GeometryManager.hh"
#include "G4LatticeLogical.hh"
#include "G4LatticeManager.hh"
#include "G4LatticePhysical.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RunManager.hh"
#include "G4SolidStore.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

HeterostructureDetectorConstruction::HeterostructureDetectorConstruction()
  : fLiquidHelium(0), fBottomMaterial(0), fTopMaterial(0), fWorldPhys(0),
    fGeSiInterface(0), fSiGeInterface(0), fGeVacInterface(0), fSiVacInterface(0), 
    fConstructed(false) {;}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

HeterostructureDetectorConstruction::~HeterostructureDetectorConstruction() {
  delete fGeSiInterface;
  delete fSiGeInterface;
  delete fGeVacInterface;
  delete fSiVacInterface;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4VPhysicalVolume* HeterostructureDetectorConstruction::Construct() {
  if (fConstructed) {
    if (!G4RunManager::IfGeometryHasBeenDestroyed()) {
      // Run manager hasn't cleaned volume stores. This code shouldn't execute
      G4GeometryManager::GetInstance()->OpenGeometry();
      G4PhysicalVolumeStore::GetInstance()->Clean();
      G4LogicalVolumeStore::GetInstance()->Clean();
      G4SolidStore::GetInstance()->Clean();
    }
    // Have to completely remove all lattices to avoid warning on reconstruction
    G4LatticeManager::GetLatticeManager()->Reset();
    // Clear all LogicalSurfaces
    // NOTE: No need to redefine the G4CMPSurfaceProperties
    G4CMPLogicalBorderSurface::CleanSurfaceTable();
  }

  DefineMaterials();
  SetupGeometry();
  fConstructed = true;

  return fWorldPhys;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void HeterostructureDetectorConstruction::DefineMaterials()
{ 
  G4NistManager* nistManager = G4NistManager::Instance();

  fLiquidHelium = nistManager->FindOrBuildMaterial("G4_AIR"); // to be corrected
  G4String mstr = "G4_" + HeterostructureConfigManager::GetBottomMaterial();
  fBottomMaterial = nistManager->FindOrBuildMaterial(mstr);
  mstr = "G4_" + HeterostructureConfigManager::GetTopMaterial();
  fTopMaterial = nistManager->FindOrBuildMaterial(mstr);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void HeterostructureDetectorConstruction::SetupGeometry()
{
  const G4double bot_thick = HeterostructureConfigManager::GetBottomThickness();
  const G4double top_thick = HeterostructureConfigManager::GetTopThickness();
  const G4String bn = HeterostructureConfigManager::GetBottomMaterial();
  const G4String tn = HeterostructureConfigManager::GetTopMaterial();
  //     
  // World
  //
  G4VSolid* worldSolid = new G4Box("World",1.*cm,1.*cm,1.*mm);
  G4LogicalVolume* worldLogical =
    new G4LogicalVolume(worldSolid,fLiquidHelium,"World");
  fWorldPhys = new G4PVPlacement(0,G4ThreeVector(),worldLogical,"World",0,
                                 false,0);
  
  //
  // Germanium layer -- should change to SiGe in the future!
  // 
  G4VSolid* BottomSolid = new G4Box("BottomSolid", 1.*mm, 1.*mm,bot_thick);
  G4LogicalVolume* BottomLog =
    new G4LogicalVolume(BottomSolid,fBottomMaterial,"BottomLogical");
  G4VPhysicalVolume* BottomPhys =
    new G4PVPlacement(0,G4ThreeVector(),BottomLog,"BottomPhysical",
                      worldLogical,false,0);

  //
  //Germanium lattice information
  //
  // G4LatticeManager gives physics processes access to lattices by volume
  G4LatticeManager* LM = G4LatticeManager::GetLatticeManager();
  G4LatticeLogical* BottomLogical = LM->LoadLattice(fBottomMaterial, bn);

  // G4LatticePhysical assigns G4LatticeLogical a physical orientation
  G4LatticePhysical* BottomPhysical = new G4LatticePhysical(BottomLogical);
  BottomPhysical->SetMillerOrientation(1,0,0);
  LM->RegisterLattice(BottomPhys, BottomPhysical);

  //
  // Silicon layer
  //
  G4VSolid* TopSolid = new G4Box("TopSolid",1.*mm,1.*mm,top_thick);
  G4LogicalVolume* TopLog =
    new G4LogicalVolume(TopSolid,fTopMaterial,"TopLogical");
  G4VPhysicalVolume* TopPhys = new G4PVPlacement(0,
    G4ThreeVector(0.,0.,bot_thick + top_thick), TopLog, "TopPhysical",
    worldLogical,false,0);

  //
  //Silicon lattice information
  //
  G4LatticeLogical* TopLogical = LM->LoadLattice(fTopMaterial, tn);
  // G4LatticePhysical assigns G4LatticeLogical a physical orientation
  G4LatticePhysical* TopPhysical = new G4LatticePhysical(TopLogical);
  TopPhysical->SetMillerOrientation(1,0,0);
  LM->RegisterLattice(TopPhys, TopPhysical);

  double qAbsProbGeSi  = HeterostructureConfigManager::GetQAbsProbGeSi();
  double qReflProbGeSi = HeterostructureConfigManager::GetQReflProbGeSi();
  double qAbsProbSiGe  = HeterostructureConfigManager::GetQAbsProbSiGe();
  double qReflProbSiGe = HeterostructureConfigManager::GetQReflProbSiGe();


  if (!fConstructed) {

    fGeSiInterface = new G4CMPSurfaceProperty("fGeSiInterface", 
                                          qAbsProbGeSi, qReflProbGeSi, 0.0, 0.0,
                                          0.0, 0.0, 0.0, 0.0, 
                                          0.0, 0.0);

    fSiGeInterface = new G4CMPSurfaceProperty("fSiGeInterface", 
                                          qAbsProbSiGe, qReflProbSiGe, 0.0, 0.0,
					  	                            0.0, 0.0, 0.0, 0.0, 
                                          0.0, 0.0);

    fGeVacInterface = new G4CMPSurfaceProperty("GeVacInterface", 
                                            0.0, 1.0, 0.0, 0.0,
					    	                            0.0, 1.0, 0.0, 0.0,
                                            0.0, 1.0);

    fSiVacInterface = new G4CMPSurfaceProperty("SiVacInterface", 
                                            0.0, 1.0, 0.0, 0.0,
                                            0.0, 1.0, 0.0, 0.0,
                                            0.0, 1.0);
  }

  //
  // Border surfaces
  //
  new G4CMPLogicalBorderSurface("GeSi", BottomPhys, TopPhys,
        fGeSiInterface);
  new G4CMPLogicalBorderSurface("SiGe", TopPhys, BottomPhys,
        fSiGeInterface);

  // new G4CMPLogicalSkinSurface("SiLogicalSkinSurface", fTopMaterialLogical,
  //          fSiVacInterface);
  // new G4CMPLogicalSkinSurface("GeLogicalSkinSurface", fBottomMaterialLogical,
  //          fGeVacInterface);

  new G4CMPLogicalBorderSurface("GeVac", BottomPhys, fWorldPhys,
				fGeVacInterface);
  new G4CMPLogicalBorderSurface("VacGe", fWorldPhys, BottomPhys,
        fGeVacInterface);

  new G4CMPLogicalBorderSurface("SiVac", TopPhys, fWorldPhys,
        fSiVacInterface);
  new G4CMPLogicalBorderSurface("VacSi", fWorldPhys, TopPhys,
        fSiVacInterface);

  //                                        
  // Visualization attributes
  //
  worldLogical->SetVisAttributes(G4VisAttributes::GetInvisible());
  G4VisAttributes* simpleBoxVisAtt= new G4VisAttributes(G4Colour(0,1.0,1.0));
  simpleBoxVisAtt->SetVisibility(true);
  G4VisAttributes* siVisAtt= new G4VisAttributes(G4Colour(1.0,0,1.0));
  siVisAtt->SetVisibility(true);
  BottomLog->SetVisAttributes(simpleBoxVisAtt);
  TopLog->SetVisAttributes(siVisAtt);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void HeterostructureDetectorConstruction::ConstructSDandField()
{
  
  G4SDManager* sdman = G4SDManager::GetSDMpointer();

  G4VSensitiveDetector* geSD = sdman->FindSensitiveDetector("GermaniumElectrode", false);
  if (!geSD){ // Only create detector if one doesn't exist.
    geSD = new HeterostructureSensitivity("GermaniumElectrode");
    sdman->AddNewDetector(geSD);
  }
  SetSensitiveDetector("BottomLogical",geSD);

  G4VSensitiveDetector* siSD = sdman->FindSensitiveDetector("SiliconElectrode", false);
  if (!siSD) {
    siSD = new HeterostructureSensitivity("SiliconElectrode");
    sdman->AddNewDetector(siSD);
  }
  SetSensitiveDetector("TopLogical", siSD);
}
