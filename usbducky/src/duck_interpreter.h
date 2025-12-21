#ifndef DUCK_INTERPRETER_H
#define DUCK_INTERPRETER_H

#include <Arduino.h>
#include <SD.h>
#include "USBHIDKeyboard.h"

extern USBHIDKeyboard Keyboard;

// --- ĐỊNH NGHĨA LOCALE US (Mặc định) ---

#define SHIFT (KEY_LEFT_SHIFT << 8)

// Định nghĩa các phím ASCII còn thiếu
#ifndef KEY_SPACE
#define KEY_SPACE ' '
#endif
#ifndef KEY_QUOTE
#define KEY_QUOTE '\''
#endif
#ifndef KEY_COMMA
#define KEY_COMMA ','
#endif
#ifndef KEY_MINUS
#define KEY_MINUS '-'
#endif
#ifndef KEY_PERIOD
#define KEY_PERIOD '.'
#endif
#ifndef KEY_SLASH
#define KEY_SLASH '/'
#endif
#ifndef KEY_SEMICOLON
#define KEY_SEMICOLON ';'
#endif
#ifndef KEY_EQUAL
#define KEY_EQUAL '='
#endif
#ifndef KEY_LEFT_BRACE
#define KEY_LEFT_BRACE '['
#endif
#ifndef KEY_BACKSLASH
#define KEY_BACKSLASH '\\'
#endif
#ifndef KEY_RIGHT_BRACE
#define KEY_RIGHT_BRACE ']'
#endif
#ifndef KEY_GRAVE
#define KEY_GRAVE '`'
#endif
#ifndef KEY_ENTER
#define KEY_ENTER KEY_RETURN
#endif

// Định nghĩa nhanh cho các phím số và chữ cái
#define KEY_0 '0'
#define KEY_1 '1'
#define KEY_2 '2'
#define KEY_3 '3'
#define KEY_4 '4'
#define KEY_5 '5'
#define KEY_6 '6'
#define KEY_7 '7'
#define KEY_8 '8'
#define KEY_9 '9'

#define KEY_A 'a'
#define KEY_B 'b'
#define KEY_C 'c'
#define KEY_D 'd'
#define KEY_E 'e'
#define KEY_F 'f'
#define KEY_G 'g'
#define KEY_H 'h'
#define KEY_I 'i'
#define KEY_J 'j'
#define KEY_K 'k'
#define KEY_L 'l'
#define KEY_M 'm'
#define KEY_N 'n'
#define KEY_O 'o'
#define KEY_P 'p'
#define KEY_Q 'q'
#define KEY_R 'r'
#define KEY_S 's'
#define KEY_T 't'
#define KEY_U 'u'
#define KEY_V 'v'
#define KEY_W 'w'
#define KEY_X 'x'
#define KEY_Y 'y'
#define KEY_Z 'z'

const uint16_t locale_us[] PROGMEM = {
    // 0-31 (Control characters)
    0, 0, 0, 0, 0, 0, 0, 0, KEY_BACKSPACE, KEY_TAB, KEY_ENTER, 0, 0, KEY_ENTER, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // 32-47 (Space to /)
    KEY_SPACE, SHIFT|KEY_1, SHIFT|KEY_QUOTE, SHIFT|KEY_3, SHIFT|KEY_4, SHIFT|KEY_5, SHIFT|KEY_7, KEY_QUOTE,
    SHIFT|KEY_9, SHIFT|KEY_0, SHIFT|KEY_8, SHIFT|KEY_EQUAL, KEY_COMMA, KEY_MINUS, KEY_PERIOD, KEY_SLASH,
    // 48-63 (0 to ?)
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    SHIFT|KEY_SEMICOLON, KEY_SEMICOLON, SHIFT|KEY_COMMA, KEY_EQUAL, SHIFT|KEY_PERIOD, SHIFT|KEY_SLASH,
    // 64-79 (@ to O)
    SHIFT|KEY_2, SHIFT|KEY_A, SHIFT|KEY_B, SHIFT|KEY_C, SHIFT|KEY_D, SHIFT|KEY_E, SHIFT|KEY_F, SHIFT|KEY_G,
    SHIFT|KEY_H, SHIFT|KEY_I, SHIFT|KEY_J, SHIFT|KEY_K, SHIFT|KEY_L, SHIFT|KEY_M, SHIFT|KEY_N, SHIFT|KEY_O,
    // 80-95 (P to _)
    SHIFT|KEY_P, SHIFT|KEY_Q, SHIFT|KEY_R, SHIFT|KEY_S, SHIFT|KEY_T, SHIFT|KEY_U, SHIFT|KEY_V, SHIFT|KEY_W,
    SHIFT|KEY_X, SHIFT|KEY_Y, SHIFT|KEY_Z, KEY_LEFT_BRACE, KEY_BACKSLASH, KEY_RIGHT_BRACE, SHIFT|KEY_6, SHIFT|KEY_MINUS,
    // 96-111 (` to o)
    KEY_GRAVE, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
    KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,
    // 112-127 (p to DEL)
    KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W,
    KEY_X, KEY_Y, KEY_Z, SHIFT|KEY_LEFT_BRACE, SHIFT|KEY_BACKSLASH, SHIFT|KEY_RIGHT_BRACE, SHIFT|KEY_GRAVE, 0
};

