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
/// \file B1PrimaryGeneratorAction.cc
/// \brief Implementation of the B1PrimaryGeneratorAction class

#include "B1PrimaryGeneratorAction.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4RandomDirection.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1PrimaryGeneratorAction::B1PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(),
  fParticleGun(0)//,
  //fEnvelopeBox(0)
{
  G4int n_particle = 1;
  fParticleGun  = new G4ParticleGun(n_particle);
//    fParticleGun->SetParticleEnergy(0.511*MeV);
/*
  // default particle kinematic
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  G4ParticleDefinition* particle
    = particleTable->FindParticle(particleName="gamma");
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticlePosition(G4ThreeVector(-195*mm,0,0));
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(1, 0, 0));
  //fParticleGun->SetParticleEnergy(0.1*MeV);

 */
 }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1PrimaryGeneratorAction::~B1PrimaryGeneratorAction()
{
  delete fParticleGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  //this function is called at the begining of each event
  //

  
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  G4ParticleDefinition* particle
    = particleTable->FindParticle(particleName="e-"); //JESSE, change this to change particles, then change energy distribution below
  fParticleGun->SetParticleDefinition(particle);
  //fParticleGun->SetParticleEnergy(0.278*MeV);
  //fParticleGun->SetParticlePosition(G4ThreeVector(3.8*mm,0,0));//186.95*mm 0 0
  //fParticleGun->SetParticleMomentumDirection(G4ThreeVector(-1, 0, 0));
  //fParticleGun->SetParticleEnergy(0.1*MeV);
   
    
    /*  //Older random direction function
     
    G4double a,b,c,n;
    
    do {
        a = (G4UniformRand()-0.5)/0.5;
        b = (G4UniformRand()-0.5)/0.5;
        c = (G4UniformRand()-0.5)/0.5;
        n = a*a+b*b+c*c;
    } while (n > 1 || n == 0.0 || a < 0.9848);
    n = std::sqrt(n);
    a /= n;
    b /= n;
    c /= n;
    G4ThreeVector direction(-a,-b,-c);
    fParticleGun->SetParticleMomentumDirection(direction);
    
    */
    

    /*
     //    Trying to make a peaked at .29 triangle distribution of energy for beta +.  JESSE, comment section out ifyou're not using
    
    G4double Emax=0.653; // for beta +, change when relevant
    G4double E, R1, R2, P;
    do{
    R1=G4UniformRand(); //the energy (when times Emax)
    R2=G4UniformRand(); //the y axis for the triangle shape, to veto if above triangle
    P=1-R1; //prob to throw it out
    } while ( R2>(3.4483*R1) || R2>(-1.4085*R1+1.4085) );  //this throws things out and make do repeat again
    E=R1*Emax;
    */


    
    
     //    Trying to make a descending triangle distribution of energy for beta -.   JESSE, comment section out ifyou're not using

    G4double Emax=0.580; // for beta -, change when relevant
    G4double E, R1, R2;
    do{
    R1=G4UniformRand();
    R2=G4UniformRand();
    } while (R2>(1-R1));  //this throws things out and make do repeat again
    E=R1*Emax;
    
    
   
    //G4double E = 1.346*MeV; //single energy for gammas   JESSE, comment section out ifyou're not using
  
    
    
    fParticleGun->SetParticleEnergy(E*MeV);
    
//   G4cout << "energy " << E << G4endl;

//      G4cout << G4endl << E*MeV;

     //this is a good random direction function
    G4double cosTheta = -1; //0.999;
    G4ThreeVector v = G4RandomDirection(cosTheta);
    v.set(v.x(), v.y(), v.z());
    
    fParticleGun->SetParticleMomentumDirection(-v);
    
  
    //disk source
    G4double radius = 3.75*mm;//3.75*mm;
    G4double phi =  360.*deg;
    
    G4double u = G4UniformRand() + G4UniformRand();
    G4double rRan = radius*(( u>1) ? 2-u : u);
    
    G4double phiRan = phi*G4UniformRand();
    G4double z0 = 0;//3.8*mm;
    
    G4double depth = (G4UniformRand() - .5)*.5*mm;//should be .5 at end XXXXXXX
    
    fParticleGun->SetParticlePosition(G4ThreeVector(rRan*cos(phiRan),rRan*sin(phiRan),z0+depth));
 
   
   //   fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));
  //    fParticleGun->SetParticlePosition(G4ThreeVector(rRan*cos(phiRan)-9.18*cm,rRan*sin(phiRan),z0));

    
//    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0,0,-1)); //pointing from copper to silicon
//    fParticleGun->SetParticlePosition(G4ThreeVector(0,0,-3.8*mm)); //between copper and silicon
    
    fParticleGun->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

