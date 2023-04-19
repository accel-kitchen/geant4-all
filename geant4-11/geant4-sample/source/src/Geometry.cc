//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Geometry.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "Geometry.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4NistManager.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include "SensitiveVolume.hh"
#include "G4SDManager.hh"
//------------------------------------------------------------------------------
// block class 
class Block{
  public:
    Block(G4String name, G4double pos_X, G4double pos_Y, G4double pos_Z, G4double size_X, G4double size_Y, G4double size_Z, G4int copyNum, G4int nDiv_z, G4String materi, G4PVPlacement*& physVol_World);
};
Block::Block(G4String name, G4double pos_X, G4double pos_Y, G4double pos_Z, G4double size_X, G4double size_Y, G4double size_Z, G4int copyNum, G4int nDiv_z, G4String materi, G4PVPlacement*& physVol_World)
{
    auto materi_Man = G4NistManager::Instance();
       auto materi_BlockEnvG = materi_Man->FindOrBuildMaterial(materi);
   auto solid_BlockEnvG = new G4Box("Solid_BlockEnvG", size_X/2.0, size_Y/2.0, size_Z/2.0);

   auto logVol_BlockEnvG = new G4LogicalVolume(solid_BlockEnvG, materi_BlockEnvG, "LogVol_BlockEnvG");
   auto threeVect_LogV_BlockEnvG = G4ThreeVector(pos_X, pos_Y, pos_Z);
   auto rotMtrx_LogV_BlockEnvG = G4RotationMatrix();
   auto trans3D_LogV_BlockEnvG = G4Transform3D(rotMtrx_LogV_BlockEnvG, threeVect_LogV_BlockEnvG);
   new G4PVPlacement(trans3D_LogV_BlockEnvG, "PhysVol_BlockEnvG", logVol_BlockEnvG, physVol_World, 
                     false, copyNum);
    /////// Sensitive Detector
    auto BlockSV = new SensitiveVolume("SensitiveVolume");
    logVol_BlockEnvG->SetSensitiveDetector(BlockSV);         // Add sensitivity to the logical volume
    auto SDmanBlock = G4SDManager::GetSDMpointer();
    SDmanBlock->AddNewDetector(BlockSV);
}
//------------------------------------------------------------------------------
  Geometry::Geometry() {}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
  Geometry::~Geometry() {}
//------------------------------------------------------------------------------
/////// 実装必須のConstruct()
// 最後に物理ワールドへのポインタを返す
//------------------------------------------------------------------------------
  G4VPhysicalVolume* Geometry::Construct()
//------------------------------------------------------------------------------
{
  /////// Generate world
   auto materi_Man = G4NistManager::Instance();//Get pointer "material manager"
   auto solid_World = new G4Box("Solid_World", 1.0*m/2.0, 1.0*m/2.0, 1.0*m/2.0); //define world volume
   auto materi_World = materi_Man->FindOrBuildMaterial("G4_AIR");
   auto logVol_World = new G4LogicalVolume(solid_World, materi_World, "LogVol_World");
   logVol_World->SetVisAttributes ( G4VisAttributes::GetInvisible() );
   auto physVol_World  =  new G4PVPlacement(G4Transform3D(), "PhysVol_World", logVol_World, 0, false, 0);


  /////// put volumess in the world

//arguments are follows:
// name, pos_X, pos_Y, pos_Z, size_X, size_Y, size_Z, copyNum, nDiv_z, material ,physVol_World
Block Block1("Block1", 0.0 * cm, 0.0 * cm, 20 * cm, 20* mm, 20 * mm, 100 * mm, 1 ,1000, "G4_WATER" ,physVol_World); 



/////// Return the physical world
   return physVol_World;
}