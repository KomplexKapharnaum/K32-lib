# ESP32 framework for KXKM board

## TODO
- Use centrale queue (RTOS) with system dispatcher to stack and forward network commands
- Update Doc !
- Update Arduin example !

## INSTALL

### Dependencies
-CNMAT/OSC:                      https://github.com/CNMAT/OSC  
-me-no-dev/AsyncTCP:             https://github.com/me-no-dev/AsyncTCP  
-marvinroger/AsyncMQTTClient:    https://github.com/marvinroger/async-mqtt-client  
-me-no-dev/ESPAsyncWebServer:    https://github.com/me-no-dev/ESPAsyncWebServer  
-tommag/PCM51xx_Arduino:         https://github.com/tommag/PCM51xx_Arduino  
-Gianbacchio/ESP8266_Spiram:     https://github.com/Gianbacchio/ESP8266_Spiram  
-earlephilhower/ESP8266Audio:    https://github.com/earlephilhower/ESP8266Audio  
-marian-craciunescu/ESP32Ping:   https://github.com/marian-craciunescu/ESP32Ping


## MESSAGING protocol

**/path/engine/action + arguments**

### MQTT

**Subscriptions:**
    
    k32/all      = all device
    k32/c[chan]  = all device with channel [chan]
    k32/e[id]    = specific device with [id]

    NB: Arguments in payload must be separated by § symbol

### OSC:
    
**Path:**
    
    /all        = all device receiving the message
    /c[chan]    = all device in [chan]
    /e[id]      = specific device with [id]

### Common

**Engine/Action:**
    
    /reset          = reset device
    /shutdown       = turn off device
    /channel [int]  = set channel

    /audio
        /play [str] [int] [bool]         = play media: file (volume) (loop)
        /stop                            = stop
        /volume [int]                    = set volume
        /loop [bool]                     = set loop
        /noteon [int] [int] [int] [bool] = sampler play: bank note (volume) (loop)
        /noteoff [int]                   = sampler stop note if playing: note

    -- MQTT only (not implemented in OSC yet)

    /leds
        /master [int]   = set master @0-255
            /less       = set master -2
            /more       = set master +2
            /fadeout    = fadeout in 6 seconds
            /fadein     = fadein in 6 seconds

        /mem [int]      = set macro

        /stop           = all black
        /off            = all black
        /blackout       = all black

        /mod/[name]     = select modulator by name
        /modi/[index]   = select modulator by index
            /faster     = set selected mod faster
            /slower     = set selected mod slower
            /bigger     = set selected mod bigger (increase amplitude)
            /smaller    = set selected mod smaller (decrease amplitude)

        

    -- OSC only (not implemented in MQTT yet)

    /leds
        /all   [int] [int] [int] [int]               = all leds r(gbw)          
        /strip [int] [int] [int] [int] [int]         = set: strip r(gbw)        
        /pixel [int] [int] [int] [int] [int] [int]   = set: strip pixel r(gbw)  
        /play  [string] [int] [int] ... [int]        = play: anim param1 param2 ... param16
        /stop                                        = stop
        /blackout                                    = all black


    /osc
        /ping           = answer with pong       
        /info           = answer with status 

