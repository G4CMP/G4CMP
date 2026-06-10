//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1DetectorConstruction.cc
/// \brief Implementation of the B1DetectorConstruction class

#include "B1DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Tubs.hh"

// Begin import G4CMP-related header files
#include "G4MaterialPropertiestable.hh"
#include "G4LatticeManager.hh"
#include "G4LatticePhysical.hh"
// End import G4CMP-related header files

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
// Begin construction of lattice-related variables
  latManager(G4LatticeManager::GetLatticeManager()),
  latticePhysical(nullptr),
  Silicon(nullptr),
// End construction of lattice-related variables
  fScoringVolume(0)
{ 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::~B1DetectorConstruction()
{ 
  // Begin clean the lattice 
  if (latticePhysical) {
    delete latticePhysical; // Free the allocated memory
    latticePhysical = nullptr; // Set the pointer to nullptr to avoid dangling pointer
  }
  // End clean the lattice
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B1DetectorConstruction::Construct()
{  

  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  // .................Start Material Definitions...................
  
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
  
  // Air definition taken from NIST
 // G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* Cu_mat = nist->FindOrBuildMaterial("G4_Cu");
    G4Material* Al_mat = nist->FindOrBuildMaterial("G4_Al");
    G4Material* Si_mat = nist->FindOrBuildMaterial("G4_Si");
    
    G4double density_Air = 0.00000001*g/cm3;
    G4Material* world_mat = nist->BuildMaterialWithNewDensity("Air_newDens","G4_AIR",density_Air);

    // and some fake Cu
    //G4double density_Air = 0.00000001*g/cm3;
    //G4Material* Cu_mat = nist->BuildMaterialWithNewDensity("Cu_newDens","G4_Cu",density_Air); // a way to get rid of Cu attenuation

    
    /*
    
   //Argon gas definition taken from NIST, then change density (https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html)
   G4double density_Ar = 0.0017857*g/cm3;
   
   G4Material* Ar_mat = nist->BuildMaterialWithNewDensity("Ar_newDens","G4_Ar",density_Ar);


  //borosilicate (Pyrex) definition, taken from material compendium Borosilicate glass
  //https://www.pnnl.gov/main/publications/external/technical_reports/PNNL-15870.pdf pg.52
   G4double a,z,density_Pyrex,fractionmass;
   G4String name,symbol;
   G4int ncomponents;
   
   a=10.811*g/mole;
   G4Element* elB=new G4Element(name="Boron",symbol="B",z=5.,a);
   
   a=15.999*g/mole;
   G4Element* elO=new G4Element(name="Oxygen",symbol="O",z=8.,a);
   
   a=22.990*g/mole;
   G4Element* elNa=new G4Element(name="Sodium",symbol="Na",z=11.,a);
   
   a=26.982*g/mole;
   G4Element* elAl=new G4Element(name="Aluminum",symbol="Al",z=13.,a);
   
   a=28.086*g/mole;
   G4Element* elSi=new G4Element(name="Silicon",symbol="Si",z=14.,a);
   
   a=39.098*g/mole;
   G4Element* elK=new G4Element(name="Potassium",symbol="K",z=19.,a);
   
   
  density_Pyrex = 2.23*g/cm3;
  G4Material* pyrex_mat = new G4Material(name="pyrex_mat",density_Pyrex,ncomponents=6);
  pyrex_mat->AddElement(elB,  fractionmass=0.040066);
  pyrex_mat->AddElement(elO, fractionmass=0.539559);
  pyrex_mat->AddElement(elNa,  fractionmass=0.028191);
  pyrex_mat->AddElement(elAl,  fractionmass=0.011644);
  pyrex_mat->AddElement(elSi, fractionmass=0.377220);
  pyrex_mat->AddElement(elK, fractionmass=0.003321);
   
  
*/
    
  //     
  // World Box
  //
  
  G4double world_sizeX  = 1*cm;
  G4double world_sizeY = 1*cm, world_sizeZ  = 1*cm;
  
  G4Box* solidWorld =    
    new G4Box("World",                       //its name
       world_sizeX, world_sizeY, world_sizeZ);     //its size
      
  G4LogicalVolume* logicWorld =                         
    new G4LogicalVolume(solidWorld,          //its solid
                        world_mat,          //its material
                        "World");            //its name
                                   
  G4VPhysicalVolume* physWorld = 
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(0,0,0),       //at (0,0,0)
                      logicWorld,            //its logical volume
                      "World",               //its name
                      0,                     //its mother  volume
                      false,                 //no boolean operation
                      0,                     //copy number
                      checkOverlaps);        //overlaps checking
   /*
//
// Cu Box
//
 
    
    G4Box* solidCu =
    new G4Box("Cu_box",                    //its name
        0.75*mm, 50*mm, 50*mm); //its size (0.1x20x20cm)
      
  G4LogicalVolume* logicCu =
    new G4LogicalVolume(solidCu,            //its solid
                        Cu_mat,             //its material
                        "Cu_box");         //its name
  
  new G4PVPlacement(0,                       //no rotation
                    G4ThreeVector(11*mm,0,0),         //at (0,0,0)
                    logicCu,                //its logical volume
                    "Cu_box",              //its name
                    logicWorld,              //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    checkOverlaps);          //overlaps checking

  
   //
// Al box
//
    
    
    G4Box* solidAl =
    new G4Box("Al_box",                    //its name
              2*mm, 50*mm, 50*mm); //its size (0.1x20x20cm)
    
    G4LogicalVolume* logicAl =
    new G4LogicalVolume(solidAl,            //its solid
                        Al_mat,             //its material
                        "Al_box");         //its name
 
    
    
    new G4PVPlacement(0,                       //no rotation
                      G4ThreeVector(29.75*mm,0,0),         //at (0,0,0)
                      logicAl,                //its logical volume
                      "Al_box",              //its name
                      logicWorld,              //its mother  volume
                      false,                   //no boolean operation
                      0,                       //copy number
                      checkOverlaps);          //overlaps checking
    
    new G4PVPlacement(0,                       //no rotation
                      G4ThreeVector(49.75*mm,0,0),         //at (0,0,0)
                      logicAl,                //its logical volume
                      "Al_box",              //its name
                      logicWorld,              //its mother  volume
                      false,                   //no boolean operation
                      0,                       //copy number
                      checkOverlaps);          //overlaps checking
    
    new G4PVPlacement(0,                       //no rotation
                      G4ThreeVector(109.35*mm,0,0),         //at (0,0,0)
                      logicAl,                //its logical volume
                      "Al_box",              //its name
                      logicWorld,              //its mother  volume
                      false,                   //no boolean operation
                      0,                       //copy number
                      checkOverlaps);          //overlaps checking
    
     */
    
    
//    Cu source disk
   
    G4Tubs* solidCu =
    new G4Tubs("Cu_disk",
               0, 3.75*mm, 0.25*mm, 0, 2*M_PI);
    
  //  G4Box* solidCu =
  //  new G4Box("Cu_box",                    //its name
  //            0.75*mm, 50*mm, 50*mm); //its size (0.1x20x20cm)
    
    G4LogicalVolume* logicCu =
    new G4LogicalVolume(solidCu,            //its solid
                        Cu_mat,             //its material
                        "Cu_disk");         //its name
    
    new G4PVPlacement(0,                       //no rotation
                      G4ThreeVector(0,0,0),         //at (0,0,0)
                      logicCu,                //its logical volume
                      "Cu_disk",              //its name
                      logicWorld,              //its mother  volume
                      false,                   //no boolean operation
                      0,                       //copy number
                      checkOverlaps);          //overlaps checking
    
    
    
     //
  //Si Box
  //
         
  G4Box* solidSi =
    new G4Box("Si_box",                     //its name
        2.5*mm, 2.5*mm, 0.19*mm);           //its size  .25 3mm 3mm
      
  G4LogicalVolume* logicSi =
    new G4LogicalVolume(solidSi,            //its solid
                        Si_mat,             //its material
                        "Si_box");          //its name
               
  G4VPhysicalVolume* physSi = 
    new G4PVPlacement(0,                       //no rotation
                      G4ThreeVector(0,0,-3.74*mm),         //at (0,0,0)  should be mm XXXXXXXXX
                      logicSi,                 //its logical volume
                      "Si_box",                //its name
                      logicWorld,              //its mother  volume
                      false,                   //no boolean operation
                      0,                       //copy number
                      checkOverlaps);          //overlaps checking

    // Begin instantiation of the lattice manager and load the lattice for silicon
    G4double density = 2.33 * g/cm3;
    G4int ncomponents = 1;
    G4Element* elSi = nist->FindOrBuildElement("Si");

    Silicon = new G4Material("Silicon", density, ncomponents);
    Silicon->AddElement(elSi, 1.0);

    //density = 2.33 * g/ cm3; 
    //G4Material* Silicon = new G4Material(name = "Silicon", density, ncomponents = 1);
    //Silicon->AddElement(elSi, fractionmass1 = 1.0);
    // End instantiation of lattice

    // Begin definition of lattice manager and loading Si lattice
    latManager = G4LatticeManager::GetLatticeManager();
    latManager->LoadLattice(Silicon, "Si");
    // End definition of lattice manager and loading Si lattice

    // Begin attach a physical lattice to the Si physical volume
    G4LatticePhysical* latticeSi = new G4LatticePhysical(latManager->GetLattice(Silicon));
    latticeSi->SetMillerOrientation(1, 0, 0, 45.*deg);
    latManager->RegisterLattice(physSi, latticeSi);
    // Begin attach a physical lattice to the Si physical volume
    
    // Al qubit

//    G4double Alhalfthickness = 100*nm;
    
    G4Box* solidAl =
    new G4Box("Al_box",                    //its name
              2.5*mm, 2.5*mm, 100*nm); //its size  .25 3mm 3mm
    
    G4LogicalVolume* logicAl =
    new G4LogicalVolume(solidAl,            //its solid
                        Al_mat,             //its material
                        "Al_box");         //its name
    
    new G4PVPlacement(0,                       //no rotation
                      G4ThreeVector(0,0,-3.5499*mm),         //at (0,0,0)  should be mm XXXXXXXXX
                      logicAl,                //its logical volume
                      "Al_box",              //its name
                      logicWorld,              //its mother  volume
                      false,                   //no boolean operation
                      0,                       //copy number
                      checkOverlaps);          //overlaps checking
    
    
                    
  // Set a shape as scoring volume
  //
    fScoringVolume = logicAl;

  //Visualization
  G4VisAttributes* boxVisAtt1= new G4VisAttributes(G4Colour(0.2,1,0.2,.3)); //Ar Box (Solid green)
  G4VisAttributes* boxVisAtt2= new G4VisAttributes(G4Colour(0,0,1,0.3)); //World (transparent blue)
      // G4VisAttributes* boxVisAtt3= new G4VisAttributes(G4Colour(0,0,1,0.3)); //
  G4VisAttributes* boxVisAtt4= new G4VisAttributes(G4Colour(1,0.1,0.1,0.3));
  logicSi ->SetVisAttributes(boxVisAtt1);
  logicCu ->SetVisAttributes(boxVisAtt4);
  //    logicAl ->SetVisAttributes(boxVisAtt3);
  logicWorld ->SetVisAttributes(boxVisAtt2); //white by default
  //
  //always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
