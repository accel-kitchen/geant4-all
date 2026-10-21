//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ScintSD.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "ScintSD.hh"
#include "MuoConfig.hh"
#include "MuoEventInfo.hh"
#include "MuoRun.hh"
#include "MuoTables.hh"

#include "G4AnalysisManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TouchableHistory.hh"
#include <algorithm>

//------------------------------------------------------------------------------
  void ScintSD::Initialize(G4HCofThisEvent*)
//------------------------------------------------------------------------------
{
  fHits.clear();
}

//------------------------------------------------------------------------------
  G4bool ScintSD::ProcessHits(G4Step* step, G4TouchableHistory*)
//------------------------------------------------------------------------------
{
  const G4double edep = step->GetTotalEnergyDeposit();
  if (edep <= 0.0) return false;
  const auto* pre = step->GetPreStepPoint();
  auto& a = fHits[pre->GetTouchable()->GetCopyNumber()];
  a.edep += edep;
  a.t = std::min(a.t, pre->GetGlobalTime());
  return true;
}

//------------------------------------------------------------------------------
  void ScintSD::EndOfEvent(G4HCofThisEvent*)
//------------------------------------------------------------------------------
{
  const auto& c = MuoConfig::Get();
  const auto* event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
  const auto* info = dynamic_cast<const MuoEventInfo*>(event->GetUserInformation());
  if (!info) return;

  // しきい値を超えたピクセルだけを数える
  G4bool layer1 = false, layer2 = false;
  for (const auto& kv : fHits) {
    if (kv.second.edep < c.threshold) continue;
    (kv.first/10000 == 1 ? layer1 : layer2) = true;
  }
  const G4bool coincidence = layer1 && layer2;

  if (coincidence) {
    auto* run = static_cast<MuoRun*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());
    run->AddCoincidence(info->w1);
  }
  if (c.coincidenceOnly && !coincidence) return;

  auto* am = G4AnalysisManager::Instance();
  for (const auto& kv : fHits) {
    if (kv.second.edep < c.threshold) continue;
    const G4int copyNo = kv.first;
    MuoTables::FillHit(am, *info, copyNo/10000, (copyNo/100) % 100, copyNo % 100, kv.second.edep, kv.second.t);
  }
}
