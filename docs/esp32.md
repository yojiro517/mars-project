# ESP32-S3 Sense ファームウェア解説

## 概要

XIAO ESP32-S3 Sense 上で動作するファームウェアです。ローバーシステムにおける**通信ハブ**として機能し、以下の3つの役割を担います。

1. **WiFi アクセスポイント**: SSID `ESP32S3Sense` で AP を起動し、地上局 PC が接続する無線ネットワークを提供
2. **コマンド中継**: 地上局から UDP で受信した操縦コマンドを UART 経由で Teensy に転送
3. **カメラ撮影・送信**: OV2640 カメラで撮影した画像を UDP で地上局に送信（`USE_CAMERA` 定義時のみ有効）

## ビルド環境

### PlatformIO 設定 (`platformio.ini`)

```ini
[env:seeed_xiao_esp32s3]
platform = espressif32
board = seeed_xiao_esp32s3
framework = arduino
monitor_speed = 115200
build_flags = -DARDUINO_USB_CDC_ON_BOOT=1
```

- **ボード**: Seeed XIAO ESP32-S3
- **フレームワーク**: Arduino
- **`ARDUINO_USB_CDC_ON_BOOT=1`**: USB CDC（シリアル通信）をブート時に有効化。`Serial` が USB 経由で使えるようになる

### ビルド・書き込み手順

```bash
cd esp32

# ビルドのみ
pio run

# ビルド + 書き込み
pio run --target upload

# シリアルモニターを開く
pio device monitor
```

### コンパイルスイッチ

ソースコード内の `#define` で以下の機能を切り替えられます。

| 定義 | 効果 |
|------|------|
| `DEBUG_MODE` | UART通信先を `Serial`（USB）に切り替え。Teensy未接続でもPCからデバッグ可能 |
| `USE_CAMERA` | カメラ機能を有効化。未定義時はカメラ初期化・画像送信をスキップ |

---

## ソースコード解説

### `main.cpp` — メインループとコマンド処理

#### 初期化 (`setup()`)

```
Serial  (USB)  → 115200bps  デバッグ出力用
Serial1 (UART) → 115200bps  Teensy との通信用 (TX=GPIO1, RX=GPIO2)
WiFi AP起動 → UDP受信開始 → カメラ初期化(USE_CAMERA時)
```

1. USB シリアルとUARTシリアルを115200bpsで初期化
2. `wifiUdp.init()` で WiFi AP を起動し、UDP 受信ポート(55555)をバインド
3. `USE_CAMERA` が定義されていれば `camera.init()` でカメラを初期化

#### メインループ (`loop()`)

```
loop() {
  ① UDPパケットを受信 → コマンドをパースして process_command() に渡す
  ② 500ms以上コマンド未受信 → 自動停止 "B" を発行
  ③ 200ms間隔でカメラ画像を地上局に送信 (USE_CAMERA時)
}
```

- **① コマンド受信**: `wifiUdp.parsePacket()` で受信を確認。データがあればバッファに読み込み、送信元IPとポートを記録して `process_command()` に渡す
- **② 自動停止**: `lastCommandTime` からの経過時間が `timeout`（500ms）を超えると、安全のため停止コマンド `"B"` を自動発行する。通信途絶時にローバーが暴走するのを防ぐフェイルセーフ機構
- **③ カメラ送信**: `camera_interval`（200ms）間隔でカメラ画像をキャプチャし、地上局（`CONSOLE_IP:CONSOLE_PORT`）にUDP送信

#### コマンド処理 (`process_command()`)

この関数は2つの処理を行います。

**1. Teensyからのテレメトリ受信（UARTフレーム解析）**

Teensy が送信するバイナリフレーム（ヘッダ `0x5C 0x94` + データ + `\n`）をバイト単位で読み取り、完全なフレームを `latestFrame` に格納します。

```
状態遷移:
  [ヘッダ検出中] → 0x5C, 0x94 を検出 → [フレーム収集中]
  [フレーム収集中] → '\n' 受信 かつ 15バイト以上 → フレーム完成
  [フレーム収集中] → バッファ溢れ → [ヘッダ検出中] に戻る（破棄）
```

