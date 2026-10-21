//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PlaneSD.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "PlaneSD.hh"
#include "MuoEventInfo.hh"
#include "MuoRun.hh"
#include "MuoTables.hh"

#include "G4AnalysisManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"

//------------------------------------------------------------------------------
  G4bool PlaneSD::ProcessHits(G4Step* step, G4TouchableHistory*)
//------------------------------------------------------------------------------
{
  const auto* pre = step->GetPreStepPoint();
  if (pre->GetStepStatus() != fGeomBoundary) return false;  // 入ってきた最初のステップだけ

  auto* track = step->GetTrack();
  if (track->GetMomentumDirection().z() >= 0.0) return false;  // 下向きのみ

  const auto* event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
  const auto* info = dynamic_cast<const MuoEventInfo*>(event->GetUserInformation());
  if (!info) return false;

  MuoTables::FillPhaseSpace(G4AnalysisManager::Instance(), *info,
                            track->GetDefinition()->GetPDGEncoding(), pre->GetKineticEnergy(),
                            pre->GetPosition(), pre->GetMomentumDirection(), pre->GetGlobalTime(),
                            track->GetTrackID(), track->GetParentID());
  static_cast<MuoRun*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun())->AddPlaneRow();

  track->SetTrackStatus(fStopAndKill);
  return true;
}
