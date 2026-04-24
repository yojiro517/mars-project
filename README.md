# MARS - Micro Ai Robot for Students

CanSat競技用の火星探査ローバーシステムです。地上局（PC）からWiFi経由でローバーを遠隔操縦し、カメラ映像の受信やセンサーテレメトリの取得を行います。

## システム構成

システムは以下の3つのコンポーネントで構成されています。

```
┌──────────────┐    UDP/WiFi    ┌──────────────┐    UART     ┌───────────┐
│ Ground       │ ◄────────────► │ ESP32-S3     │ ◄─────────► │ Teensy4.1 │
│ Station (PC) │  192.168.1.x   │ Sense        │  Serial     │           │
│              │                │ (通信+カメラ) │  115200bps  │ (駆動+計測)│
└──────────────┘                └──────────────┘             └───────────┘
```

- **Ground Station** : PCから操縦コマンドを送信し、テレメトリ・カメラ映像を受信する地上局ソフトウェア
- **ESP32-S3 Sense** : WiFiアクセスポイントとして動作し、地上局とTeensyの間の通信を中継。カメラ撮影も担当
- **Teensy 4.1** : サーボモーターの駆動制御と、BME280センサー（気圧・温度・湿度）の計測を担当

## ディレクトリ構成

```
mars-project/
├── README.md
├── docs/                   # 各コンポーネントの詳細ドキュメント
│   ├── esp32.md
│   ├── teensy.md
│   └── ground_station.md
├── esp32/                  # ESP32-S3 Sense ファームウェア (PlatformIO)
│   ├── platformio.ini
│   └── src/
│       ├── main.cpp        # メインループ・コマンド処理
│       ├── camera.cpp      # カメラ制御・画像送信
│       ├── camera.hpp
│       ├── wifi_udp.cpp    # WiFi AP・UDP通信
│       └── wifi_udp.hpp
├── teensy/                 # Teensy 4.1 ファームウェア (PlatformIO)
│   ├── platformio.ini
│   └── src/
│       ├── main.cpp        # メインループ・コマンド実行・テレメトリ送信
│       ├── servo_maneuver.cpp  # サーボモーター制御
│       └── servo_maneuver.h
└── ground_station/         # 地上局ソフトウェア (Python)
    └── GS.py               # キーボード操縦・テレメトリ表示・カメラ表示
```

## 通信プロトコル

### 操縦コマンド（地上局 → ESP32 → Teensy）

| コマンド | 動作 |
|---------|------|
| `W` | 前進 |
| `S` | 後退 |
| `A` | 左旋回 |
| `D` | 右旋回 |
| `B` | 停止 |
| `G` | 緑LED点滅 |
| `R` | 赤LED点滅 |
| `T` | テレメトリ要求（地上局→ESP32のみ） |

コマンドが500ms以上受信されない場合、自動的に停止（`B`）コマンドが発行されます。

### テレメトリパケット（Teensy → ESP32 → 地上局）

Teensyが送信するBME280センサーデータのバイナリフレーム構成:

| オフセット | サイズ | 内容 |
|-----------|--------|------|
| 0 | 1 byte | ヘッダ `0x5C` |
| 1 | 1 byte | ヘッダ `0x94` |
| 2 | 4 bytes | 気圧 (float, little-endian) |
| 6 | 4 bytes | 温度 (float, little-endian) |
| 10 | 4 bytes | 湿度 (float, little-endian) |
| 14 | 1 byte | フッタ `\n` |

### カメラ画像（ESP32 → 地上局）

- 解像度: 240x240 (RGB565)
- 画像はRGBチャンネルごとに分割され、UDPパケットとして送信
- 各パケットにはパケット番号とチェックサムが付与
- パケット番号 `0xFF` が全パケット送信完了の合図

## ネットワーク構成

| 機器 | IPアドレス | ポート |
|------|-----------|--------|
| ESP32-S3 (AP) | 192.168.1.1 | 55555 (受信) |
| Ground Station | 192.168.1.2 | 50000 (受信) |

WiFi SSID: `ESP32S3Sense` / パスワード: `Password`

> **複数チームで同時に開発・運用する場合**: 各チームの WiFi AP が干渉しないよう、SSID とパスワードをチームごとに変更してください。`esp32/src/main.cpp` の `ssid` / `password` を書き換えるだけで対応できます。例: `ESP32S3Sense_TeamA`。地上局側（`ground_station/GS.py`）は接続先 AP を変えるだけで、コードの変更は不要です。

---

## 各ソフトウェアの詳細

### ESP32-S3 Sense ファームウェア (`esp32/`)

#### 概要

XIAO ESP32-S3 Sense上で動作するファームウェアです。以下の3つの役割を持ちます。

1. **WiFiアクセスポイント**: SSID `ESP32S3Sense` でAPを起動し、地上局PCが接続する
2. **コマンド中継**: 地上局からUDPで受信した操縦コマンドをUART経由でTeensyに転送する
3. **カメラ撮影・送信**: OV2640カメラで撮影した画像をUDPで地上局に送信する（`USE_CAMERA`定義時）

#### ファイル構成

