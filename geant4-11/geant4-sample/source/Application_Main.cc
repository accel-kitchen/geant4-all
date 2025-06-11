//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Geant4 Application: Tutorial course for Hep/Medicine Users
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "Geometry.hh"
#include "UserActionInitialization.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "FTFP_BERT.hh"

//-------------------------------------------------------------------------------
  int main( int argc, char** argv )
//-------------------------------------------------------------------------------
{
// Construct the default run manager
   auto runManager = new G4RunManager;

// Set up mandatory user initialization: Geometry
   runManager->SetUserInitialization( new Geometry{} );

// Set up mandatory user initialization: Physics-List
   runManager->SetUserInitialization( new FTFP_BERT{} );

// Set up user initialization: User Actions
   runManager->SetUserInitialization( new UserActionInitialization{} );

// Initialize G4 kernel
   runManager->Initialize();

// Create visualization environment
   auto visManager = new G4VisExecutive{};
   visManager->Initialize();

// Start interactive session
   G4UImanager*  uiManager = G4UImanager::GetUIpointer();
   
   // コマンドライン引数でmacファイルが指定されている場合
   if ( argc > 1 ) {
     // バッチモードかGUIモードかをファイル名で判断
     G4String macFile = argv[1];
     if ( macFile.find("batch") != std::string::npos || 
          macFile.find("test") != std::string::npos ) {
       // バッチモード実行
       G4String command = "/control/execute ";
       command += argv[1];
       uiManager->ApplyCommand( command );
     } else {
       // GUIモードで実行
       auto uiExec = new G4UIExecutive( argc, argv );
       G4String command = "/control/execute ";
       command += argv[1];
       uiManager->ApplyCommand( command );
       uiExec->SessionStart();
       delete uiExec;
     }
   } else {
     // デフォルトでGUIモード
     auto uiExec = new G4UIExecutive( argc, argv );
     uiManager->ApplyCommand( "/control/execute TestBench/GlobalSetup.mac" );
     uiExec->SessionStart();
     delete uiExec;
   }

// Job termination
   delete visManager;
   delete runManager;

   return 0;
}
