//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ScintSD.hh
// シンチレータピクセルの L0 応答。ピクセルごとにエネルギー付与を集め、
// しきい値を超えたものをヒットとして表に書く。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef ScintSD_h
#define ScintSD_h 1

#include "G4VSensitiveDetector.hh"
#include <map>

class ScintSD : public G4VSensitiveDetector {
public:
  explicit ScintSD(const G4String& name) : G4VSensitiveDetector(name) {}
  ~ScintSD() override = default;

  void Initialize(G4HCofThisEvent*) override;
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
  void EndOfEvent(G4HCofThisEvent*) override;

private:
  struct Acc { G4double edep = 0.0; G4double t = 1.0e99; };
  std::map<G4int, Acc> fHits;  // key: CopyNo
};

#endif
