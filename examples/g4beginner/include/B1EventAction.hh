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
/// \file B1EventAction.hh
/// \brief Definition of the B1EventAction class

#ifndef B1EventAction_h
#define B1EventAction_h 1

#include "G4UserEventAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"

//Start G4CMP application
#include "G4ThreeVector.hh"

#include <set>
#include <vector>
#include <map>
//End G4CMP application
  
class B1RunAction;

/// Event action class
///

class B1EventAction : public G4UserEventAction
{
  public:
    B1EventAction(B1RunAction* runAction);
    virtual ~B1EventAction();

    virtual void BeginOfEventAction(const G4Event* event);
    virtual void EndOfEventAction(const G4Event* event);

    void AddEdep(G4double edep) { fEdep += edep; }

    //Start G4CMP application
    // Electron bookkeeping
    void CountElectronInSi(G4int trackID, G4double weight);
    void CountElectronReachedInterface(G4int trackID, G4double weight);
    void CountElectronTrappedInSi(G4int trackID, G4double weight);

    // Weighted accessors
    G4double GetWeightedElectronsInSi() const;
    G4double GetWeightedElectronsReachedInterface() const;
    G4double GetWeightedElectronsTrappedInSi() const;
    G4double GetWeightedElectronReachFraction() const;

    // Electron interface observables
    void AddInterfaceElectronKE(G4double ke);
    void AddInterfaceElectronVperp(G4double vperp);
    void AddInterfaceElectronTime(G4double time);
    void AddInterfaceElectronPosition(const G4ThreeVector& pos);

    // Accessors if needed later
    G4int GetNElectronsInSi() const { return static_cast<G4int>(fElectronTracksInSi.size()); }
    G4int GetNElectronsReachedInterface() const { return static_cast<G4int>(fElectronTracksReachedInterface.size()); }
    G4int GetNElectronsTrappedInSi() const { return static_cast<G4int>(fElectronTracksTrappedInSi.size()); }

    G4double GetElectronReachFraction() const;

    //End G4CMP application
    static G4int GetShotNumber() { return fShotNumber; } // Static method to get shot number

  private:
    B1RunAction* fRunAction;
    //SteppingAction* fSteppingAction; // Pointer to SteppingAction
    static G4int fShotNumber; // Static variable to keep track of shot number
    G4double     fEdep;

    //Start G4CMP application
    // Unique electron track bookkeeping
    std::map<G4int, G4double> fElectronTracksInSi;
    std::map<G4int, G4double> fElectronTracksReachedInterface;
    std::map<G4int, G4double> fElectronTracksTrappedInSi;

    // Interface-hit observables for electrons
    std::vector<G4double>      fElectronInterfaceKE;
    std::vector<G4double>      fElectronInterfaceVperp;
    std::vector<G4double>      fElectronInterfaceTime;
    std::vector<G4ThreeVector> fElectronInterfacePos;
    //End G4CMP application

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

    
