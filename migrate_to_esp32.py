#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Arduino → ESP32 自动化迁移脚本
自动转换 Arduino 代码到 ESP32 版本
"""

import sys
import os
import re
from pathlib import Path

class ArduinoToESP32Converter:
    def __init__(self, input_file):
        self.input_file = input_file
        self.output_file = input_file.replace('.ino', '_esp32.ino')

        # PIN 映射表
        self.pin_mapping = {
            'SERVO_PIN': {'old': 8, 'new': 5},
            'SWITCH_OPEN': {'old': 2, 'new': 35},
            'SWITCH_CLOSE': {'old': 3, 'new': 34},
            'SWITCH_HOLD': {'old': 4, 'new': 39},
            'VIBRATION_PIN': {'old': 6, 'new': 18},
        }

        self.i2c_mapping = {
            'A4': 'GPIO 21 (SDA)',
            'A5': 'GPIO 22 (SCL)',
        }

        self.replacements = [
            # PIN 定义
            (r'const int SERVO_PIN = 8;', 'const int SERVO_PIN = 5;'),
            (r'const int SWITCH_OPEN = 2;', 'const int SWITCH_OPEN = 35;'),
            (r'const int SWITCH_CLOSE = 3;', 'const int SWITCH_CLOSE = 34;'),
            (r'const int SWITCH_HOLD = 4;', 'const int SWITCH_HOLD = 39;'),
            (r'const int VIBRATION_PIN = 6;', 'const int VIBRATION_PIN = 18;'),

            # 波特率
            (r'Serial\.begin\(9600\);', 'Serial.begin(115200);  // ESP32 faster baud rate'),

            # INPUT_PULLUP (按键不能用)
            (r'pinMode\(SWITCH_OPEN, INPUT_PULLUP\);', 'pinMode(SWITCH_OPEN, INPUT);       // ESP32 input-only, no internal pull-up'),
            (r'pinMode\(SWITCH_CLOSE, INPUT_PULLUP\);', 'pinMode(SWITCH_CLOSE, INPUT);      // ESP32 input-only, no internal pull-up'),
            (r'pinMode\(SWITCH_HOLD, INPUT_PULLUP\);', 'pinMode(SWITCH_HOLD, INPUT);       // ESP32 input-only, no internal pull-up'),

            # I2C 初始化
            (r'Wire\.begin\(\);', 'Wire.begin(21, 22);  // ESP32: SDA=GPIO 21, SCL=GPIO 22'),

            # EEPROM 初始化
            (r'// Load configuration from EEPROM', 'EEPROM.begin(512);  // Initialize EEPROM\n\n  // Load configuration from EEPROM'),

            # EEPROM 提交
            (r'Serial\.println\("\✓ Configuration saved to EEPROM"\);',
             'EEPROM.commit();  // Important: commit EEPROM changes on ESP32\n  Serial.println("✓ Configuration saved to EEPROM");'),
        ]

    def read_file(self):
        """读取输入文件"""
        try:
            with open(self.input_file, 'r', encoding='utf-8') as f:
                return f.read()
        except FileNotFoundError:
            print(f"❌ 错误：找不到文件 {self.input_file}")
            return None

    def convert(self, content):
        """转换代码"""
        result = content

        # 应用所有替换规则
        for pattern, replacement in self.replacements:
            result = re.sub(pattern, replacement, result)

        # 添加 ESP32 标记
        if 'Electric Gripper Control System v2.0' in result:
            result = result.replace(
                'Electric Gripper Control System v2.0',
                'Electric Gripper Control System v2.0 - ESP32 Version'
            )

        # 添加注释说明这是自动转换的
        header = "// ⚠️  自动转换生成 - 由 migrate_to_esp32.py 脚本生成\n// ⚠️  请在上传前验证所有 PIN 定义\n\n"
        result = header + result

        return result

    def write_file(self, content):
        """写入输出文件"""
        try:
            with open(self.output_file, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        except IOError as e:
            print(f"❌ 错误：无法写入文件 {self.output_file}: {e}")
            return False

    def migrate(self):
        """执行迁移"""
        print(f"\n{'='*50}")
        print(f"Arduino → ESP32 自动化迁移脚本")
        print(f"{'='*50}\n")

        print(f"📖 输入文件: {self.input_file}")

        # 检查文件是否存在
        if not os.path.exists(self.input_file):
            print(f"❌ 错误：文件不存在")
            return False

        # 读取文件
        print("📖 正在读取文件...")
        content = self.read_file()
        if content is None:
            return False

        # 转换代码
        print("🔄 正在转换代码...")
        converted = self.convert(content)

        # 写入文件
        print(f"💾 正在保存到: {self.output_file}")
        if not self.write_file(converted):
            return False

        # 显示转换摘要
        self.print_summary()

        print(f"\n✅ 迁移完成！")
        print(f"\n📋 接下来的步骤:")
        print(f"  1. 检查生成的 {self.output_file}")
        print(f"  2. 验证所有 PIN 定义是否正确")
        print(f"  3. 根据 PIN_MAPPING.md 重新连接硬件")
        print(f"  4. 在 Arduino IDE 中选择 ESP32 Dev Module")
        print(f"  5. 上传代码\n")

        return True

    def print_summary(self):
        """打印转换摘要"""
        print(f"\n📊 转换摘要:")
        print(f"  PIN 定义改动:")
        print(f"    SERVO_PIN:      PIN 8 → GPIO 5")
        print(f"    SWITCH_OPEN:    PIN 2 → GPIO 35")
        print(f"    SWITCH_CLOSE:   PIN 3 → GPIO 34")
        print(f"    SWITCH_HOLD:    PIN 4 → GPIO 39")
        print(f"    VIBRATION_PIN:  PIN 6 → GPIO 18")
        print(f"\n  其他改动:")
        print(f"    ✓ 波特率: 9600 → 115200")
        print(f"    ✓ I2C: Wire.begin() → Wire.begin(21, 22)")
        print(f"    ✓ INPUT_PULLUP 改为 INPUT（需要外部上拉电阻）")
        print(f"    ✓ 添加 EEPROM.begin(512) 和 EEPROM.commit()")


def main():
    if len(sys.argv) < 2:
        print("用法: python migrate_to_esp32.py <input_file>")
        print("例如: python migrate_to_esp32.py gripper_main.ino")
        print("输出: gripper_main_esp32.ino")
        sys.exit(1)

    input_file = sys.argv[1]
    converter = ArduinoToESP32Converter(input_file)

    if converter.migrate():
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == '__main__':
    main()
