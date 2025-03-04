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
    components: [Sim900Component]

uart:
  id: uart_bus        # Optional.
  baud_rate: 9600
  tx_pin: TX
  rx_pin: RX

Sim900Component:
  id: gsm_module      # Optional.
  uart_id: uart_bus   # Optional.
  power_key_switch_id: power_key_switch # Optional.

sensor:
  - platform: Sim900Component
    signal_reseau:
      name: "Sim900 Signal réseau"

binary_sensor:
  - platform: Sim900Component
    etat_reseau:
      name: "Sim900 Etat réseau"

text_sensor:
  - platform: Sim900Component
    etat_module:
      name: "Sim900 Etat du module"

switch:
  - platform: gpio
    pin: D3
    id: power_key_switch
...
```