- **`main.cpp`**: WiFiとUARTの初期化、UDPコマンドの受信・パースと転送、Teensyからのテレメトリフレーム受信処理を行う。コマンド未受信時の自動停止タイムアウト（500ms）も実装。
- **`wifi_udp.cpp / .hpp`**: `WiFiUDP`を継承した`WifiUdp`クラス。WiFi APの起動、固定IP設定、UDPパケットの送受信をラップする。画像データ送信用の`send()`とテレメトリ転送用の`send_data()`を提供。
- **`camera.cpp / .hpp`**: OV2640カメラの初期化（240x240, RGB565）と画像送信を担当。RGB565の2ピクセル分を4bitずつR/G/Bに分離し、チャンネルごとにUDPパケットとして送信する。

#### ビルド環境

- PlatformIO / Arduino Framework
- ボード: `seeed_xiao_esp32s3`
- UART: TX=GPIO1, RX=GPIO2 (115200bps)

---

### Teensy 4.1 ファームウェア (`teensy/`)

#### 概要

Teensy 4.1上で動作するファームウェアで、ローバーの物理的な駆動とセンサー計測を担当します。

1. **サーボモーター制御**: 左右2つのサーボモーターを使った差動駆動（前進・後退・左右旋回・停止）
2. **BME280センサー読み取り**: 気圧・温度・湿度を計測し、バイナリフレームとしてESP32に送信
3. **LED制御**: 緑・赤LEDの点滅制御

#### ファイル構成

- **`main.cpp`**: ESP32からのUARTコマンドを受信し、対応する動作を実行する。コマンド実行のたびにBME280のセンサーデータをバイナリフレームとしてESP32に返送する。
- **`servo_maneuver.cpp / .h`**: `ServoManeuver`クラス。左右のサーボモーターを差動制御し、前進・後退・左右旋回・停止の動作を提供する。左右のモーターは逆方向に回転するため、速度パラメータの符号が異なる。

#### ピン割り当て

| ピン | 用途 |
|------|------|
| 2 | 右サーボモーター |
| 3 | 左サーボモーター |
| 22 | 緑LED |
| 23 | 赤LED |

#### ビルド環境

- PlatformIO / Arduino Framework
- ボード: `teensy41`
- 依存ライブラリ: [cansat-school-arduino-lib v2025.1.0](https://github.com/ut-issl/cansat-school-arduino-lib)
- UART: Serial5 (115200bps)

---

### 地上局ソフトウェア (`ground_station/`)

#### 概要

PCで動作するPythonスクリプトで、ローバーの遠隔操縦と受信データの表示を行います。

#### 機能

1. **キーボード操縦**: WASDキーでローバーを操縦。キー入力をUDPコマンドとしてESP32に送信する
2. **テレメトリ表示**: BME280のセンサーデータ（気圧・温度・湿度）をコンソールに表示
3. **カメラ映像表示**: ESP32から受信した画像をOpenCVウィンドウにリアルタイム表示。RGB分離表示や色差分表示モードも実装済み（コメントアウト）

#### キー操作

| キー | 動作 |
|------|------|
| `W` | 前進 |
| `S` | 後退 |
| `A` | 左旋回 |
| `D` | 右旋回 |
| `G` | 緑LED |
| `R` | 赤LED |
| `T` | テレメトリ要求 |
| `B` | 停止 |
| `Q` | 終了 |

#### 依存パッケージ

- Python 3.x
- numpy
- opencv-python (`cv2`)
- keyboard

#### 実行方法

```bash
# ESP32のWiFi APに接続した状態で
python ground_station/GS.py
```

---

## 開発環境セットアップ

### 必要なソフトウェア

- [VS Code](https://code.visualstudio.com/) + [PlatformIO IDE 拡張](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)（ESP32・Teensy ファームウェアのビルド・書き込み）
- Python 3.x（地上局ソフトウェア）

### 必要なハードウェア

- XIAO ESP32-S3 Sense
- Teensy 4.1
- BME280センサー（I2C接続）
- 連続回転サーボモーター x2
- 緑LED / 赤LED

### ファームウェアのビルドと書き込み

1. VS Code で `esp32/` または `teensy/` フォルダを開く
2. PlatformIO 拡張が `platformio.ini` を自動認識し、必要な依存ライブラリをインストールする
3. 画面下部のステータスバーから以下の操作を行う
   - **Build（チェックマークアイコン）**: ファームウェアをビルド
   - **Upload（矢印アイコン）**: ボードに書き込み
   - **Serial Monitor（プラグアイコン）**: シリアルモニターを開く

> **補足**: CLI から操作する場合は `pio run` / `pio run --target upload` / `pio device monitor` でも可能です。

### 地上局の実行

```bash
pip install numpy opencv-python keyboard
# ESP32のWiFi AP (ESP32S3Sense) に接続した状態で
python ground_station/GS.py
```

## 詳細ドキュメント

各コンポーネントのソースコード解説は以下を参照してください。

- [ESP32-S3 Sense ファームウェア解説](docs/esp32.md)
- [Teensy 4.1 ファームウェア解説](docs/teensy.md)
- [地上局ソフトウェア解説](docs/ground_station.md)
