#include <AccelStepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

// ==== WIFI Steup =====
#ifndef WIFI_SSID
#define WIFI_SSID "default_ssid"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "default_pass"
#endif

// ===== OLED Setup =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Button Setup =====
const int buttonPin = 15;

// ===== Stepper Setup =====
#define IN1 16
#define IN2 17
#define IN3 18
#define IN4 19
AccelStepper stepper(AccelStepper::FULL4WIRE, IN1, IN3, IN2, IN4);

// ===== WiFi Setup =====
String wifiSSID = "";
String wifiPassword = "";
bool wifiConfigured = false;
bool wifiConnected = false;
bool apMode = true;  // Start in Access Point mode

// AP Mode settings - Generate random setup password
const char* ap_ssid = "MTG_Scanner_Setup";
String ap_password = "";
String device_token = "";

// Security settings
unsigned long apStartTime = 0;
const unsigned long AP_TIMEOUT = 300000;     // 5 minutes
const unsigned long SETUP_TIMEOUT = 600000;  // 10 minutes total
bool setupComplete = false;
unsigned long lastConnectAttempt = 0;
int connectAttempts = 0;
const int MAX_CONNECT_ATTEMPTS = 3;

// Web server on port 80
WebServer server(80);

// DNS server for captive portal
DNSServer dnsServer;
const byte DNS_PORT = 53;

// ===== Security Helper Functions =====
String generateRandomPassword(int length = 8) {
    String chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    String password = "";
    for (int i = 0; i < length; i++) {
        password += chars[random(chars.length())];
    }
    return password;
}

String generateDeviceToken() {
    return generateRandomPassword(12);
}

bool isValidToken(String token) {
    return token.equals(device_token) && token.length() > 0;
}

// Forward declaration
void oledPrint(const char* text, int x, int y, int size, bool clear);

void checkSetupTimeout() {
    if (apMode && millis() - apStartTime > SETUP_TIMEOUT) {
        Serial.println("Setup timeout - rebooting to secure mode");
        oledPrint("Setup Timeout", 0, 0, 2, true);
        oledPrint("Rebooting...", 0, 20, 1, false);
        delay(2000);
        ESP.restart();
    }
}

// ===== OLED Helper =====
void oledPrint(const char* text, int x = 0, int y = 0, int size = 2, bool clear = true) {
    if (clear) {
        display.clearDisplay();
    }
    display.setTextSize(size);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(x, y);
    display.println(text);
    display.display();
}

// ===== Stepper Function =====
void runStepper(int steps = 1024, int speed = 300) {
    stepper.setMaxSpeed(speed);
    stepper.setSpeed(speed);

    oledPrint("Stepper Run!");

    for (int i = 0; i < steps; i++) {
        stepper.runSpeed();
        delay(2);
    }

    oledPrint("Done!");
}

// ===== WiFi Connection Function =====
void connectToWiFi() {
    if (wifiSSID.length() == 0) {
        Serial.println("No WiFi credentials configured");
        return;
    }

    oledPrint("Connecting...", 0, 0, 2);
    oledPrint(wifiSSID.c_str(), 0, 20, 1, false);

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        apMode = false;
        setupComplete = true;

        // Disable AP mode for security
        WiFi.mode(WIFI_STA);

        Serial.println("\n🔒 WiFi connected securely!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.println("Device Token: " + device_token);
        Serial.println("⚠️ Setup mode disabled - use token for access");

        oledPrint("🔒 Connected!", 0, 0, 2);
        oledPrint(WiFi.localIP().toString().c_str(), 0, 20, 1, false);
        oledPrint("Token required", 0, 40, 1, false);
    } else {
        wifiConnected = false;
        Serial.println("\nWiFi connection failed!");
        oledPrint("WiFi Failed!", 0, 0, 2);
        oledPrint("Check settings", 0, 20, 1, false);

        // Return to AP mode after failure
        delay(3000);
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ap_ssid, ap_password);
        oledPrint("WiFi Setup", 0, 0, 2);
        oledPrint(ap_ssid, 0, 20, 1, false);
        oledPrint(WiFi.softAPIP().toString().c_str(), 0, 40, 1, false);
    }
}

