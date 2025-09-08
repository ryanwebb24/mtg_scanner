# MTG Card Scanner with ESP32 + OCR

This project automates scanning **Magic: The Gathering (MTG)** cards using an **ESP32** and a connected camera. The ESP32 captures an image of the card and sends it to a server, where **OpenCV** and **OCR** are used to recognize the card details.

## Features
- 📷 Capture MTG card images with ESP32
- 📡 Send images over network/Tailscale to a server for processing 
- 🔎 Process images with OCR + OpenCV  
- 🃏 Identify and log card details automatically  

## Project Structure
- `UNKWN maybe esp32/` → Firmware for ESP32-CAM (image capture + upload)  
- `UNKWN maybe server/` → Python server for receiving images, running OCR/OpenCV, and returning card data  

## Requirements
- ESP32 CAM module  
- Python
- OpenCV, Tesseract OCR  

## Getting Started
1. Flash the ESP32 with the firmware in `UNKWN maybe esp32/`.  
2. Set up the server with dependencies: `pip install -r requirements.txt`