// Hàm gõ một ký tự dựa trên locale
static void typeChar(char c) {
    // Đọc keycode từ PROGMEM
    // Ép kiểu uint8_t để xử lý đúng chỉ số mảng
    uint16_t keycode = pgm_read_word_near(locale_us + (uint8_t)c);
    
    if (keycode) {
        // High byte is the modifier (e.g., SHIFT)
        uint8_t modifier = keycode >> 8;
        if (modifier) Keyboard.press(modifier);
        
        // Low byte is the key
        Keyboard.press(keycode & 0xFF);
        Keyboard.releaseAll();
    }
}

// Biến lưu độ trễ mặc định (mặc định 0ms)
static int defaultDelay = 0;

// Helper: Chuyển đổi chuỗi số (hỗ trợ hex 0x...)
long parseNumber(String str) {
    str.trim();
    if (str.startsWith("0x") || str.startsWith("0X")) {
        return strtol(str.c_str(), NULL, 16);
    }
    return str.toInt();
}

// Helper: Chuyển đổi tên phím sang mã phím HID
uint8_t getKeyCode(String k) {
    // Giữ nguyên case cho ký tự đơn để tránh lỗi Shift (ví dụ: 'r' trong 'GUI r')
    if (k.length() == 1) return (uint8_t)k[0]; 
    
    String kUpper = k;
    kUpper.toUpperCase();
    
    if (kUpper == "ENTER" || kUpper == "RETURN") return KEY_RETURN;
    if (kUpper == "ESC" || kUpper == "ESCAPE") return KEY_ESC;
    if (kUpper == "BACKSPACE") return KEY_BACKSPACE;
    if (kUpper == "TAB") return KEY_TAB;
    if (kUpper == "SPACE") return ' ';
    if (kUpper == "CAPSLOCK" || kUpper == "CAPS") return KEY_CAPS_LOCK;
    
    if (kUpper == "PRINTSCREEN") return 206; // 0xCE
    if (kUpper == "SCROLLLOCK") return 207;  // 0xCF
    if (kUpper == "PAUSE" || kUpper == "BREAK") return 208; // 0xD0
    if (kUpper == "NUMLOCK") return 219;     // 0xDB
    if (kUpper == "MENU" || kUpper == "APP") return 0xED; // 0xED

    if (kUpper.startsWith("F")) {
        int f = kUpper.substring(1).toInt();
        if (f >= 1 && f <= 12) return KEY_F1 + (f - 1);
        if (f >= 13 && f <= 24) return KEY_F13 + (f - 13);
    }
    
    if (kUpper == "UP" || kUpper == "UPARROW") return KEY_UP_ARROW;
    if (kUpper == "DOWN" || kUpper == "DOWNARROW") return KEY_DOWN_ARROW;
    if (kUpper == "LEFT" || kUpper == "LEFTARROW") return KEY_LEFT_ARROW;
    if (kUpper == "RIGHT" || kUpper == "RIGHTARROW") return KEY_RIGHT_ARROW;
    
    if (kUpper == "DELETE") return KEY_DELETE;
    if (kUpper == "PAGEUP") return KEY_PAGE_UP;
    if (kUpper == "PAGEDOWN") return KEY_PAGE_DOWN;
    if (kUpper == "HOME") return KEY_HOME;
    if (kUpper == "END") return KEY_END;
    if (kUpper == "INSERT") return KEY_INSERT;
    
    return 0;
}

