#include <AccelStepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

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
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Web server on port 80
WebServer server(80);

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

// ===== Web Handler =====
void handleRunStepper() {
    oledPrint("HTTP Trigger!");
    runStepper(1024, 300);
    server.send(200, "text/plain", "Stepper executed!");
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

    // WiFi
    WiFi.begin(ssid, password);
    oledPrint("Connecting WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    oledPrint("IP:", 0, 0, 2);
    oledPrint(WiFi.localIP().toString().c_str(), 0, 20, 1, false);

    // Web server
    server.on("/runStepper", handleRunStepper);
    server.begin();
}

// ===== Loop =====
void loop() {
    server.handleClient();

    // Button press also triggers stepper
    if (digitalRead(buttonPin) == LOW) {
        oledPrint("Button Pushed!");
        runStepper(1024, 300);
        delay(500);  // debounce
    }
}
