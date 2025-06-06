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
#include "G4SubtractionSolid.hh"
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
   auto solid_World = new G4Box("Solid_World", 10.0*m, 10.0*m, 10.0*m); // 20m×20m×20mのワールド
   auto materi_World = materi_Man->FindOrBuildMaterial("G4_AIR");
   auto logVol_World = new G4LogicalVolume(solid_World, materi_World, "LogVol_World");
   logVol_World->SetVisAttributes ( G4VisAttributes::GetInvisible() );
   auto physVol_World  =  new G4PVPlacement(G4Transform3D(), "PhysVol_World", logVol_World, 0, false, 0);


  /////// 土のブロック (Z=1mから11m、2種類の密度)
   
   // 左側の土材料（X<0、密度1.4g/cm³）
   auto materi_SoilLeft = new G4Material("SoilLeft", 1.4*g/cm3, 1);
   materi_SoilLeft->AddMaterial(materi_Man->FindOrBuildMaterial("G4_CONCRETE"), 1.0);
   
   // 右側の土材料（X>0、密度1.8g/cm³）
   auto materi_SoilRight = new G4Material("SoilRight", 1.8*g/cm3, 1);
   materi_SoilRight->AddMaterial(materi_Man->FindOrBuildMaterial("G4_CONCRETE"), 1.0);
   
   // 左側土ブロックの形状を作成（X<0側、X=-4mからX=0まで）
   auto solid_SoilBlockLeft = new G4Box("Solid_SoilBlockLeft", 8.0*m, 16.0*m, 5.0*m); // 16m×32m×10m
   
   // 右側土ブロックの形状を作成（X>0側、X=0からX=4mまで）
   auto solid_SoilBlockRight = new G4Box("Solid_SoilBlockRight", 8.0*m, 16.0*m, 5.0*m); // 16m×32m×10m
   
   // 左側土ブロックの論理ボリュームを作成
   auto logVol_SoilBlockLeft = new G4LogicalVolume(solid_SoilBlockLeft, materi_SoilLeft, "LogVol_SoilBlockLeft");
   
   // 右側土ブロックの論理ボリュームを作成
   auto logVol_SoilBlockRight = new G4LogicalVolume(solid_SoilBlockRight, materi_SoilRight, "LogVol_SoilBlockRight");
   
   // 土ブロックの可視化設定
   auto visAttr_SoilLeft = new G4VisAttributes(G4Colour(0.6, 0.4, 0.2, 0.5)); // 茶色、半透明
   auto visAttr_SoilRight = new G4VisAttributes(G4Colour(0.7, 0.5, 0.8, 0.5)); // 少し明るい茶色、半透明
   logVol_SoilBlockLeft->SetVisAttributes(visAttr_SoilLeft);
   logVol_SoilBlockRight->SetVisAttributes(visAttr_SoilRight);
   
   // Z=6mの位置に配置 (1mから11mをカバー)
   // 左側土ブロック（X<0側、中心をX=-8mに配置）
   auto threeVect_SoilBlockLeft = G4ThreeVector(-8.0*m, 0, 6.0*m);
   new G4PVPlacement(0, threeVect_SoilBlockLeft, "PhysVol_SoilBlockLeft", logVol_SoilBlockLeft, physVol_World, false, 0);
   
   // 右側土ブロック（X>0側、中心をX=8mに配置）
   auto threeVect_SoilBlockRight = G4ThreeVector(8.0*m, 0, 6.0*m);
   new G4PVPlacement(0, threeVect_SoilBlockRight, "PhysVol_SoilBlockRight", logVol_SoilBlockRight, physVol_World, false, 1);

  /////// 検出器アレイ (コインシデンス測定用上下2段)
   // プラスチックシンチレータ材料
   auto materi_Detector = materi_Man->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
   auto solid_Detector = new G4Box("Solid_Detector", 5.0*cm, 5.0*cm, 0.5*cm); // 10cm×10cm×1cm
   auto logVol_Detector = new G4LogicalVolume(solid_Detector, materi_Detector, "LogVol_Detector");
   
   // 検出器の可視化設定
   auto visAttr_Detector = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 0.8)); // 緑色
   logVol_Detector->SetVisAttributes(visAttr_Detector);
   
   // 検出器に感度を設定
   auto DetectorSV = new SensitiveVolume("DetectorSV");
   logVol_Detector->SetSensitiveDetector(DetectorSV);
   auto SDmanDetector = G4SDManager::GetSDMpointer();
   SDmanDetector->AddNewDetector(DetectorSV);
   
   // X=-3mから3mまで10cm間隔で上下2段に配置
   // CopyNo と検出器位置の対応関係:
   // 下段 (Z=0):
   // CopyNo  0: X = -3.0m, Z = 0
   // CopyNo  1: X = -2.9m, Z = 0
   // ...
   // CopyNo 60: X =  3.0m, Z = 0
   // 上段 (Z=10cm):
   // CopyNo 61: X = -3.0m, Z = 10cm
   // CopyNo 62: X = -2.9m, Z = 10cm
   // ...
   // CopyNo 121: X = 3.0m, Z = 10cm
   
   G4int detectorCount = 0;
   
   // 下段の検出器 (Z=0)
   for (G4double x = -3.0*m; x <= 3.0*m; x += 0.1*m) {
     auto threeVect_Detector = G4ThreeVector(x, 0, 0);
     new G4PVPlacement(0, threeVect_Detector, "PhysVol_Detector_Lower", logVol_Detector, 
                       physVol_World, false, detectorCount);
     detectorCount++;
   }
   
   // 上段の検出器 (Z=10cm)
   for (G4double x = -3.0*m; x <= 3.0*m; x += 0.1*m) {
     auto threeVect_Detector = G4ThreeVector(x, 0, 10.0*cm);
     new G4PVPlacement(0, threeVect_Detector, "PhysVol_Detector_Upper", logVol_Detector, 
                       physVol_World, false, detectorCount);
     detectorCount++;
   }

/////// Return the physical world
   return physVol_World;
}
