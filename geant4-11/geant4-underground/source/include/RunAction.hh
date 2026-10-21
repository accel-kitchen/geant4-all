//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RunAction.hh
// 出力ファイルの開閉 (スレッドごと)、マスターでの集計と meta.json の書き出し。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include <chrono>

class RunAction : public G4UserRunAction {
public:
  RunAction();
  ~RunAction() override = default;

  G4Run* GenerateRun() override;
  void BeginOfRunAction(const G4Run*) override;
  void EndOfRunAction(const G4Run*) override;

private:
  std::chrono::steady_clock::time_point fStart;
};

#endif
