## BOOTSTRAPPING

- pip --version
- pip install --upgrade pip
- pip install platformio
- pio pkg update
- pip install esphome

## PLATFORMIO (pio)

Here are some commands I used for setting up Platformio CLI
```
platformio --version or pio --version
pio run --target clean
platformio lib --global update
platformio run --target clean --verbose
platformio run --target build --verbose
platformio run --target upload --verbose
platformio run --verbose
```

## ESPHOME

### Install esphome CLI
```
pip install esphome
```
### Check esphome version
```
esphome --version
```
### Upload ESPHome configuration to board and run it
Note: this isn't a permanent update to the on-board yaml file, since this command does not upload the yaml file to the `esphome/config/` directory on the Docker host.

```
esphome run esphome_blink_led.yaml
```