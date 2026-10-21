//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuoRun.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "MuoRun.hh"

//------------------------------------------------------------------------------
  void MuoRun::Merge(const G4Run* run)
//------------------------------------------------------------------------------
{
  const auto* other = static_cast<const MuoRun*>(run);
  sumGenW    += other->sumGenW;
  sumGenW2   += other->sumGenW2;
  nCoinc     += other->nCoinc;
  sumCoincW  += other->sumCoincW;
  sumCoincW2 += other->sumCoincW2;
  nPlaneRows += other->nPlaneRows;
  G4Run::Merge(run);
}
