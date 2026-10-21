//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Application_Main.cc
// 使い方: Application_Main <macro.mac> [-t スレッド数] [--em4] [--vis]
//   マクロの中で /muo/ コマンドを実行し、/run/initialize の後に /run/beamOn する。
//   既定はバッチ実行 (可視化なし)。--vis は -DWITH_GEANT4_UIVIS=ON でビルドしたときだけ使える。
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "MuoConfig.hh"
#include "MuoMessenger.hh"

#include "G4EmStandardPhysics_option4.hh"
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "FTFP_BERT.hh"
#include "Randomize.hh"

#ifdef MUO_WITH_UIVIS
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#endif

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

//------------------------------------------------------------------------------
  int main( int argc, char** argv )
//------------------------------------------------------------------------------
{
  std::string macro;
  int threads = 0;
  bool em4 = false, vis = false;
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "-t" && i + 1 < argc) threads = std::atoi(argv[++i]);
    else if (a == "--em4") em4 = true;
    else if (a == "--vis") vis = true;
    else macro = a;
  }
  if (macro.empty() && !vis) {
    std::cerr << "usage: Application_Main <macro.mac> [-t N] [--em4] [--vis]" << std::endl;
    return 1;
  }

  auto& cfg = MuoConfig::Get();
  cfg.macroName = macro;
  G4Random::setTheSeed(cfg.seed);

  auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Tasking);
  auto* messenger = new MuoMessenger;

  runManager->SetUserInitialization(new DetectorConstruction);
  auto* physics = new FTFP_BERT(0);
  if (em4) physics->ReplacePhysics(new G4EmStandardPhysics_option4(0));
  runManager->SetUserInitialization(physics);
  runManager->SetUserInitialization(new ActionInitialization);

  auto* ui = G4UImanager::GetUIpointer();
  if (threads > 0) ui->ApplyCommand("/run/numberOfThreads " + std::to_string(threads));

#ifdef MUO_WITH_UIVIS
  G4VisManager* visManager = nullptr;
  if (vis) {
    visManager = new G4VisExecutive("Quiet");
    visManager->Initialize();
    auto* exec = new G4UIExecutive(argc, argv);
    if (!macro.empty()) ui->ApplyCommand("/control/execute " + macro);
    exec->SessionStart();
    delete exec;
    delete visManager;
    delete messenger;
    delete runManager;
    return 0;
  }
#else
  if (vis) {
    std::cerr << "--vis は -DWITH_GEANT4_UIVIS=ON でビルドしたときだけ使えます" << std::endl;
    return 1;
  }
#endif

  ui->ApplyCommand("/control/execute " + macro);

  delete messenger;
  delete runManager;
  return 0;
}
