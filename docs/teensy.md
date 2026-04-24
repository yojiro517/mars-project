# Teensy 4.1 ファームウェア解説

## 概要

Teensy 4.1 上で動作するファームウェアで、ローバーの**物理的な駆動とセンサー計測**を担当します。ESP32 から UART 経由で受信した操縦コマンドに基づいてサーボモーターを制御し、BME280 センサーのデータをテレメトリとして ESP32 に返送します。

主な機能:

1. **サーボモーター制御**: 左右2つの連続回転サーボモーターを使った差動駆動
2. **BME280 センサー読み取り**: 気圧・温度・湿度を計測し、バイナリフレームとして送信
3. **LED 制御**: 緑・赤 LED の点滅制御

## ビルド環境

### PlatformIO 設定 (`platformio.ini`)

```ini
[env:teensy41]
platform = teensy
board = teensy41
framework = arduino
monitor_speed = 115200
lib_deps =
    https://github.com/ut-issl/cansat-school-arduino-lib.git#v2025.1.0
```

- **ボード**: Teensy 4.1
- **フレームワーク**: Arduino
- **依存ライブラリ**: [cansat-school-arduino-lib](https://github.com/ut-issl/cansat-school-arduino-lib) v2025.1.0
  - `CanSatSchool.h` を提供。BME280 センサー (`BaroThermoHygrometer`)、LED (`Led`)、サーボモーター (`ServoMotor`) のラッパークラスが含まれる

### ビルド・書き込み手順

```bash
cd teensy

# ビルドのみ
pio run

# ビルド + 書き込み
pio run --target upload

# シリアルモニターを開く
pio device monitor
```

### コンパイルスイッチ

| 定義 | 効果 |
|------|------|
| `DEBUG_MODE` | UART通信先を `Serial`（USB）に切り替え。ESP32未接続でもPCから直接コマンドを入力してデバッグ可能 |

---

## ハードウェア接続

### ピン割り当て

| ピン番号 | 用途 | 接続先 |
|---------|------|--------|
| 2 | 右サーボモーター | PWM信号 |
| 3 | 左サーボモーター | PWM信号 |
| 22 | 緑LED | デジタル出力 |
| 23 | 赤LED | デジタル出力 |
| Serial5 TX/RX | UART通信 | ESP32 (RX=GPIO2 / TX=GPIO1) |
| SDA/SCL (Wire) | I2C | BME280センサー |

---

## ソースコード解説

### `main.cpp` — メインループ

#### 初期化 (`setup()`)

```
Serial  (USB)  → 115200bps  デバッグ出力用
Serial5 (UART) → 115200bps  ESP32 との通信用
Wire (I2C)     → BME280 センサー
green_led, red_led → LED初期化
bth → BME280初期化
servo_maneuver → サーボモーター初期化（停止状態）
```

各ペリフェラルを順次初期化します。サーボモーターは初期化時に `stop()` が呼ばれ、停止状態で起動します。

#### メインループ (`loop()`)

```
loop() {
  ① Serial5(UART)からコマンド文字列を1行読み取り
  ② command_execute() で対応する動作を実行
  ③ 100ms待機
}
```

ESP32 から `\n` 区切りで送られてくるコマンド文字列を読み取り、`command_execute()` に渡します。

#### コマンド実行 (`command_execute()`)

各コマンドに対応する動作:

| コマンド | 実行される動作 |
|---------|--------------|
| `W` | `servo_maneuver.moveForward()` — 前進 |
| `S` | `servo_maneuver.moveBackward()` — 後退 |
| `A` | `servo_maneuver.turnLeft()` — 左旋回 |
| `D` | `servo_maneuver.turnRight()` — 右旋回 |
| `B` | `servo_maneuver.stop()` — 停止 |
| `G` | `green_led.blink(1000)` — 緑LED 1秒点滅 |
| `R` | `red_led.blink(1000)` — 赤LED 1秒点滅 |

**重要**: どのコマンドが来ても、コマンド実行後に必ず `send_bth_data()` が呼ばれ、最新のセンサーデータが ESP32 に送信されます。

#### テレメトリ送信 (`send_bth_data()`)

BME280 からセンサーデータを読み取り、以下のバイナリフレームを UART で送信します。

```
バイト構成 (合計15バイト):
  [0]     0x5C          ヘッダ1
  [1]     0x94          ヘッダ2
  [2-5]   float         気圧 (hPa)
  [6-9]   float         温度 (℃)
  [10-13] float         湿度 (%)
  [14]    '\n'          フッタ (改行)
```

- `bth.read()` が返す `BaroThermoHygrometer_t` 構造体を `memcpy` でバイナリとしてコピー
- ヘッダ `0x5C 0x94` は ESP32 側でフレーム同期に使用される固定パターン
- フッタの `\n` はフレーム終端を示す

---

### `servo_maneuver.cpp / .h` — サーボモーター制御

#### `ServoManeuver` クラス

左右2つの連続回転サーボモーターを制御し、差動駆動方式でローバーの移動を実現します。

#### 差動駆動の仕組み

左右のモーターの回転方向と速度の組み合わせで、ローバーの動きを制御します。

```
前進: 左モーター → 正転(+15)    右モーター → 逆転(-10)
後退: 左モーター → 逆転(-10)    右モーター → 正転(+10)
左旋回: 左モーター → 低速正転(+10) 右モーター → 逆転(-10)
右旋回: 左モーター → 正転(+15)    右モーター → 低速逆転(-5)
停止:  左モーター → 0            右モーター → 0
```

左右のモーターは物理的に鏡像配置のため、同じ方向に進むには**逆符号の速度**を与える必要があります。

#### 速度パラメータ

| パラメータ | 値 | 用途 |
|-----------|-----|------|
| `stop_speed_` | 0 | 停止 |
| `left_forward_speed_` | +15 | 左モーター前進（通常速度） |
| `left_forward_speed_slow_` | +10 | 左モーター前進（旋回用低速） |
| `left_backward_speed_` | -10 | 左モーター後退 |
| `right_forward_speed_` | -10 | 右モーター前進（通常速度） |
| `right_forward_speed_slow_` | -5 | 右モーター前進（旋回用低速） |
| `right_backward_speed_` | +10 | 右モーター後退 |

旋回は片輪を低速にすることで実現しています（信地旋回）。左右の速度差が大きいほど旋回半径が小さくなります。
