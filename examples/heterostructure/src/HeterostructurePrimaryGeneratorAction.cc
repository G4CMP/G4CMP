/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

#include "HeterostructurePrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4Geantino.hh"
#include "G4ParticleGun.hh"
#include "G4RandomDirection.hh"
#include "G4PhononTransFast.hh"
#include "G4PhononTransSlow.hh"
#include "G4PhononLong.hh"
#include "G4SystemOfUnits.hh"
#include "G4CMPDriftElectron.hh"
#include "G4CMPDriftHole.hh"

using namespace std;

HeterostructurePrimaryGeneratorAction::HeterostructurePrimaryGeneratorAction() { 
  G4int n_particle = 1;
  fParticleGun  = new G4ParticleGun(n_particle);   

  // default particle kinematics 
  fParticleGun->SetParticleDefinition(G4Geantino::Definition());
  fParticleGun->SetParticleMomentumDirection(G4RandomDirection());
  fParticleGun->SetParticlePosition(G4ThreeVector(0.0,0.0,90.*nm));
  fParticleGun->SetParticleEnergy(1e-6*eV);  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....


HeterostructurePrimaryGeneratorAction::~HeterostructurePrimaryGeneratorAction() {
  delete fParticleGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

 
// void HeterostructurePrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
//   if (fParticleGun->GetParticleDefinition() == G4Geantino::Definition()) {
//     G4double selector = G4UniformRand();
//     if (selector<0) {//<1) { //SQD: Silly vis exercise
//       fParticleGun->SetParticleDefinition(G4CMPDriftElectron::Definition()); 
//       std::cout<<"SQD: generated a drift electron"<<std::endl;
//     } 
//     else {
//       fParticleGun->SetParticleDefinition(G4CMPDriftHole::Definition());
//       std::cout<<"SQD: generated a drift hole"<<std::endl;
//     }
//   }

//   fParticleGun->SetParticleMomentumDirection(G4RandomDirection());
//   fParticleGun->GeneratePrimaryVertex(anEvent);
// }

void HeterostructurePrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
  if (fParticleGun->GetParticleDefinition() == G4Geantino::Definition()) {
    // Generate hole
    fParticleGun->SetParticleDefinition(G4CMPDriftHole::Definition());
    fParticleGun->SetParticleMomentumDirection(G4RandomDirection());
    fParticleGun->GeneratePrimaryVertex(anEvent);
    
    // Generate electron with different random direction
    fParticleGun->SetParticleDefinition(G4CMPDriftElectron::Definition());
    fParticleGun->SetParticleMomentumDirection(G4RandomDirection());
    fParticleGun->GeneratePrimaryVertex(anEvent);

    // Restore "not set" condition for next event
    fParticleGun->SetParticleDefinition(G4Geantino::Definition());
  } else {
    fParticleGun->GeneratePrimaryVertex(anEvent);
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....


