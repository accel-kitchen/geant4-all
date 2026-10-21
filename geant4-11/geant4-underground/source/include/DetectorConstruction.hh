//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DetectorConstruction.hh
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"

class G4LogicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction() = default;
  ~DetectorConstruction() override = default;

  G4VPhysicalVolume* Construct() override;
  void ConstructSDandField() override;

private:
  void BuildGround(G4LogicalVolume* world);
  void BuildDetector(G4LogicalVolume* world);
  void BuildPlane(G4LogicalVolume* world);

  G4LogicalVolume* fLogScint = nullptr;
  G4LogicalVolume* fLogPlane = nullptr;
};

#endif
