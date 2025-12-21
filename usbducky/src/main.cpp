#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <SD.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBMSC.h" // Thêm thư viện MSC

#include "duck_interpreter.h"
#include "web_index.h"

#define SD_CS 34
USBHIDKeyboard Keyboard;
USBMSC MSC; // Khai báo đối tượng MSC
AsyncWebServer server(80);

// Biến cờ hiệu chế độ
bool isMSCMode = false;

void setup() {
    Serial.begin(115200);

    // 0. KIỂM TRA NÚT BOOT (GPIO 0) ĐỂ CHỌN CHẾ ĐỘ
    pinMode(0, INPUT_PULLUP);
    delay(100); // Chờ tín hiệu ổn định
    if (digitalRead(0) == LOW) {
        isMSCMode = true;
        Serial.println(">> DETECTED BOOT BUTTON: Starting MSC Mode (USB Storage)...");
    } else {
        Serial.println(">> Starting Normal Mode (Web + HID)...");
    }

    // 1. Khởi tạo SD Card (Chung cho cả 2 chế độ)
    SPI.begin(36, 37, 35, 34);
    if (!SD.begin(SD_CS, SPI, 40000000)) Serial.println("SD FAIL!");

    // 2. XỬ LÝ PHÂN NHÁNH CHẾ ĐỘ
    if (isMSCMode) {
        // --- CHẾ ĐỘ USB MASS STORAGE ---
        MSC.setID("ESP32", "USB_MSC", "1.0");
        
        // LƯU Ý: Để MSC hoạt động thực sự với thẻ nhớ, bạn cần map các hàm đọc/ghi sector
        // từ thư viện SD/SdFat vào đây. Ví dụ:
        // MSC.setReadWriteCallback(sdReadSector, sdWriteSector, sdFlush);
        // MSC.setCapacity(cardSectorCount, 512);
        // Hiện tại code này chỉ khởi động giao diện USB MSC để máy tính nhận diện thiết bị.
        
        MSC.begin();
        USB.begin();
        
        Serial.println("MSC Started. Connect USB to PC.");
        // Trong chế độ này, vòng lặp vô tận để duy trì USB, không chạy WebServer
        while(1) { 
            delay(100); 
        }
    }

    // --- CHẾ ĐỘ BÌNH THƯỜNG (HID + WEB) ---
    
    Keyboard.begin(); // Chỉ khởi tạo bàn phím ở chế độ thường
    USB.begin();      // Khởi động USB stack

    WiFi.softAP("Test", "04122006");
    // --- CÁC API XỬ LÝ FILE ---

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", INDEX_HTML);
    });

    // API: Cấu trúc cây thư mục cho jstree
    server.on("/tree", HTTP_GET, [](AsyncWebServerRequest *request){
        String path = request->hasParam("id") && request->getParam("id")->value() != "#" ? request->getParam("id")->value() : "/";
        String json = "[";
        File root = SD.open(path);
        File file = root.openNextFile();
        bool first = true;
        while(file){
            if(!first) json += ",";
            String fileName = String(file.name());
            if(fileName.lastIndexOf('/') >= 0) fileName = fileName.substring(fileName.lastIndexOf('/') + 1);
            String fullPath = (path == "/" ? "" : path) + "/" + fileName;
            
            json += "{\"id\":\"" + fullPath + "\",\"text\":\"" + fileName + "\",";
            json += "\"children\":" + String(file.isDirectory() ? "true" : "false") + ",";
            json += "\"type\":\"" + String(file.isDirectory() ? "default" : "file") + "\"}";
            first = false;
            file = root.openNextFile();
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    // API: Lấy nội dung file để sửa
    server.on("/get-content", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("path")){
            String path = request->getParam("path")->value();
            if(SD.exists(path)) request->send(SD, path, "text/plain");
            else request->send(404);
        }
    });

    // API: Lưu file sau khi sửa
    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("path", true) && request->hasParam("content", true)){
            String path = request->getParam("path", true)->value();
            String content = request->getParam("content", true)->value();
            
            // Xóa file cũ để ghi mới (tránh append)
            if(SD.exists(path)) SD.remove(path);
            
            File f = SD.open(path, FILE_WRITE);
            if(f) { f.print(content); f.close(); request->send(200, "text/plain", "Đã lưu thành công!"); }
            else request->send(500);
        }
    });

    // API: Tạo file hoặc folder mới
    server.on("/create", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("path", true) && request->hasParam("name", true) && request->hasParam("type", true)){
            String parent = request->getParam("path", true)->value();
            String name = request->getParam("name", true)->value();
            String type = request->getParam("type", true)->value();
            
            if(!parent.endsWith("/")) parent += "/";
            String fullPath = parent + name;
            fullPath.replace("//", "/"); // Xử lý trường hợp đường dẫn gốc

            if(SD.exists(fullPath)) { request->send(400, "text/plain", "Đã tồn tại!"); return; }

            if(type == "folder") {
                if(SD.mkdir(fullPath)) request->send(200, "text/plain", "OK");
                else request->send(500, "text/plain", "Lỗi tạo folder");
            } else {
                File f = SD.open(fullPath, FILE_WRITE);
                if(f) { f.close(); request->send(200, "text/plain", "OK"); }
                else request->send(500, "text/plain", "Lỗi tạo file");
            }
        }
    });

    // API: Đổi tên file/folder
    server.on("/rename", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("old", true) && request->hasParam("new", true)){
            String oldPath = request->getParam("old", true)->value();
            String newPath = request->getParam("new", true)->value();
            
            if(!SD.exists(oldPath)) { request->send(400, "text/plain", "File không tồn tại!"); return; }
            if(SD.exists(newPath)) { request->send(400, "text/plain", "Tên mới đã tồn tại!"); return; }
            
            if(SD.rename(oldPath, newPath)) request->send(200, "text/plain", "OK");
            else request->send(500, "text/plain", "Lỗi đổi tên");
        }
    });

    // API: Thực thi payload
    server.on("/run", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("f")) {
            runPayload(request->getParam("f")->value());
            request->send(200);
        }
    });

    // Load tài nguyên tĩnh (js, css) từ thư mục /web trên SD
    // Sử dụng serveStatic chuẩn để tránh lỗi 500 và tối ưu tốc độ
    server.serveStatic("/web", SD, "/web");

    server.begin();
}

void loop() {}