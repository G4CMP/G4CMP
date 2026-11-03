/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

// 20261103 G4CMP-692 -- Initial implementation

#ifndef G4CMPQPHit_h
#define G4CMPQPHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

class G4AttDef;
class G4AttValue;

class G4CMPQPHit : public G4VHit {
public:
  G4CMPQPHit();
  G4bool operator==(const G4CMPQPHit &right) const;

  inline void *operator new(size_t);
  inline void operator delete(void *aHit);

  virtual void Draw() override;
  virtual const std::map<G4String, G4AttDef>* GetAttDefs() const override;
  virtual std::vector<G4AttValue>* CreateAttValues() const override;
  virtual void Print() override;

  void SetTrackID(G4int id) { trackID = id; }
  G4int GetTrackID() const { return trackID; }

  void SetStepID(G4int id) { stepID = id; }
  G4int GetStepID() const { return stepID; }

  void SetStartTime(G4double t) { startTime = t; }
    G4double GetStartTime() const { return startTime; }

  void SetFinalTime(G4double t) { finalTime = t; }
  G4double GetFinalTime() const { return finalTime; }

  void SetTime(G4double t) { SetFinalTime(t); }
  G4double GetTime() const { return GetFinalTime(); }

  void SetWeight(G4double w) { weight = w; }
    G4double GetWeight() const { return weight; }

  void SetStartPosition(G4ThreeVector xyz) { startPos = xyz; }
  G4ThreeVector GetStartPosition() const { return startPos; }

  void SetFinalPosition(G4ThreeVector xyz) { finalPos = xyz; }
  G4ThreeVector GetFinalPosition() const { return finalPos; }

  void SetNqp(G4double val) { Nqp = val; }
  G4double GetNqp() const { return Nqp; }

private:
  G4int trackID;
  G4int stepID;
  G4double startTime;
  G4double finalTime;
  G4double weight;
  G4ThreeVector startPos;
  G4ThreeVector finalPos;
  G4double Nqp;
}; // class G4CMPQPHit

typedef G4THitsCollection<G4CMPQPHit> G4CMPQPHitsCollection;
using G4CMPQPWaveform = G4CMPQPHitsCollection;

extern G4ThreadLocal G4Allocator<G4CMPQPHit>* G4CMPQPHitAllocator;

inline void* G4CMPQPHit::operator new(size_t) {
  if (!G4CMPQPHitAllocator)
    G4CMPQPHitAllocator = new G4Allocator<G4CMPQPHit>;
  return (void*)G4CMPQPHitAllocator->MallocSingle();
}

inline void G4CMPQPHit::operator delete(void* aHit) {
  G4CMPQPHitAllocator->FreeSingle((G4CMPQPHit*) aHit);
}

#endif  /* G4CMPQPHit_h */
