//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuoTables.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "MuoTables.hh"
#include "MuoEventInfo.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

namespace MuoTables {

namespace {
  void CommonColumns(G4AnalysisManager* am, G4int id)
  {
    am->CreateNtupleIColumn(id, "event");
    am->CreateNtupleDColumn(id, "w1");
    am->CreateNtupleDColumn(id, "E0");
    am->CreateNtupleDColumn(id, "cth0");
    am->CreateNtupleDColumn(id, "ph0");
  }
  void FillCommon(G4AnalysisManager* am, const MuoEventInfo& info)
  {
    am->FillNtupleIColumn(0, 0, info.srcEvent);
    am->FillNtupleDColumn(0, 1, info.w1);
    am->FillNtupleDColumn(0, 2, info.E0);
    am->FillNtupleDColumn(0, 3, info.cth0);
    am->FillNtupleDColumn(0, 4, info.ph0);
  }
}

//------------------------------------------------------------------------------
  void Create(G4AnalysisManager* am, const G4String& tableName)
//------------------------------------------------------------------------------
{
  const G4int id = am->CreateNtuple(tableName, tableName);
  CommonColumns(am, id);
  if (tableName == "phasespace") {
    am->CreateNtupleIColumn(id, "pdg");
    am->CreateNtupleDColumn(id, "ekin");
    am->CreateNtupleDColumn(id, "x");
    am->CreateNtupleDColumn(id, "y");
    am->CreateNtupleDColumn(id, "z");
    am->CreateNtupleDColumn(id, "ux");
    am->CreateNtupleDColumn(id, "uy");
    am->CreateNtupleDColumn(id, "uz");
    am->CreateNtupleDColumn(id, "t");
    am->CreateNtupleIColumn(id, "trk");
    am->CreateNtupleIColumn(id, "parent");
  } else {
    am->CreateNtupleIColumn(id, "layer");
    am->CreateNtupleIColumn(id, "ix");
    am->CreateNtupleIColumn(id, "iy");
    am->CreateNtupleDColumn(id, "edep");
    am->CreateNtupleDColumn(id, "t");
  }
  am->FinishNtuple(id);
}

//------------------------------------------------------------------------------
  void FillHit(G4AnalysisManager* am, const MuoEventInfo& info,
               G4int layer, G4int ix, G4int iy, G4double edep, G4double t)
//------------------------------------------------------------------------------
{
  FillCommon(am, info);
  am->FillNtupleIColumn(0, 5, layer);
  am->FillNtupleIColumn(0, 6, ix);
  am->FillNtupleIColumn(0, 7, iy);
  am->FillNtupleDColumn(0, 8, edep/MeV);
  am->FillNtupleDColumn(0, 9, t/ns);
  am->AddNtupleRow(0);
}

//------------------------------------------------------------------------------
  void FillPhaseSpace(G4AnalysisManager* am, const MuoEventInfo& info,
                      G4int pdg, G4double ekin, const G4ThreeVector& pos,
                      const G4ThreeVector& dir, G4double t, G4int trk, G4int parent)
//------------------------------------------------------------------------------
{
  FillCommon(am, info);
  am->FillNtupleIColumn(0, 5, pdg);
  am->FillNtupleDColumn(0, 6, ekin/MeV);
  am->FillNtupleDColumn(0, 7, pos.x()/mm);
  am->FillNtupleDColumn(0, 8, pos.y()/mm);
  am->FillNtupleDColumn(0, 9, pos.z()/mm);
  am->FillNtupleDColumn(0, 10, dir.x());
  am->FillNtupleDColumn(0, 11, dir.y());
  am->FillNtupleDColumn(0, 12, dir.z());
  am->FillNtupleDColumn(0, 13, t/ns);
  am->FillNtupleIColumn(0, 14, trk);
  am->FillNtupleIColumn(0, 15, parent);
  am->AddNtupleRow(0);
}

}  // namespace MuoTables
