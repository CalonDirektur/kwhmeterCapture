#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "TELPRO-7Heroe";
const char* password = "Telpro070707";

// Initialize Telegram BOT
String BOTtoken = "7762900310:AAGQ6JgCSGaLgFKmRL0CsDGt_SK--HY29FM";  // your Bot Token (Get from Botfather)
String CHAT_ID = "458535076";

bool sendPhoto = false;

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4
bool flashState = LOW;

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void configInitCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        config.frame_size = FRAMESIZE_VGA; // Gunakan ukuran frame yang lebih kecil
        config.jpeg_quality = 10;  // 0-63 lower number means higher quality
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA; // 800x600
        config.jpeg_quality = 12;  // 0-63 lower number means higher quality
        config.fb_count = 1;
    }

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        delay(1000);
        ESP.restart();
    }
}

void handleNewMessages(int numNewMessages) {
    Serial.print("Handle New Messages: ");
    Serial.println(numNewMessages);

    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        if (chat_id != CHAT_ID) {
            bot.sendMessage(chat_id, "Unauthorized user", "");
            continue;
        }

        // Print the received message
        String text = bot.messages[i].text;
        Serial.println(text);

        String from_name = bot.messages[i].from_name;
        if (text == "/start") {
            String welcome = "Welcome, " + from_name + "\n";
            welcome += "Use the following commands to interact with the ESP32-CAM \n";
            welcome += "/photo : takes a new photo\n";
            welcome += "/flash : toggles flash LED \n";
            bot.sendMessage(CHAT_ID, welcome, "");
        }
        if (text == "/flash") {
            flashState = !flashState;
            digitalWrite(FLASH_LED_PIN, flashState);
            Serial.println("Change flash LED state");
        }
        if (text == "/photo") {
            sendPhoto = true; // Set flag to send photo
            Serial.println("New photo request");
        }
    }
}

String sendPhotoTelegram() {
    const char* myDomain = "api.telegram.org";
    String getAll = "";
    String getBody = "";

    camera_fb_t * fb = esp_camera_fb_get();  
    if (!fb) {
        Serial.println("Camera capture failed");
        return "Camera capture failed";
    }  

    Serial.println("Connecting to " + String(myDomain));

    if (clientTCP.connect(myDomain, 443)) {
        Serial.println("Connection successful");

        // Define the boundary
        String boundary = "boundary123456789";
        String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"chat_id\"\r\n\r\n" + CHAT_ID + "\r\n--" + boundary + "\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
        String tail = "\r\n--" + boundary + "--\r\n";

        uint16_t imageLen = fb->len;
        uint16_t extraLen = head.length() + tail.length();
        uint16_t totalLen = imageLen + extraLen;

        // Kirim HTTP header
        clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
        clientTCP.println("Host: " + String(myDomain));
        clientTCP.println("Content-Length: " + String(totalLen));
        clientTCP.println("Content-Type: multipart/form-data; boundary=" + boundary);
        clientTCP.println();
        clientTCP.print(head);

        // Kirim data gambar
        uint8_t *fbBuf = fb->buf;
        size_t fbLen = fb->len;
        for (size_t n = 0; n < fbLen; n += 1024) {
            if (n + 1024 < fbLen) {
                clientTCP.write(fbBuf, 1024);
                fbBuf += 1024;
            } else if (fbLen % 1024 > 0) {
                size_t remainder = fbLen % 1024;
                clientTCP.write(fbBuf, remainder);
            }
        }

        // Kirim tail
        clientTCP.print(tail);

        esp_camera_fb_return(fb); // Kembalikan buffer frame

        int waitTime = 500;  // Mengurangi waktu tunggu
        long startTimer = millis();
        boolean state = false;

        while ((startTimer + waitTime) > millis()) {
            while (clientTCP.available()) {
                char c = clientTCP.read();
                if (state == true) getBody += String(c);
                if (c == '\n') {
                    if (getAll.length() == 0) state = true;
                    getAll = "";
                } else if (c != '\r')
                    getAll += String(c);
                startTimer = millis();
            }
            if (getBody.length() > 0) break;
        }
        clientTCP.stop();
        Serial.println(getBody);
    } else {
        getBody = "Failed to connect to Telegram server.";
        Serial.println("Connection failed.");
    }
    return getBody;
}

String sendPhotoToServer() {
    const char* serverUrl = "http://36.66.242.11/tesseract-main/upload.php";
    String getAll = "";
    String getBody = "";

    camera_fb_t * fb = esp_camera_fb_get();  
    if (!fb) {
        Serial.println("Camera capture failed");
        return "Camera capture failed";
    }  

    Serial.println("Connecting to server: " + String(serverUrl));

    WiFiClient client;
    if (client.connect("36.66.242.11", 80)) {
        Serial.println("Connection to server successful");

        // Define the boundary
        String boundary = "boundary123456789";
        String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"image\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
        String tail = "\r\n--" + boundary + "--\r\n";

        uint16_t imageLen = fb->len;
        uint16_t extraLen = head.length() + tail.length();
        uint16_t totalLen = imageLen + extraLen;

        // Kirim HTTP header
        client.println("POST /tesseract-main/upload.php HTTP/1.1");
        client.println("Host: 36.66.242.11");
        client.println("Content-Length: " + String(totalLen));
        client.println("Content-Type: multipart/form-data; boundary=" + boundary);
        client.println();

        // Kirim header
        client.print(head);

        // Kirim data gambar
        uint8_t *fbBuf = fb->buf;
        size_t fbLen = fb->len;
        for (size_t n = 0; n < fbLen; n += 1024) {
            if (n + 1024 < fbLen) {
                client.write(fbBuf, 1024);
                fbBuf += 1024;
            } else if (fbLen % 1024 > 0) {
                size_t remainder = fbLen % 1024;
                client.write(fbBuf, remainder);
            }
        }

        // Kirim tail
        client.print(tail);

        // Tunggu respons dari server
        while (client.connected() || client.available()) {
            if (client.available()) {
                char c = client.read();
                getAll += String(c);
            }
        }
        client.stop();
        Serial.println("Response from server: " + getAll);
    } else {
        Serial.println("Failed to connect to server.");
    }
    esp_camera_fb_return(fb); // Kembalikan buffer frame
    return getAll;
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
    // Init Serial Monitor
    Serial.begin(115200);

    // Set LED Flash as output
    pinMode(FLASH_LED_PIN, OUTPUT);
    digitalWrite(FLASH_LED_PIN, flashState);

    // Config and init the camera
    configInitCamera();

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("ESP32-CAM IP Address: ");
    Serial.println(WiFi.localIP()); 
}

void loop() {
    if (sendPhoto) {
        Serial.println("Preparing photo");
        String responseTelegram = sendPhotoTelegram(); 
        Serial.println("Response from Telegram: " + responseTelegram);
        
        String responseServer = sendPhotoToServer();
        Serial.println("Response from Server: " + responseServer);
        
        sendPhoto = false; 
    }
    if (millis() > lastTimeBotRan + botRequestDelay)  {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages) {
            Serial.println("got response");
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
}
