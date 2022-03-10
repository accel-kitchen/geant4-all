//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SensitiveVolume.hh
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef SensitiveVolume_h
#define SensitiveVolume_h 1

#include "G4VSensitiveDetector.hh"

#include "json.hpp"
using json = nlohmann::json;

class G4Step;

//------------------------------------------------------------------------------
  class SensitiveVolume : public G4VSensitiveDetector
//------------------------------------------------------------------------------
{
  public:
      SensitiveVolume(G4String);
     ~SensitiveVolume();

      void Initialize(G4HCofThisEvent*);
      G4bool ProcessHits(G4Step*, G4TouchableHistory*);
      void EndOfEvent(G4HCofThisEvent*);
private:
    G4double sum_eDep;
    G4int no_Step=0;
    G4int pre_copyNo;
    G4int water_flag;
    G4int calorimeter_flag;
    G4int run_number=0;
    json tracking;           
    json tracking2;  
};
#endif
