/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file HeterostructureSteppingAction.cc
/// \brief Implementation of the HeterostructureSteppingAction class

// 20260818  Selby Q. Dang (Stanford/SLAC)

#include "HeterostructureSteppingAction.hh"
#include "HeterostructureConfigManager.hh"
#include <iostream>
#include <iomanip>
#include "globals.hh"
#include "G4CMPDriftTrackInfo.hh"
#include "G4CMPProcessUtils.hh"
#include "G4CMPTrackUtils.hh"
#include "G4CMPUtils.hh"
#include "G4CMPVTrackInfo.hh"
#include "G4Run.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4Threading.hh"
#include "G4RunManager.hh"
#include "G4StepPoint.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//Default constructor
HeterostructureSteppingAction::HeterostructureSteppingAction() {
}

HeterostructureSteppingAction::~HeterostructureSteppingAction() {
  fOutputFile.close();
}

//Alternative constructor
void HeterostructureSteppingAction::UserSteppingAction(const G4Step* step) {
  if (!fOutputFile.is_open()) {
    G4String stepFileName = HeterostructureConfigManager::GetStepOutput();
    G4cout << "StepFileName: " << stepFileName << G4endl;

    fOutputFile.open(stepFileName,std::ios::trunc);
    //Output file header
    fOutputFile << "runNo" << " " << "eventNo" << " " << "trackNo"
                  << " " << "particleName" << " "
                  << "preSPName" << " "
                  << "preStepX_mm" << " " << "preStepY_mm"
                  << " " << "preStepZ_mm" << " " << "preStepT_ns" << " "
                  << "preStepEnergy_eV" << " " << "preStepKinEnergy_eV" << " "
                  << "postSPName" << " "
                  << "postStepX_mm" << " " << "postStepY_mm"
                  << " " << "postStepZ_mm" << " " << "postStepT_ns" << " "
                  << "postStepEnergy_eV" << " " << "postStepKinEnergy_eV" << " "
                  // << postStepV_mm_ns << " "
                  << "valley" << " " << "nReflections" << " "
                  << "stepProcess" << " " << "trackStatus" << " " << std::endl;
  }

  //For now, simple: look at the pre-step point volume name and the track name

  //First up: do generic exporting of step information (no cuts made here)
  ExportStepInformation(step);

  return;
}

// Do a set of queries of information to test for anharmonic decay
void HeterostructureSteppingAction::ExportStepInformation(const G4Step* step) {
  //Test
  G4StepPoint* preSP = step->GetPreStepPoint();
  G4StepPoint* postSP = step->GetPostStepPoint();
  std::string preSPName  = preSP->GetPhysicalVolume()->GetName();
  std::string postSPName = postSP->GetPhysicalVolume()->GetName();

  G4Track* track = step->GetTrack();
  if (G4CMP::IsChargeCarrier(track)) {
    int runNo = G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID();
    int eventNo = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    
    if (eventNo > 1000000) { return; }
    int trackNo = track->GetTrackID();
    std::string particleName = track->GetParticleDefinition()->GetParticleName();

    double preStepX_mm = preSP->GetPosition().x() / CLHEP::mm;
    double preStepY_mm = preSP->GetPosition().y() / CLHEP::mm;
    double preStepZ_mm = preSP->GetPosition().z() / CLHEP::mm;
    double preStepT_ns = preSP->GetGlobalTime() / CLHEP::ns;
    double preStepEnergy_eV = preSP->GetTotalEnergy() / CLHEP::eV;
    double preStepKinEnergy_eV = preSP->GetKineticEnergy() / CLHEP::eV;

    double postStepX_mm = postSP->GetPosition().x() / CLHEP::mm;
    double postStepY_mm = postSP->GetPosition().y() / CLHEP::mm;
    double postStepZ_mm = postSP->GetPosition().z() / CLHEP::mm;
    double postStepT_ns = postSP->GetGlobalTime() / CLHEP::ns;
    double postStepEnergy_eV = postSP->GetTotalEnergy() / CLHEP::eV;
    double postStepKinEnergy_eV = postSP->GetKineticEnergy() / CLHEP::eV;
    // Holding off on the velocity output until G4CMP-610,
    // when we can use G4VParticleChange to access the lattice info.
    // //Post step point velocity in lab frame
    // G4ThreeVector postStepV_mm_ns = GetGlobalVelocityVector(track);  
    // Valley at post step point, -1 for holes, >=0 for electrons
    G4int valley = G4CMP::GetTrackInfo<G4CMPDriftTrackInfo>(track)->ValleyIndex();

    
    //Get reflection count
    size_t nReflections =
    G4CMP::GetTrackInfo<G4CMPVTrackInfo>(track)->ReflectionCount();
    //Get track status
    std::string trackStatus = TrackStatusToString(track->GetTrackStatus());
    std::string stepProcess = postSP->GetProcessDefinedStep()->GetProcessName();
    
    //Fill the output file with the step info  
    fOutputFile << runNo << " " << eventNo << " " << trackNo
                << " " << particleName << " " 
                << preSPName << " "
                << std::setprecision(14) << preStepX_mm << " " << preStepY_mm
                << " " << preStepZ_mm << " " << preStepT_ns << " "
                << preStepEnergy_eV << " " << preStepKinEnergy_eV << " " 
                << postSPName << " "
                << std::setprecision(14) << postStepX_mm << " " << postStepY_mm 
                << " " << postStepZ_mm << " " << postStepT_ns << " " 
                << postStepEnergy_eV << " " << postStepKinEnergy_eV << " " 
                // << postStepV_mm_ns << " " 
                << valley << " " << nReflections << " "
                << stepProcess << " " << trackStatus << " " << std::endl;
  }
}