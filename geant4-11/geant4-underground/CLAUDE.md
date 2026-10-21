# CLAUDE.md（引き継ぎ指示書）

このディレクトリ `geant4-11/geant4-underground/` で作業する Claude Code 向けの指示書です。
最初に **この文書 → `Plan.html`（ブラウザで開く）** の順に読んでください。判断の理由は Plan.html にあります。

## 1. 何をしているか

創発的研究支援事業の研究計画書「自律移動型ミュオグラフィによる地下密度分布マッピング技術の創出」
（研究代表者: 田中 香津生）の検出器計画を、Geant4 で検証するコードを作っています。

- 検出器: 60 cm 角・シンチレータピクセル 2 cm ピッチ・2 層。下水道管内のドローンに載せ、真上の地盤を見る。
- 目的: 計画書の数値目標（Plan.html §1 の T1〜T9）をシミュレーションで確かめる。
- 計画書の docx（`創発_研究計画書本文_…docx`）は**Git に入れていません**（公開リポジトリの可能性があるため）。
  必要ならこの PC のこのディレクトリにあります。内容は Plan.html に要約済みです。
- ブランチは `2d-array`。リモートは `https://github.com/accel-kitchen/geant4-all`。

## 2. 決定済みの方針（Plan.html §12 の回答）

| 項目 | 決定 |
|---|---|
| Q1 土被り | 3〜10 m を基準にし、20 m・50 m は感度確認として掃引 |
| Q2 定義 | 円錐角は半角（天頂角 50° まで）、角度分解能 3° は σ |
| Q3 「分離」の基準 | 3σ を検出、5σ を確定。ROC（誤警報率 1%）で検出確率を併記 |
| Q4 土台 | `geant4-muography-underground` を土台に、このディレクトリへ移植して作り直す（済） |
| Q5 出力 | CSV（`G4AnalysisManager`）。容量が問題になったら再検討 |
| Q6 土壌 | かさ密度・含水率から作る簡易モデル（SiO₂ + 水） |
| Q7 着手順 | WP0・WP1（コード基盤と SC-01）を先に作り、その後 WP2・WP3 を並行 |

設計判断 D1〜D9（二段構成、重み付き発生、MT など）の詳細は Plan.html §5。

## 3. 現在の状態（2026-10-21 時点）

| 項目 | 状態 |
|---|---|
| ビルド | 通る（Geant4 11.3.2、WSL2） |
| 追跡と実行 | 動く。`v3_air.mac` が 40 万イベント・20 スレッドで 13 秒 |
| ヒットの集計（`ScintSD`） | **不具合あり。同時計数が 0 件になる（§6）** |
| Stage A / Stage B / 一段の動作 | **未確認**（`phasespace` CSV の列名・ファイル名の確認も未実施） |
| 検証 V1〜V5 | 未実施 |
| `analysis/` | `feasibility.py`（概算）のみ。`muo_io.py` と `summarize.py` は未作成 |
| 命令 `/muo/…` | 実装済み。実際に使ったのは v3_air.mac の範囲だけ |

## 4. ビルドと実行

Geant4 の環境を読み込んでから行います（`~/.bashrc` は対話シェルだけで読むので、スクリプト内では明示する）。

```bash
source ~/geant4-install/bin/geant4.sh
cd /home/kaduo/geant4-all/geant4-11/geant4-underground
./TestBench/build.sh          # 初回は clean を付ける: ./TestBench/build.sh clean
```

実行は `TestBench/` から行います（出力が `TestBench/runs/` に出る）。

```bash
cd TestBench
../bin/Application_Main v3_air.mac            # マクロは /run/initialize の前に /muo/… を書く
../bin/Application_Main v3_air.mac -t 8       # スレッド数を CLI で指定 (--em4 で EM を option4 に)
```

- 既定はバッチ実行で、可視化は付きません（WSL2 の Qt ハング回避）。可視化が要るときだけ
  `./TestBench/build.sh uivis` でビルドし、`--vis` で起動します。
