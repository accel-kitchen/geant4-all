//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RunAction.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "RunAction.hh"
#include "MuoConfig.hh"
#include "MuoRun.hh"
#include "MuoTables.hh"
#include "PhaseSpaceStore.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4Threading.hh"
#include "G4Version.hh"
#include "Randomize.hh"

#include <cmath>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace {
  std::string GitHash()
  {
    std::string out;
    if (FILE* p = popen("git rev-parse --short HEAD 2>/dev/null", "r")) {
      char buf[64];
      if (std::fgets(buf, sizeof(buf), p)) out = buf;
      pclose(p);
    }
    while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
    return out.empty() ? "unknown" : out;
  }

  std::string Now()
  {
    char buf[32];
    const std::time_t t = std::time(nullptr);
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&t));
    return buf;
  }
}

//------------------------------------------------------------------------------
  RunAction::RunAction()
  : G4UserRunAction()
//------------------------------------------------------------------------------
{
  auto* am = G4AnalysisManager::Instance();
  am->SetDefaultFileType("csv");
  am->SetVerboseLevel(0);
  MuoTables::Create(am, MuoConfig::Get().stage == MuoStage::A ? "phasespace" : "hits");
}

//------------------------------------------------------------------------------
  G4Run* RunAction::GenerateRun()
//------------------------------------------------------------------------------
{
  return new MuoRun;
}

//------------------------------------------------------------------------------
  void RunAction::BeginOfRunAction(const G4Run* run)
//------------------------------------------------------------------------------
{
  const auto& c = MuoConfig::Get();
  std::filesystem::create_directories(c.outDir.c_str());
  G4AnalysisManager::Instance()->OpenFile(c.outDir + "/" + c.prefix);

  if (IsMaster()) {
    fStart = std::chrono::steady_clock::now();
    if (c.stage == MuoStage::B) {
      const auto nGroups = PhaseSpaceStore::Get().Groups().size();
      if (static_cast<size_t>(run->GetNumberOfEventToBeProcessed()) != nGroups) {
        G4cout << "### 警告: Stage B の beamOn は位相空間のイベント数 " << nGroups
               << " に合わせてください (今回 " << run->GetNumberOfEventToBeProcessed() << ")" << G4endl;
      }
    }
  }
}

//------------------------------------------------------------------------------
  void RunAction::EndOfRunAction(const G4Run* run)
//------------------------------------------------------------------------------
{
  auto* am = G4AnalysisManager::Instance();
  am->Write();
  am->CloseFile();
  if (!IsMaster()) return;

  const auto& c = MuoConfig::Get();
  const auto* r = static_cast<const MuoRun*>(run);
  const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - fStart).count();

  const long nEvents = run->GetNumberOfEvent();
  const long nNorm = (c.stage == MuoStage::B) ? PhaseSpaceStore::Get().SrcGenerated() : nEvents;
  const double rate = nNorm > 0 ? r->sumCoincW/nNorm : 0.0;
  const double rateErr = nNorm > 0 ? std::sqrt(r->sumCoincW2)/nNorm : 0.0;
  const double genRate = nNorm > 0 ? r->sumGenW/nNorm : 0.0;

  G4cout << "\n=== Muo run summary ===\n"
         << " stage            : " << c.StageName() << "\n"
         << " events (run)    : " << nEvents << "  (normalization N = " << nNorm << ")\n"
         << " plane rows       : " << r->nPlaneRows << "\n"
         << " coincidences     : " << r->nCoinc << "\n"
         << " coincidence rate : " << rate << " +- " << rateErr << " Hz (weighted, MC error)\n"
         << " source rate      : " << genRate << " Hz (target disk x cone x energy range)\n"
         << " wall time        : " << elapsed << " s\n"
         << "=======================" << G4endl;

  std::ofstream meta(c.outDir + "/" + c.prefix + "_meta.json");
  meta << std::setprecision(10)
       << "{\n"
       << "  \"stage\": \"" << c.StageName() << "\",\n"
       << "  \"n_generated\": " << nNorm << ",\n"
       << "  \"n_events_run\": " << nEvents << ",\n"
       << "  \"n_coincidences\": " << r->nCoinc << ",\n"
       << "  \"n_plane_rows\": " << r->nPlaneRows << ",\n"
       << "  \"sum_w1_generated\": " << r->sumGenW << ",\n"
       << "  \"sum_w1_coincidence\": " << r->sumCoincW << ",\n"
       << "  \"sum_w1sq_coincidence\": " << r->sumCoincW2 << ",\n"
       << "  \"rate_hz\": " << rate << ",\n"
       << "  \"rate_err_hz\": " << rateErr << ",\n"
       << "  \"elapsed_s\": " << elapsed << ",\n"
       << "  \"threads\": " << G4Threading::GetNumberOfRunningWorkerThreads() << ",\n"
       << "  \"seed\": " << c.seed << ",\n"
       << "  \"macro\": \"" << c.macroName << "\",\n"
       << "  \"git\": \"" << GitHash() << "\",\n"
       << "  \"geant4_version_number\": " << G4VERSION_NUMBER << ",\n"
       << "  \"date\": \"" << Now() << "\",\n"
       << c.ToJson() << "\n"
       << "}\n";
}
