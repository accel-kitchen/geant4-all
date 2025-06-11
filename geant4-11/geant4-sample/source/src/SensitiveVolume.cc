//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SensitiveVolume.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "SensitiveVolume.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4ParticleDefinition.hh"
#include "G4HCofThisEvent.hh"
#include "G4UnitsTable.hh"
#include <iostream>
#include <fstream>
//------------------------------------------------------------------------------
  SensitiveVolume::SensitiveVolume(G4String name)
  : G4VSensitiveDetector(name)
{}
//------------------------------------------------------------------------------
  SensitiveVolume::~SensitiveVolume()
{}
//------------------------------------------------------------------------------
  void SensitiveVolume::Initialize(G4HCofThisEvent*)
{

     sum_eDep_1 = 0.;
     sum_stepLength_1 =0.; 
     sum_eDep_2 = 0.;
     sum_stepLength_2 =0.;
     
     // 記録済みトラックIDをクリア（イベント開始時）
     recordedTrackIDs.clear();
     
     // トラックヒット情報をクリア（イベント開始時）
     trackHits.clear();

   // G4cout << "==  Initialize sum_eDeps and sum_stepLengths   " <<G4endl;
}
//------------------------------------------------------------------------------
  void SensitiveVolume::EndOfEvent(G4HCofThisEvent*)
{
   // コインシデンス測定のロジック
   // 各トラックについて、上下の検出器でコインシデンスがあったかチェック
   
   for (const auto& trackEntry : trackHits) {
     G4int trackID = trackEntry.first;
     const std::vector<G4int>& hitDetectors = trackEntry.second;
     
     // このトラックが既に記録済みかチェック
     if (recordedTrackIDs.find(trackID) != recordedTrackIDs.end()) {
       continue; // 既に記録済みなのでスキップ
     }
     
     // ヒットした検出器をセットに変換（重複除去のため）
     std::set<G4int> hitSet(hitDetectors.begin(), hitDetectors.end());
     
     // 各検出器ペアについてコインシデンスをチェック
     // 下段: CopyNo 0-20 (X=-6.0mから6.0m)
     // 上段: CopyNo 21-41 (X=-6.0mから6.0m)
     // ペア関係: 下段CopyNo i に対して上段CopyNo (i+21)
     
     for (G4int lowerCopyNo = 0; lowerCopyNo <= 20; lowerCopyNo++) {
       G4int upperCopyNo = lowerCopyNo + 21;
       
       // 上下両方の検出器にヒットしているかチェック
       if (hitSet.find(lowerCopyNo) != hitSet.end() && 
           hitSet.find(upperCopyNo) != hitSet.end()) {
         
         // コインシデンス検出！上段の検出器CopyNoを記録
         std::ofstream outFile("tracking.dat", std::ios::app);
         outFile << std::setw(4) << upperCopyNo << "\t";
         outFile.close();
         
         // このトラックを記録済みとしてマーク
         recordedTrackIDs.insert(trackID);
         
         // 1つのトラックにつき1回だけ記録するのでbreak
         break;
       }
     }
   }

   // G4cout <<  " sum_eDep_1 = " << G4BestUnit(sum_eDep_1, "Energy") << " sum_stepLength_1 = " << G4BestUnit(sum_stepLength_1, "Length" ) <<G4endl;
   // G4cout <<  " sum_eDep_2 = "<<  G4BestUnit(sum_eDep_2, "Energy") << " sum_stepLength_2 = " << G4BestUnit(sum_stepLength_2, "Length" )<<G4endl;
}

//------------------------------------------------------------------------------
  G4bool SensitiveVolume::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
   // コンソール出力を削除してパフォーマンス向上
   // G4cout << "=== SenstiveVolume-ProcessHits:  ===" << G4endl;

// Retrieve information from the track object
   G4int idTrack = aStep->GetTrack()->GetTrackID();
   G4double tEnergy = aStep->GetTrack()->GetKineticEnergy();
   G4String particleName = aStep->GetTrack()->GetDefinition()->GetParticleName();

// get prePoint coordinates and copy Number and Volume name
   G4StepPoint* pre = aStep->GetPreStepPoint() ;
   G4int copyNo = pre->GetPhysicalVolume()->GetCopyNo() ;
   
   // トラックIDごとにヒットした検出器のCopyNoを記録
   trackHits[idTrack].push_back(copyNo);

  return true;
}