- 環境: Geant4 11.3.2（MT 有効、hdf5 無効）。Python は numpy 1.x（`np.trapz` を使う）、pandas 2.1.4、
  matplotlib 3.10.3。scipy、ROOT、uproot、h5py は**ありません**。
- **前の PC は負荷が高くて落ちました。** スレッド数は 8 前後から始めてください。
  `/tracking/verbose` は 1〜2 イベントだけ、出力は必ずファイルに落とし、長い実行は小さい試走を済ませてから。

## 5. コードの構成

```
source/
  Application_Main.cc        引数 (-t, --em4, --vis)、物理リスト FTFP_BERT、G4TaskRunManager
  include/ src/
    MuoConfig        実行時設定 (シングルトン)。派生量 (記録面の高さ、土の幅など) もここ
    MuoMessenger     /muo/ コマンド。値は MuoConfig に入る
    DetectorConstruction  地盤・管路・検出器・記録面。stage で中身が変わる
    SoilModel        SiO2 + 水の混合材料
    PrimaryGenerator 重み付きの方向指定発生 / Stage B の再生
    MuonFlux.hh      Guan 2015 の修正 Gaisser 式 (analysis/feasibility.py と同じ)
    ScintSD          ピクセルのエネルギー付与 → しきい値 → 同時計数 → hits 表
    PlaneSD          Stage A の記録面。下向きの粒子を phasespace 表に書いて止める
    PhaseSpaceStore  Stage B が読む phasespace CSV (event ごとにまとめる)
    MuoTables        表の列定義 (作成と書き込みの列順をここに集約)
    MuoRun, RunAction, ActionInitialization, MuoEventInfo
TestBench/  build.sh, v3_air.mac        analysis/  feasibility.py
```

### 約束事

- 座標は z 上向き。原点は 2 層の中点。管の軸は X 方向。単位は Geant4 内部 (mm, MeV, ns)。
- CopyNo = `layer*10000 + ix*100 + iy`。layer 1 が下段 (z = −gap/2)、layer 2 が上段 (+gap/2)。nx, ny は 99 まで。
- 土被り `/muo/soil/overburden` は「検出器の基準点から地表までの高さ」。管路なしのときは検出器の上の
  空気 `/muo/soil/airGap`（既定 0.5 m）を含む。
- **重み `w1` は 1 イベントあたりのレート [Hz]**。測定レート = Σ w1 / N_generated。
  N は meta.json の `n_generated`。Stage B では Stage A のイベント数が入る。
- 表は `<outDir>/<prefix>_nt_<table>_t<スレッド番号>.csv` の想定（**未確認**）。ヘッダ行は `#` で始まる。
  列は `MuoTables.cc` を参照。`E0` だけ GeV。
- Stage B の `beamOn` は phasespace のイベント数に合わせる（合わないと警告が出る）。
- 隠しスイッチは使わない（`BG_UNIFORM` のような環境変数は廃止）。条件はマクロに書く。

## 6. 未解決の不具合: 同時計数が 0 件

`TestBench/v3_air.mac`（土なし・60 cm 角・層間 30 cm）で、`coincidences : 0` になります。
解析式の期待は約 28 Hz（§7 の V3）です。実行ログの線源側レートは 485 Hz なので、
40 万イベントなら約 6%、つまり約 2 万件の同時計数があるはずです。

分かっていること:
- ミュオンは両方の層のピクセルに入り、`muIoni` でエネルギーを落とす（`/tracking/verbose 1` で確認）。
- `ScintSD::ProcessHits` は呼ばれ、`edep > 0` で `copy=20321` などが取れる（一時的な出力で確認）。
- 一方、`EndOfEvent` 側の一時出力（`fHits.size() > 0` のとき表示）は**一度も出なかった**。
  つまり EndOfEvent が呼ばれていないか、呼ばれたときに `fHits` が空。
- 出力 CSV は 1 つもできず、`runs/v3_air_meta.json` だけができた。

次の順で調べてください（軽い順）。

