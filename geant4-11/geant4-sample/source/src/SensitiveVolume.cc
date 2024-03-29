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

   G4cout << "==  Initialize sum_eDeps and sum_stepLengths   " <<G4endl;
}
//------------------------------------------------------------------------------
  void SensitiveVolume::EndOfEvent(G4HCofThisEvent*)
{

   G4cout <<  " sum_eDep_1 = " << G4BestUnit(sum_eDep_1, "Energy") << " sum_stepLength_1 = " << G4BestUnit(sum_stepLength_1, "Length" ) <<G4endl;
   G4cout <<  " sum_eDep_2 = "<<  G4BestUnit(sum_eDep_2, "Energy") << " sum_stepLength_2 = " << G4BestUnit(sum_stepLength_2, "Length" )<<G4endl;
}

//------------------------------------------------------------------------------
  G4bool SensitiveVolume::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
   G4cout << "=== SenstiveVolume-ProcessHits:  ===" << G4endl;

// Retrieve information from the track object

   G4int idTrack = aStep->GetTrack()->GetTrackID();
   G4double tEnergy = aStep->GetTrack()->GetKineticEnergy();
   G4String particleName = aStep->GetTrack()->GetDefinition()->GetParticleName();

//   G4cout << "======  track ID , particle ID , Energy ====" << G4endl;
   G4cout << "  Track Number: " << idTrack 
          << "  Particle: " << particleName << "  Energy: " << tEnergy << G4endl;

// get prePoint coordinates andcopy Number and  Volume name
   G4StepPoint* pre = aStep->GetPreStepPoint() ;
   G4int copyNo = pre->GetPhysicalVolume()->GetCopyNo() ;
   G4ThreeVector position_World = pre->GetPosition();
   G4String volumeName = pre->GetPhysicalVolume()->GetName();
  G4double time =  pre->GetGlobalTime();
  G4double kE = pre->GetKineticEnergy();
   G4cout << "   World(x,y,z): " 
         << position_World.x() << ", " << position_World.y() << ", " 
         << position_World.z()  << " copyNo : " << copyNo << "   Volume name " << volumeName << G4endl; 
 
// get total energy deposit in this step "edep"  and  accumulate it in "sum_eDep"
    G4double edep = aStep->GetTotalEnergyDeposit();

        std::ofstream o2("tracking.dat",std::ios::app);
      o2 << std::setw(4) << idTrack << "\t" << copyNo <<"\t" << time <<"\t" << kE << "\t" << position_World.x()  << "\t"  << position_World.y()  << "\t"  << position_World.z()  << "\t"  << edep << std::endl;

  return true;
}






