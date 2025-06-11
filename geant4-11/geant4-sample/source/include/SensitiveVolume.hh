//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SensitiveVolume.hh
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef SensitiveVolume_h
#define SensitiveVolume_h 1

#include "G4VSensitiveDetector.hh"
#include <set>
#include <map>
#include <vector>
class G4Step;

//------------------------------------------------------------------------------
  class SensitiveVolume : public G4VSensitiveDetector
//------------------------------------------------------------------------------
{
  public:
      SensitiveVolume(G4String);
     ~SensitiveVolume();

      void Initialize(G4HCofThisEvent*) override;
      G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
      void EndOfEvent(G4HCofThisEvent*)override;
private:
    G4double sum_eDep_1;
    G4double sum_stepLength_1;
    G4double sum_eDep_2;
    G4double sum_stepLength_2;
    
    // 記録済みトラックIDを管理（1つのミュオンにつき最初のCopyNoのみ記録）
    std::set<G4int> recordedTrackIDs;
    
    // コインシデンス測定用メンバ変数
    // トラックIDごとのヒットした検出器CopyNoを記録
    std::map<G4int, std::vector<G4int>> trackHits;

};
#endif