1. 1 スレッド・10 イベントで、`ScintSD::Initialize` と `EndOfEvent` の**先頭に無条件の出力**を置く。
   `Initialize` も呼ばれなければ、SD マネージャとイベントマネージャの組み合わせの問題。
   `Application_Main` の先頭（`CreateRunManager` の前）で `G4SDManager::GetSDMpointer();` を呼ぶと直る可能性がある。
2. `ProcessHits` を呼ぶ SD のインスタンスと、`EndOfEvent` が呼ばれるインスタンスが別の可能性
   （`ConstructSDandField` がマスターとワーカーで `new ScintSD` している）。ポインタをログに出して比べる。
3. 上で直らなければ、SD に頼らず `G4UserSteppingAction` か `G4UserEventAction` でピクセルごとのエネルギー付与を集める。
   ヒット集計の書き方が変わるだけで、表の形式 (`MuoTables`) は変えなくてよい。
4. 直したら、デバッグ用の出力は残さない。

## 7. 次の作業（この順で）

- [ ] §6 の不具合を直す。
- [ ] **V3**: `v3_air.mac` の結果を `analysis/feasibility.py` の解析式と比べる。同じ E 範囲（0.15〜500 GeV）で
      積分範囲を合わせ、数% 以内で一致すれば合格。`rate(0.6, 0.3, 0)` は 28.46 Hz（E ≥ 0.1 GeV）。
- [ ] `analysis/muo_io.py`（CSV と meta の読み込み）と `analysis/summarize.py`（レート、角度残差 σθ）を書く。
      pandas の `read_csv(comment='#')`、列名は `#column <型> <名前>` 行から取る。
- [ ] **V4**: `targetRadius`、`eLow`、`zenithMax` を広げても結果が統計誤差内で変わらないことを確認。
- [ ] **V5**: 浅い設定（土被り 3 m など）で、一段 (`full`) と Stage A→B を比べる。
      `sc01_stageA.mac`、`sc01_stageB.mac` を作る。Stage A の `beamOn` 数と、B の `beamOn` 数（phasespace のグループ数）に注意。
- [ ] **V1**: ミュオンの飛程を標準岩石の表（Groom 2001）と比べる。`--em4` との差も見る。
- [ ] **SC-01**（Plan.html §6）: 土被り 10 m、砂 1.4 / 粘土 1.8 の鉛直境界、密度差 0.2〜0.4 を 10⁷ イベントで。
      MT での実行時間を記録し、計画書の予測との比較を 1 ページのレポートにする。
- [ ] `/muo/…` の未使用コマンド（`addLayer`、`cavity`、`pipe/*`、`housing`）を、小さな試走で一度ずつ動かす。
      管路の幾何は `/geometry/test/run` でオーバーラップを確認する。
- [ ] その後 WP2・WP3（検出器応答 L1、光学 L2）へ（Plan.html §10）。

## 8. 進め方の注意

- Git: ステージは**パスを指定**して行う（`git add -A` は使わない）。`geant4-11/geant4-sample/` の変更は別作業で、
  このリポジトリには他のプロジェクトも入っている。コミットメッセージは日本語、末尾に Co-Authored-By を付ける。
- push には GitHub の認証が要る。パスワードは使えない。`gh auth login`（HTTPS、Authenticate Git: Yes）を済ませる。
  最初のコミット (`1ab7819`: 新コードと Plan.html) は push 済み。この文書と `analysis/` は、その後に追加した。
  push 済みかは `git status -sb` の `ahead` で確認できる。
- 計画書 docx、トークン、個人情報はコミットしない。
- 結果の数値を Plan.html に反映するときは、`Plan.html` を直接編集する（チャートは静的な SVG）。
  Artifact 版（https://claude.ai/artifact/7QtGaqZ6DPtPhW6kGYRCAq、非公開）は公開時点のコピーで、自動では更新されない。
- 概算は ±30% 程度（Plan.html §2、付録）。MC の結果が概算と 3 割以上ずれたら、まず MC 側のバグを疑い、
  次に概算の前提（検出効率 100%、一様な ρ、標準岩石の a, b）を疑う。
- 既存の `geant4-muography-underground/CLAUDE.md` に、WSL2 の Qt ハングなどの既知の癖がまとまっている。
