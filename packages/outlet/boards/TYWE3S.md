## TYWE3S

### Setup compile
1. Copy platformio board `tywe3s.json` to `~/.platformio/platforms/espressif8266/boards`
2. Set `platformio.ini` with `board = tywe3s`

### Setup runtime configuration
1. Config `data/led.json` with `{"pin":[12,0],"enable":1}`
2. Config `data/button.json` with `{"pin":[4,0],"enable":1}`
3. Config `data/output.json` with `{"pin":[5,1],"enable":1}`
4. Config `data/output2.json` with `{"pin":[14,1],"enable":1}`
5. Config `data/state.json` with `{"save":0,"isOn":0}`
