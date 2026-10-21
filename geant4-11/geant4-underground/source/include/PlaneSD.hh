//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PlaneSD.hh
// Stage A の記録面。下向きに入ってきた粒子を位相空間の表に書いて止める。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef PlaneSD_h
#define PlaneSD_h 1

#include "G4VSensitiveDetector.hh"

class PlaneSD : public G4VSensitiveDetector {
public:
  explicit PlaneSD(const G4String& name) : G4VSensitiveDetector(name) {}
  ~PlaneSD() override = default;

  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
};

#endif
