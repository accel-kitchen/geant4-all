
#include "RunAction.hh"
#include "Analysis.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"



RunAction::RunAction()
 : G4UserRunAction()
{
    // Get analysis manager
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    G4cout << "Using " << analysisManager->GetType() << G4endl;
    analysisManager->SetActivation(true);

    // Create directories
    //analysisManager->SetHistoDirectoryName("histograms");
    //analysisManager->SetNtupleDirectoryName("ntuple");
    analysisManager->SetVerboseLevel(1);
    //analysisManager->SetFirstHistoId(1);
    analysisManager->SetFileName("Water");
    //analysisManager->SetHistoDirectoryName("histo");
    // Book histograms, ntuple
    //
    
    // Creating histograms
//H1
    G4int id = analysisManager->CreateH1("h1:0","copyNo", 200, 0, 200, "none");
	//analysisManager->SetH1Activation(id, false);
     id = analysisManager->CreateH1("h1:1","edep", 20, 0, 10, "none");
	//analysisManager->SetH1Activation(id, false);
   //  id = analysisManager->CreateH1("h1:2","sum_eDep", 100, 0., 5.*MeV);
//	analysisManager->SetH1Activation(id, false);
// H2
     G4int id2 = analysisManager->CreateH2("h2:0","copyNo vs edep", 200, 0, 200, 200,  0, 10);
//	analysisManager->SetH2Activation(id2, false);
     G4int id3 = analysisManager->CreateH2("h2:1","Front x vs y", 100, 0,100, 100, 0,100);
     G4int id4 = analysisManager->CreateH2("h2:2","Back x vs y", 100, 0,100, 100, 0,100);
    // Creating ntuple
    //
    analysisManager->CreateNtuple("WaterSOBP", "EnergyLoss");
    analysisManager->CreateNtupleFColumn("copyNo");
    analysisManager->CreateNtupleFColumn("edp");
    analysisManager->FinishNtuple();

 }



RunAction::~RunAction()
{
  delete G4AnalysisManager::Instance();  
}



void RunAction::BeginOfRunAction(const G4Run* /*run*/)
{ 
    
  // Get analysis manager
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  // Open an output file
  //
  // G4String fileName = "Pixel";
  analysisManager->OpenFile();
    //G4cout << "File " << fileName << " Open" <<G4endl;
    
}



void RunAction::EndOfRunAction(const G4Run* /*run*/)
{
  // print histogram statistics
  //
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  // save histograms & ntuple
  //
  analysisManager->Write();
  analysisManager->CloseFile();


}
