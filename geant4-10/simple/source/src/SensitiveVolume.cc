//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SensitiveVolume.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "SensitiveVolume.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4ParticleDefinition.hh"
#include "G4HCofThisEvent.hh"
#include "G4RunManager.hh"
#include "Analysis.hh"
#include "json.hpp"
#include <iostream>
#include <fstream>
using json = nlohmann::json;
//------------------------------------------------------------------------------
  SensitiveVolume::SensitiveVolume(G4String name)
  : G4VSensitiveDetector(name)
//------------------------------------------------------------------------------
{}

//------------------------------------------------------------------------------
  SensitiveVolume::~SensitiveVolume()
//------------------------------------------------------------------------------
{}

//------------------------------------------------------------------------------
  void SensitiveVolume::Initialize(G4HCofThisEvent*)
//------------------------------------------------------------------------------
{
}

//------------------------------------------------------------------------------
  void SensitiveVolume::EndOfEvent(G4HCofThisEvent*)
//------------------------------------------------------------------------------
{
  //next run number
  run_number++;
}

//------------------------------------------------------------------------------
  G4bool SensitiveVolume::ProcessHits(G4Step* aStep, G4TouchableHistory*)
//------------------------------------------------------------------------------
{

//   G4cout << "=== SenstiveVolume-ProcessHits: Output of Information ===" << G4endl;

   G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

    
// energy deposit in this step and its accumulation over steps
    G4double edep = aStep->GetTotalEnergyDeposit();
    G4double stepLength = aStep->GetStepLength();

// Retrieve information from the track object
   G4int nTrack = aStep->GetTrack()->GetParentID();
   G4int nStep  = aStep->GetTrack()->GetCurrentStepNumber();
   G4String particleName = aStep->GetTrack()->GetDefinition()->GetParticleName();

if(aStep->GetTrack()->GetTrackStatus()!=fAlive) return false;
// Primary track is picked up
 //   if (nTrack != 1) return false;
      no_Step = no_Step + 1;    
// Get PreStepPoint and TouchableHandle objects
   G4StepPoint* preStepPoint=aStep->GetPreStepPoint();
   G4TouchableHandle theTouchable = preStepPoint->GetTouchableHandle();

  // Touchable infomation: Start position of the current step
  G4ThreeVector position_World = preStepPoint->GetPosition();
  G4ThreeVector position_Local = theTouchable->GetHistory()
                                  -> GetTopTransform().TransformPoint(position_World);

    G4double time =  preStepPoint->GetGlobalTime();
    G4double kE = preStepPoint->GetKineticEnergy();
    // Touchable information: Volume name and copy# at the current step
    G4String volumeName = theTouchable->GetVolume()->GetName();
    G4int copyNo = theTouchable->GetCopyNumber();

 
        std::ofstream o2("tracking.dat",std::ios::app);
if (nTrack==0){
      o2 << std::setw(4) << run_number << "\t" << copyNo <<"\t" << time <<"\t" << kE << "\t" << position_World.x()  << "\t"  << position_World.y()  << "\t"  << position_World.z()  << "\t"  << edep << std::endl;
}


  return true;
}