// ===== Web Configuration Page =====
void handleRoot() {
    // Check for timeout
    checkSetupTimeout();

    String html = "<!DOCTYPE html><html><head><title>MTG Scanner Setup</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0}";
    html += ".container{max-width:400px;margin:auto;background:white;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}";
    html += ".warning{background:#fff3cd;border:1px solid #ffeaa7;padding:10px;margin:10px 0;border-radius:4px}";
    html += "input,button{width:100%;padding:12px;margin:8px 0;border:1px solid #ddd;border-radius:4px;box-sizing:border-box}";
    html += "button{background:#007cba;color:white;border:none;cursor:pointer}";
    html += "button:hover{background:#005a8a}</style></head><body>";
    html += "<div class='container'><h2>MTG Scanner WiFi Setup</h2>";

    if (apMode) {
        unsigned long remainingTime = (SETUP_TIMEOUT - (millis() - apStartTime)) / 1000;
        html += "<div class='warning'>⚠️ Setup expires in " + String(remainingTime / 60) + ":" + String(remainingTime % 60, DEC) + "</div>";
        html += "<p>🔒 Secure one-time setup. Connect to your WiFi:</p>";
        html += "<form action='/connect' method='POST'>";
        html += "<input type='hidden' name='token' value='" + device_token + "'>";
        html += "<input type='text' name='ssid' placeholder='WiFi Network Name' required><br>";
        html += "<input type='password' name='password' placeholder='WiFi Password' required><br>";
        html += "<button type='submit'>🔗 Connect to WiFi</button></form>";
        html += "<br><button onclick='location.href=\"/stepper?token=" + device_token + "\"'>🧪 Test Stepper</button>";
    } else {
        html += "<p>✅ Connected to: " + wifiSSID + "</p>";
        html += "<p>📍 IP: " + WiFi.localIP().toString() + "</p>";
        html += "<p>🔒 Setup mode disabled for security</p>";
        html += "<form action='/stepper' method='POST'>";
        html += "<input type='password' name='token' placeholder='Device Token' required><br>";
        html += "<button type='submit'>🎯 Run Stepper</button></form>";
    }

    html += "</div></body></html>";
    server.send(200, "text/html", html);
}

void handleConnect() {
    // Rate limiting
    if (millis() - lastConnectAttempt < 10000 && connectAttempts >= MAX_CONNECT_ATTEMPTS) {
        server.send(429, "text/html", "<html><body><h2>Too Many Attempts</h2><p>Wait 10 seconds before trying again</p></body></html>");
        return;
    }

    // Check token
    String token = server.arg("token");
    if (!isValidToken(token)) {
        server.send(401, "text/html", "<html><body><h2>Unauthorized</h2><p>Invalid token</p></body></html>");
        return;
    }

    wifiSSID = server.arg("ssid");
    wifiPassword = server.arg("password");

    if (wifiSSID.length() == 0) {
        server.send(400, "text/html", "<html><body><h2>Error</h2><p>WiFi name required</p></body></html>");
        return;
    }

    connectAttempts++;
    lastConnectAttempt = millis();

    server.send(200, "text/html", "<html><body><h2>🔐 Connecting Securely...</h2><p>Connecting to " + wifiSSID + "</p><script>setTimeout(function(){window.location='/';},5000);</script></body></html>");

    connectToWiFi();
}  // Captive Portal - redirect all requests to our setup page
void handleNotFound() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
}

// Handle all captive portal requests
void handleCaptivePortal() {
    // Check if the request is for our setup page
    if (server.hostHeader() != "192.168.4.1") {
        server.sendHeader("Location", "http://192.168.4.1/", true);
        server.send(302, "text/plain", "");
        return;
    }
    handleRoot();
}
void handleScan() {
    String html = "<!DOCTYPE html><html><head><title>WiFi Networks</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0}";
    html += ".container{max-width:400px;margin:auto;background:white;padding:20px;border-radius:8px}";
    html += ".network{padding:10px;border:1px solid #ddd;margin:5px 0;cursor:pointer;border-radius:4px}";
    html += ".network:hover{background:#f5f5f5}</style></head><body>";
    html += "<div class='container'><h2>Available Networks</h2>";

    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        html += "<div class='network' onclick='selectNetwork(\"" + WiFi.SSID(i) + "\")'>;";
        html += WiFi.SSID(i) + " (" + WiFi.RSSI(i) + "dBm)</div>";
    }

    html += "<br><a href='/'>Back to Setup</a>";
    html += "<script>function selectNetwork(ssid){document.location='/?ssid='+encodeURIComponent(ssid);}</script>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
}