**2. コマンドのTeensyへの転送**

受信したコマンド文字列に応じて UART (`Serial1`) 経由で Teensy にコマンドを送信します。

| コマンド | 動作 |
|---------|------|
| `W/S/A/D` | 移動コマンドをそのまま転送 |
| `R/G` | LED制御コマンドをそのまま転送 |
| `B` | 停止コマンドを転送 |
| `T` | テレメトリ応答: `latestFrame` の内容を UDP で地上局に返送 |

`T` コマンドの場合のみ、Teensy への転送は行わず、ESP32 が保持している最新のテレメトリフレームを地上局に直接返送します。フレームの妥当性（長さ15バイト、ヘッダ `0x5C 0x94`、フッタ `\n`）を検証してから送信します。

---

### `wifi_udp.cpp / .hpp` — WiFi・UDP 通信

#### `WifiUdp` クラス

`WiFiUDP` クラスを継承し、WiFi AP の起動と UDP 通信をまとめて管理します。

#### `init()` — WiFi AP の起動

1. `WiFi.softAP(ssid, password)` でアクセスポイントを開始
2. `WiFi.softAPConfig()` で固定IP `192.168.1.1` を設定
3. UDP ポート `55555` で受信を開始
4. 送信電力を `15dBm` に設定

#### `send()` — 画像データ送信

カメラ画像の送信に使用されます。パケット構成:

```
[パケット番号(1byte)] [データ(IMAGE_SIZE bytes)] [チェックサム(1byte)]
```

- `IMAGE_SIZE` = 1440 バイト
- チェックサムはデータ部の全バイトの合計（8bit）

#### `send_data()` — テレメトリデータ送信

指定されたIPアドレス・ポートに生バイトデータをそのまま UDP で送信します。テレメトリフレームの転送に使用されます。

#### ネットワーク定数

| 定数 | 値 | 用途 |
|------|-----|------|
| `ESP32_PORT` | 55555 | ESP32のUDP受信ポート |
| `IMAGE_SIZE` | 1440 | 1パケットあたりの画像データサイズ |
| `CONSOLE_IP` | "192.168.1.2" | 地上局のIPアドレス |
| `CONSOLE_PORT` | 50000 | 地上局の受信ポート |

---

### `camera.cpp / .hpp` — カメラ制御

#### `init()` — カメラ初期化

OV2640 カメラを以下の設定で初期化します。

| 設定項目 | 値 | 説明 |
|---------|-----|------|
| `frame_size` | `FRAMESIZE_240X240` | 240x240ピクセル |
| `pixel_format` | `PIXFORMAT_RGB565` | RGB565形式（1ピクセル16bit） |
| `fb_location` | `CAMERA_FB_IN_PSRAM` | フレームバッファをPSRAMに配置 |
| `fb_count` | 2 | ダブルバッファリング |
| `grab_mode` | `CAMERA_GRAB_LATEST` | 常に最新フレームを取得 |

GPIOピンのマッピングは `camera.hpp` で定義されており、XIAO ESP32-S3 Sense のハードウェアに合わせた設定になっています。

#### `send_photo()` — 画像送信

RGB565 形式の画像データを R/G/B チャンネルに分離し、UDP パケットとして送信します。

**RGB565 からの色分離処理:**

1. 隣接する2ピクセル（4バイト）を読み取り、それぞれ16bitの `p0`, `p1` として解釈
2. 各ピクセルから R(4bit), G(4bit), B(4bit) を抽出し、2ピクセル分を1バイトに詰める
3. R, G, B それぞれを別パケットとして送信

```
RGB565 (16bit):  RRRRR GGGGGG BBBBB
                  ↓ 上位4bitを抽出
                 R:4bit  G:4bit  B:4bit
```

**パケット番号の割り当て:**

- 画像は20行ブロックに分割（各ブロック = 12行 x 240列）
- 各ブロックにつきR/G/Bの3パケットを送信
- パケット番号: `3*i+1`(R), `3*i+2`(G), `3*i+3`(B) （i = 0〜19）
- 合計60パケットで1フレーム送信