// Helper: Nhấn một token (Modifier hoặc Phím thường)
void pressToken(String token) {
    String tokenUpper = token;
    tokenUpper.toUpperCase();

    if (tokenUpper == "CTRL" || tokenUpper == "CONTROL") Keyboard.press(KEY_LEFT_CTRL);
    else if (tokenUpper == "SHIFT") Keyboard.press(KEY_LEFT_SHIFT);
    else if (tokenUpper == "ALT") Keyboard.press(KEY_LEFT_ALT);
    else if (tokenUpper == "GUI" || tokenUpper == "WINDOWS" || tokenUpper == "COMMAND") Keyboard.press(KEY_LEFT_GUI);
    else if (tokenUpper == "RCTRL") Keyboard.press(KEY_RIGHT_CTRL);
    else if (tokenUpper == "RSHIFT") Keyboard.press(KEY_RIGHT_SHIFT);
    else if (tokenUpper == "RALT") Keyboard.press(KEY_RIGHT_ALT);
    else if (tokenUpper == "RGUI") Keyboard.press(KEY_RIGHT_GUI);
    else {
        // Truyền token gốc (chưa uppercase) vào getKeyCode để giữ case cho ký tự đơn
        uint8_t code = getKeyCode(token); 
        if (code != 0) Keyboard.press(code);
    }
}

// Helper: Xử lý logic từng dòng lệnh
void processLine(String line) {
    line.trim();
    if (line.length() == 0) return;

    int sp = line.indexOf(' ');
    String cmd = (sp == -1) ? line : line.substring(0, sp);
    String args = (sp == -1) ? "" : line.substring(sp + 1);

    String cmdUpper = cmd;
    cmdUpper.toUpperCase();

    if (cmdUpper == "REM") return;

    if (cmdUpper == "STRING") {
        for (size_t i = 0; i < args.length(); i++) {
            typeChar(args[i]);
        }
    } 
    else if (cmdUpper == "DELAY") {
        delay(parseNumber(args));
    } 
    else if (cmdUpper == "DEFAULTDELAY" || cmdUpper == "DEFAULT_DELAY") {
        defaultDelay = parseNumber(args);
    }
    else if (cmdUpper == "KEYCODE") {
        // KEYCODE key [modifier]
        // Ví dụ: KEYCODE 0x04 0x02 (A, Shift)
        int sp1 = args.indexOf(' ');
        if (sp1 == -1) {
            uint8_t key = (uint8_t)parseNumber(args);
            if(key) {
                Keyboard.press(key);
                Keyboard.releaseAll();
            }
        } else {
            uint8_t key = (uint8_t)parseNumber(args.substring(0, sp1));
            uint8_t mod = (uint8_t)parseNumber(args.substring(sp1 + 1));
            if(mod) Keyboard.press(mod);
            if(key) Keyboard.press(key);
            Keyboard.releaseAll();
        }
    }
    else {
        // Xử lý phím hoặc tổ hợp phím (VD: "CTRL ALT DELETE", "GUI r", "ENTER")
        int start = 0;
        int end = line.indexOf(' ');
        while (end != -1) {
            pressToken(line.substring(start, end));
            start = end + 1;
            end = line.indexOf(' ', start);
        }
        pressToken(line.substring(start)); // Token cuối cùng
        Keyboard.releaseAll();
    }
    
    if (defaultDelay > 0) delay(defaultDelay);
}

// Hàm thực thi kịch bản từ đường dẫn file trên SD
void runPayload(String filename) {
    File file = SD.open(filename, FILE_READ);
    if (!file) {
        Serial.println("Lỗi: Không tìm thấy file trên SD!");
        return;
    }

    Serial.println("Đang thực thi: " + filename);
    defaultDelay = 0; // Reset delay mặc định
    String lastLine = "";

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.length() == 0) continue;

        // Tạo bản sao chữ hoa để kiểm tra lệnh (giữ nguyên line gốc cho lệnh STRING)
        String upperLine = line;
        upperLine.toUpperCase();

        // Xử lý lệnh REPEAT (Lặp lại lệnh trước đó)
        if (upperLine.startsWith("REPEAT")) {
            int count = 1;
            int sp = upperLine.indexOf(' ');
            if (sp != -1) count = upperLine.substring(sp + 1).toInt();
            
            for (int i = 0; i < count; i++) {
                processLine(lastLine);
            }
            continue;
        }
        
        processLine(line);
        
        // Lưu lại dòng lệnh (trừ REM) để dùng cho REPEAT
        if (!upperLine.startsWith("REM")) {
            lastLine = line;
        }
    }
    file.close();
    Serial.println("Hoàn thành payload.");
}

#endif