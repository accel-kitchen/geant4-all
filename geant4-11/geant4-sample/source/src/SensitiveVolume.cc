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
   // 2次元配列コインシデンス測定のロジック
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
     
     // モジュールごとにコインシデンスをチェック
     // CopyNo形式: AABXXYY (AA: モジュール00-20, B: 層0-1, XX: X位置00-19, YY: Y位置00-19)
     
     std::vector<std::pair<G4int, G4int>> coincidencePairs;
     
     for (G4int moduleID = 0; moduleID < 21; moduleID++) {
       // このモジュールでコインシデンスを探す
       std::set<G4int> lowerHits, upperHits;
       
       // モジュール内の上下層のヒットを分類
       for (G4int copyNo : hitSet) {
         G4int module = copyNo / 100000;
         G4int layer = (copyNo / 10000) % 10;
         
         if (module == moduleID) {
           if (layer == 0) {
             lowerHits.insert(copyNo);
           } else if (layer == 1) {
             upperHits.insert(copyNo);
           }
         }
       }
       
       // 上下両方にヒットがあるかチェック
       if (!lowerHits.empty() && !upperHits.empty()) {
         // 最初のヒットペアを記録
         G4int lowerCopyNo = *lowerHits.begin();
         G4int upperCopyNo = *upperHits.begin();
         coincidencePairs.push_back(std::make_pair(lowerCopyNo, upperCopyNo));
       }
     }
     
     // コインシデンスが検出された場合、出力ファイルに記録
     if (!coincidencePairs.empty()) {
       std::ofstream outFile("tracking.dat", std::ios::app);
       
       for (size_t i = 0; i < coincidencePairs.size(); i++) {
         if (i > 0) outFile << "\t"; // ペア間の区切り
         outFile << coincidencePairs[i].first << "\t" << coincidencePairs[i].second;
       }
       outFile << "\n"; // イベント終了で改行
       outFile.close();
       
       // このトラックを記録済みとしてマーク
       recordedTrackIDs.insert(trackID);
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
