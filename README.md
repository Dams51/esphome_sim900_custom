# ESPHome component for SIM900

This component use ESPHome UART to connect with SIM900 with AT commands and communicates directly with Home Assistant.

### Supported SIM modules

Only tested on one specific board

## Hardware

Tested with wemos D1 mini

## Installation

Add this to your configuratino file :

```yaml
...
esphome:
  libraries:
    - "PDUlib"

external_components:
  - source: 
      type: git
      url: https://github.com/Dams51/esphome_sim900_custom
    components: [sim900]

uart:
  id: uart_bus        # Optional.
  baud_rate: 9600
  tx_pin: TX
  rx_pin: RX

sim900:
  id: gsm_module      # Optional.
  uart_id: uart_bus   # Optional.

sensor:
  - platform: sim900
    signal_reseau:
      name: "Sim900 Signal réseau"

binary_sensor:
  - platform: sim900
    etat_reseau:
      name: "Sim900 Etat réseau"

text_sensor:
  - platform: sim900
    etat_module:
      name: "Sim900 Etat du module"

gpio_switch:
  - platform: sim900
    power_key_pin:
      pin: D3
...
```
