I wanted a small cyberdeck I could keep on a keychain and use while working on projects, in a workshop. Ibntroducing the new greates tempo lab assistant: Sprout, a solarpunk themed board with a weather screen, buttons, and a headphone connection. The assistant runs on my laptop because I wanted to keep the device small, eventually I want to move it onto a pi and have it be native.

The PCB is a 60 by 50 mm carrier for a Seeed XIAO ESP32-S3, an Adafruit 4440 OLED, and an Adafruit 6309 headphone DAC breakout. It has leaves on the silkscreen and a sun around the keyring hole. The DAC connects with a wire harness and needs its own mount, so youy can put in headphones suring the event.

## Instructions
Download solar-cyber.zip from this repo and unzip it. The complete project is inside, with the folders kept together. Open Sprout.kicad_pro in KiCad to work on the board(Let me kn ow if you have any advice as this was my first board). The fabrication folder has the Gerbers and drill files, and BOM.csv has the parts list. The software folder has the device code, laptop bridge, simulator, and setup instructions!

To run the laptop side, install Python 3.10 or newer and Ollama. Run ollama pull qwen3:1.7b, then open a terminal in the software folder and run python3 start_workshop.py. Weather defaults to wherever you live!! in Fahrenheit and mph. The workshop assistant uses the local model. Speech uses macOS say and afconvert.

For the device, copy Config.example.h to Config.h in software/firmware/Sprout and add your Wi-Fi details, laptop address, and bridge token. Run the laptop bridge with lan so the keychain can connect on your local Wi-Fi.(eventually want to add individual network to connect to computer. Keep Config.h and the generated token private. Open Sprout.ino in Arduino IDE with the XIAO ESP32S3 board selected, install the libraries listed in the software README, and rebuild before uploading. The included binaries use placeholder settings.

The simulator files are in software/simulator. Copy sketch.ino, diagram.json, and libraries.txt into a new ESP32-S3 project on Wokwi and start it. It tests the screen and buttons using labelled demo weather and answers. It does not simulate the battery, physical PCB copper, or headphone electronics(cus i could not figurw them out online).

The PCB passed the saved KiCad DRC and ERC checks. The firmware compiled, 19 core checks and 13 bridge tests passed, and the simulator was tested.

The board render is below. The editable board, schematic, and a copy of the firmware source are also available directly in this repo. Use the ZIP for the complete folder layout and custom footprint library. bom.csv is the USB prototype budget estimate; it substitutes a SparkFun OLED, assumes some existing workshop supplies, and still needs exact small-part and delivery quotes. 

## Bill of materials (BOM)

<img width="1046" height="918" alt="Screenshot 2026-10-18 at 10 43 14 PM" src="https://github.com/user-attachments/assets/e31f6877-b4d6-4811-93a9-e870c408573c" />

| Part | What it's for | Qty | Unit | Total | Vendor |
| --- | --- | ---: | ---: | ---: | --- |
| Seeed XIAO ESP32S3 standard with antenna | Existing carrier pinout; USB power | 1 | $7.49 | $7.49 | [Seeed Studio](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html) |
| SparkFun Qwiic OLED 0.91 inch 128x32 LCD-24606 | SSD1306 128x32 at 0x3C; external 44.45x12.7mm display; adjust mounting | 1 | $9.95 | $9.95 | [SparkFun](https://www.sparkfun.com/sparkfun-qwiic-oled-display-0-91-in-128x32-lcd-24606.html) |
| Adafruit TLV320DAC3100 headphone DAC breakout 6309 | Use existing J1 eight-wire harness; headphone jack included | 1 | $6.95 | $6.95 | [Adafruit #6309](https://www.adafruit.com/product/6309) |
| SPROUT 60x50mm two-layer 1.6mm green FR4 HASL bare boards | Full five-board minimum-order cost; build one unit; no assembly or ENIG; delivery excluded | 5 | $0.40 | $2.00 | [JLCPCB fabrication quote](https://cart.jlcpcb.com/quote) |
| STEMMA QT / Qwiic 50mm four-pin cable | Connect J2 to external OLED | 1 | $0.95 | $0.95 | [Adafruit #4399](https://www.adafruit.com/product/4399) |
| 6mm four-leg THT momentary switches | Match existing 6.5x4.5mm pin spacing before purchase | 3 | $0.20 | $0.60 | [LCSC C18078028](https://www.lcsc.com/product-detail/C18078028.html) |
| 1x7 2.54mm female sockets | 15.24mm row spacing on carrier; use included XIAO male headers if supplied | 2 | $0.33 | $0.66 | [LCSC C22438157](https://www.lcsc.com/product-detail/C22438157.html) |
| JST SH SM04B-SRSS-TB horizontal four-pin connector | Must match saved J2 footprint | 1 | $0.35 | $0.35 | [LCSC C160404](https://www.lcsc.com/product-detail/C160404.html) |
| 0805 10uF 10V X5R and 100nF 16V capacitors | Populate both decoupling capacitors | 1 | $0.20 | $0.20 | [LCSC C440198 (C1)](https://www.lcsc.com/product-detail/C440198.html), [C49678 (C2)](https://www.lcsc.com/product-detail/C49678.html) |
| Eight insulated DAC wires and insulating stand-offs | If not available add actual order cost; reserve may be insufficient | 1 | $0.00 | $0.00 | Existing workshop supplies; no purchase requested |
| USB-C data cable; USB power; headphones; existing laptop | No purchase requested for these; model runs on laptop | 1 | $0.00 | $0.00 | Existing equipment; no purchase requested |
| Battery connectors and battery switch | USB-only prototype; leave battery section unpopulated; no battery or solar charging included | 1 | $0.00 | $0.00 | Not populated in USB prototype; no purchase requested |
| Parts subtotal | — | — | — | $29.15 | — |
| Tax & shipping | — | — | — | $0.00 | — |
| Total | — | — | — | $29.15 | — |


![SPROUT solarpunk PCB render](board-preview.png)