void handleReset() {
    apMode = true;
    wifiConnected = false;
    server.send(200, "text/html", "<html><body><h2>WiFi Reset</h2><p>Device returning to setup mode...</p><script>setTimeout(function(){window.location='/';},3000);</script></body></html>");
    delay(1000);
    ESP.restart();
}

// ===== Web Handler =====
void handleRunStepper() {
    String token = "";

    // Check token from GET or POST
    if (server.method() == HTTP_GET) {
        token = server.arg("token");
    } else {
        token = server.arg("token");
    }

    // In setup mode, allow with device token. In normal mode, require secure token
    bool authorized = false;
    if (apMode && isValidToken(token)) {
        authorized = true;
    } else if (!apMode && token.equals(device_token)) {
        authorized = true;
    }

    if (!authorized) {
        server.send(401, "text/html", "<html><body><h2>🔒 Unauthorized</h2><p>Invalid or missing token</p><a href='/'>Back to Home</a></body></html>");
        return;
    }

    oledPrint("HTTP Trigger!");
    runStepper(1024, 300);

    if (server.method() == HTTP_GET) {
        server.send(200, "text/html", "<html><body><h2>✅ Stepper Executed!</h2><p>Motor test complete</p><a href='/?token=" + device_token + "'>Back to Setup</a></body></html>");
    } else {
        server.send(200, "text/html", "<html><body><h2>✅ Stepper Executed!</h2><p>Motor run complete</p><a href='/'>Back to Home</a></body></html>");
    }
}

// ===== Setup =====
void setup() {
    Serial.begin(115200);

    // OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    // Button
    pinMode(buttonPin, INPUT_PULLUP);

    // Stepper
    stepper.setMaxSpeed(500);
    stepper.setSpeed(300);

    // Generate secure random passwords
    randomSeed(analogRead(0) + millis());
    ap_password = generateRandomPassword(8);
    device_token = generateDeviceToken();
    apStartTime = millis();

    // Start in WiFi Access Point mode for secure setup
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password.c_str());

    Serial.println("🔒 SECURE WiFi Access Point started!");
    Serial.println("Connect to WiFi: MTG_Scanner_Setup");
    Serial.println("Password: " + ap_password);
    Serial.println("Device Token: " + device_token);
    Serial.print("Setup URL: http://");
    Serial.println(WiFi.softAPIP());
    Serial.println("⏰ Setup expires in 10 minutes");

    oledPrint("SECURE Setup", 0, 0, 2);
    String passDisplay = "Pass:" + ap_password;
    oledPrint(passDisplay.c_str(), 0, 20, 1, false);
    oledPrint("10min timeout", 0, 40, 1, false);  // Start DNS server for captive portal
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // Web server setup with captive portal
    server.on("/", handleCaptivePortal);
    server.on("/connect", HTTP_POST, handleConnect);
    server.on("/stepper", handleRunStepper);
    server.on("/runStepper", handleRunStepper);
    server.onNotFound(handleNotFound);  // Redirect all other requests
    server.begin();
    Serial.println("Device ready! Connect to WiFi and visit http://");
    Serial.println(WiFi.softAPIP().toString());
}

// ===== Loop =====
void loop() {
    // Handle DNS server for captive portal
    dnsServer.processNextRequest();

    // Handle web server
    server.handleClient();  // Handle web server if WiFi is connected
    if (wifiConnected) {
        server.handleClient();
    }

    // Button press also triggers stepper
    if (digitalRead(buttonPin) == LOW) {
        oledPrint("Button Pushed!");
        runStepper(1024, 300);

        // Button pressed - stepper executed
        Serial.println("Button pressed - stepper executed!");

        delay(500);  // debounce
    }
}
