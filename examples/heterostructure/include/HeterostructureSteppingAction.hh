/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file HeterostructureSteppingAction.hh
/// \brief Definition of the HeterostructureSteppingAction class

// 20260818  Selby Q. Dang (Stanford/SLAC)

#ifndef HeterostructureSteppingAction_hh
#define HeterostructureSteppingAction_hh 1

#include "G4TrackStatus.hh"
#include "G4UserSteppingAction.hh"

#include <fstream>

class G4Step;

class HeterostructureSteppingAction : public G4UserSteppingAction
{
public:

  HeterostructureSteppingAction();
  virtual ~HeterostructureSteppingAction();
  virtual void UserSteppingAction(const G4Step* step);
  void ExportStepInformation( const G4Step * step );

  std::string TrackStatusToString(G4TrackStatus status) {
    switch (status) {
        case fAlive:                   return "Alive";
        case fStopButAlive:            return "StopButAlive";
        case fStopAndKill:             return "StopAndKill";
        case fKillTrackAndSecondaries: return "KillTrackAndSecondaries";
        case fSuspend:                 return "Suspend";
        case fSuspendAndWait:          return "SuspendAndWait";
        case fPostponeToNextEvent:     return "PostponeToNextEvent";
        default:                       return "Unknown";
    }
  };
  
private:

  //Step info output file
  std::ofstream fOutputFile;
};

#endif