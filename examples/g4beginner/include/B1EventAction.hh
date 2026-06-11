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

#include "G4ThreeVector.hh"

#include <vector>
#include <map>

class B1RunAction;

class B1EventAction : public G4UserEventAction
{
  public:
    B1EventAction(B1RunAction* runAction);
    virtual ~B1EventAction();

    virtual void BeginOfEventAction(const G4Event* event);
    virtual void EndOfEventAction(const G4Event* event);

    void AddEdep(G4double edep) { fEdep += edep; }

    // Electron bookkeeping
    void CountElectronInSi(G4int trackID, G4double weight);
    void CountElectronReachedInterface(G4int trackID, G4double weight);
    void CountElectronTrappedInSi(G4int trackID, G4double weight);

    // Weighted electron accessors
    G4double GetNElectronsInSi() const;
    G4double GetNElectronsReachedInterface() const;
    G4double GetNElectronsTrappedInSi() const;
    G4double GetElectronReachFraction() const;

    // Electron interface observables
    void AddInterfaceElectronKE(G4double ke);
    void AddInterfaceElectronVperp(G4double vperp);
    void AddInterfaceElectronTime(G4double time);
    void AddInterfaceElectronPosition(const G4ThreeVector& pos);

    // Phonon event-level bookkeeping
    void AddPhononInterfaceHit(G4double weight, G4double energy);

    static G4int GetShotNumber() { return fShotNumber; }

  private:
    B1RunAction* fRunAction;
    static G4int fShotNumber;
    G4double     fEdep;

    // Unique electron track bookkeeping with weights
    std::map<G4int, G4double> fElectronTracksInSi;
    std::map<G4int, G4double> fElectronTracksReachedInterface;
    std::map<G4int, G4double> fElectronTracksTrappedInSi;

    // Interface-hit observables for electrons
    std::vector<G4double>      fElectronInterfaceKE;
    std::vector<G4double>      fElectronInterfaceVperp;
    std::vector<G4double>      fElectronInterfaceTime;
    std::vector<G4ThreeVector> fElectronInterfacePos;

    // Phonon event-level accumulators
    G4double fPhononsInterfaceWeighted;
    G4double fPhononEnergyInterfaceWeighted;
    G4double fPhononsAbove2DeltaWeighted;
    G4double fPhononEnergyAbove2DeltaWeighted;
};

#endif
