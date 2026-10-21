//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PhaseSpaceStore.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "PhaseSpaceStore.hh"
#include "MuoConfig.hh"
#include "G4Exception.hh"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

namespace fs = std::filesystem;

//------------------------------------------------------------------------------
  const PhaseSpaceStore& PhaseSpaceStore::Get()
//------------------------------------------------------------------------------
{
  static const PhaseSpaceStore store;
  return store;
}

//------------------------------------------------------------------------------
  PhaseSpaceStore::PhaseSpaceStore()
//------------------------------------------------------------------------------
{
  const fs::path prefix(std::string(MuoConfig::Get().phaseSpacePrefix));
  const fs::path dir = prefix.has_parent_path() ? prefix.parent_path() : fs::path(".");
  const std::string stem = prefix.filename().string() + "_nt_phasespace";

  std::vector<fs::path> files;
  if (fs::exists(dir)) {
    for (const auto& e : fs::directory_iterator(dir)) {
      const std::string n = e.path().filename().string();
      if (n.rfind(stem, 0) == 0 && e.path().extension() == ".csv") files.push_back(e.path());
    }
  }
  if (files.empty()) {
    G4Exception("PhaseSpaceStore", "Muo010", FatalException,
                ("位相空間ファイルが見つかりません: " + prefix.string() + "_nt_phasespace*.csv").c_str());
  }
  std::sort(files.begin(), files.end());

  std::map<G4int, PsGroup> byEvent;
  for (const auto& f : files) {
    std::ifstream in(f);
    std::map<std::string, size_t> col;
    std::string line;
    while (std::getline(in, line)) {
      if (line.empty()) continue;
      if (line[0] == '#') {
        // "#column <type> <name>"
        std::istringstream h(line);
        std::string tag, type, name;
        h >> tag >> type >> name;
        if (tag == "#column") col[name] = col.size();
        continue;
      }
      std::vector<double> v;
      std::istringstream is(line);
      std::string tok;
      while (std::getline(is, tok, ',')) v.push_back(std::stod(tok));
      if (v.size() < col.size()) continue;
      auto at = [&](const char* k) { return v[col.at(k)]; };

      const G4int ev = static_cast<G4int>(at("event"));
      auto& g = byEvent[ev];
      if (g.rows.empty()) {
        g.srcEvent = ev; g.w1 = at("w1"); g.E0 = at("E0"); g.cth0 = at("cth0"); g.ph0 = at("ph0");
      }
      g.rows.push_back({static_cast<G4int>(at("pdg")), at("ekin"), at("x"), at("y"), at("z"),
                        at("ux"), at("uy"), at("uz"), at("t")});
    }
  }
  fGroups.reserve(byEvent.size());
  for (auto& kv : byEvent) fGroups.push_back(std::move(kv.second));

  // Stage A のイベント数 (レート換算の分母) を meta.json から読む
  std::ifstream meta(prefix.string() + "_meta.json");
  std::string text((std::istreambuf_iterator<char>(meta)), std::istreambuf_iterator<char>());
  const auto p = text.find("\"n_generated\":");
  if (p == std::string::npos) {
    G4Exception("PhaseSpaceStore", "Muo011", FatalException,
                ("meta.json に n_generated がありません: " + prefix.string() + "_meta.json").c_str());
  }
  fSrcGenerated = std::stol(text.substr(p + 14));
}
