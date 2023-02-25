## TYWE3S

1. Copy platformio board `tywe3s.json` to `~/.platformio/platforms/espressif8266/boards`

2. Set `platformio.ini` with `board = tywe3s`

3. Config `data/led.json` with `{"pin":[12,0],"enable":1}`

4. Config `data/binary.json` with `{"input":[4,0],"output":[5,1],"output2":[14,0],"state":[0,0]}`
