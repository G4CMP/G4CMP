/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

// 20261103 G4CMP-692 -- Initial implementation

#include "G4CMPQPHit.hh"

#include "G4ios.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4AttDefStore.hh"
#include "G4AttDef.hh"
#include "G4AttValue.hh"
#include "G4UnitsTable.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"


G4ThreadLocal G4Allocator<G4CMPQPHit>* G4CMPQPHitAllocator=0;

G4CMPQPHit::G4CMPQPHit() {}

G4bool G4CMPQPHit::operator==(const G4CMPQPHit &right) const {
  G4bool val = trackID == right.GetTrackID();
  if (val) val &= stepID == right.GetStepID();
  if (val) val &= startTime == right.GetStartTime();
  if (val) val &= finalTime == right.GetFinalTime();
  return val;
}

void G4CMPQPHit::Draw() {
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if (pVVisManager) {
    G4Circle circle(finalPos);
    circle.SetScreenSize(15);
    circle.SetFillStyle(G4Circle::filled);
    G4Colour colour(0.65,0.,0.65);
    G4VisAttributes attribs(colour);
    attribs.SetStartTime(startTime);
    attribs.SetEndTime(finalTime);
    circle.SetVisAttributes(attribs);
    pVVisManager->Draw(circle);
  }
}

const std::map<G4String,G4AttDef>* G4CMPQPHit::GetAttDefs() const {
  G4bool isNew;
  std::map<G4String,G4AttDef>* store
    = G4AttDefStore::GetInstance("G4CMPQPHit",isNew);
  if (isNew) {
    G4String HitType("HitType");
    (*store)[HitType] = G4AttDef(HitType,"Hit Type","Physics","","G4String");

    G4String sTime("sTime");
    (*store)[sTime] = G4AttDef(sTime,"Start Time","Physics","G4BestUnit","G4double");

    G4String fTime("fTime");
    (*store)[fTime] = G4AttDef(fTime,"Final Time","Physics","G4BestUnit","G4double");

    G4String Weight("weight");
    (*store)[Weight] = G4AttDef(Weight, "Weight","Physics","","G4double");

    G4String sPos("sPos");
    (*store)[sPos] = G4AttDef(sPos, "Start Position",
          "Physics","G4BestUnit","G4ThreeVector");

    G4String fPos("fPos");
    (*store)[fPos] = G4AttDef(fPos, "Final Position",
          "Physics","G4BestUnit","G4ThreeVector");

    G4String num_qp("Nqp");
    (*store)[num_qp] = G4AttDef(num_qp, "Num Quasiparticles","Physics","","G4double");
  }
  return store;
}

std::vector<G4AttValue>* G4CMPQPHit::CreateAttValues() const {
  std::vector<G4AttValue>* values = new std::vector<G4AttValue>;

  values->push_back(G4AttValue("HitType","G4CMPQPHit",""));

  values->push_back
    (G4AttValue("fTime",G4BestUnit(finalTime,"fTime"),""));

  values->push_back
    (G4AttValue("sTime",G4BestUnit(startTime,"sTime"),""));

  values->push_back
    (G4AttValue("Weight",G4BestUnit(weight,""),""));

  values->push_back
    (G4AttValue("fPos",G4BestUnit(finalPos,"Length"),""));

  values->push_back
    (G4AttValue("sPos",G4BestUnit(startPos,"Length"),""));

  values->push_back
    (G4AttValue("Nqp",G4BestUnit(Nqp,""),""));

  return values;
}

void G4CMPQPHit::Print() {
  G4cout << "trk=" << trackID << " step=" << stepID << " pos0=" << startPos/mm << "mm t0=" << startTime/us << "us pos1=" << finalPos/mm << " mm t1=" << finalTime/us << "us wgt=" << weight << " Nqp=" << Nqp << G4endl;
}


