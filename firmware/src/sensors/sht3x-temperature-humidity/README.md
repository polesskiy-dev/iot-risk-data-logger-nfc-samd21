## Communucation description

```sequence
MCU->SHT3x: Read Status Register (self test)
SHT3x->MCU: Status
MCU->SHT3x: Start Measurements (Single Shot) command
MCU->MCU: Wait 15ms
MCU->SHT3x: Read Measurements
SHT3x->MCU: Measurements
```