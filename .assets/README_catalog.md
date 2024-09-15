# Flipper Game Boy Printer

Emulate a Game Boy Printer with your Flipper, files printed to the Flipper are saved as PNGs. Additionally the raw binary of the image content is saved as well.

## Connection: Flipper Zero GPIO - Game Boy

See the project's README on GitHub https://github.com/kbembedded/flipper-gb-printer/blob/main/README.md#hardware-interface for Link Cable interface adapters available on Tindie.

The pinout is as follows:

| Cable Game Link (Socket) | Flipper Zero GPIO |
| ------------------------ | ----------------- |
| 6 (GND)                  | 8 (GND)           |
| 5 (CLK)                  | 6 (B2)            |
| 3 (SI)                   | 7 (C3)            |
| 2 (SO)                   | 5 (B3)            |

This matches the Original pinout used on a number of other Flipper applications supporting Game Boy Link interface.


## Tested to Support Game Boy models
- Game Boy Color (GBC)
- Game Boy Advance (GBA)
- Game Boy Advance SP (GBA SP)
- Analogue Pocket
