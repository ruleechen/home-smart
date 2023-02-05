## TYWE3S

1. Copy platformio board `tywe3s.json` to `~/.platformio/platforms/espressif8266/boards`

2. Set `platformio.ini` with `board = tywe3s`

3. Config `data/app.json` with `{"name":"RuleeSmart","sleep":200,"led":[12,0,1]}`

4. Config `data/switch.json` with `{"input":[4,0],"output":[5,1],"output2":[14,0],"state":[0,0]}`
