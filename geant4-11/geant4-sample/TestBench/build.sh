#!/bin/bash
# Geant4プロジェクトの自動ビルドと実行スクリプト

echo "=== Geant4プロジェクトビルド開始 ==="

# buildディレクトリに移動してビルド実行
cd ../build
echo "CMake設定中..."
cmake ../source
echo "ビルド実行中..."
make
echo "インストール中..."
make install

# TestBenchディレクトリに戻る
cd ../TestBench

echo "=== ビルド完了 ==="
echo "=== 実行モードを選択してください ==="
echo "1. 高速版（10000発、可視化無効）"
echo "2. GUI版（10発、ビジュアライゼーション有効）"
echo ""
read -p "選択してください [1 or 2]: " choice

case $choice in
    1)
        echo "=== 高速版を実行中... ==="
        echo "天頂角度分布の統計を収集します（可視化無効）"
        echo "1000発ごとに進捗が表示されます"
        ../build/Application_Main sphericalDistributionTest.mac
        ;;
    2)
        echo "=== 詳細表示版を実行中... ==="
        echo "天頂角度分布の詳細情報とVRMLファイルを出力します"
        echo "10発のミュオンの軌跡と検出器ヒットを詳細表示"
        echo "muography_visualization.wrlファイルが作成されます"
        ../build/Application_Main sphericalDistributionGUI.mac
        ;;
    *)
        echo "無効な選択です。1または2を選択してください。"
        exit 1
        ;;
esac
