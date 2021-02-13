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

    NB: Arguments are packed as strings in payload using § as separator


### OSC base path:
    
    /all        = all device receiving the message
    /c[chan]    = all device in [chan]
    /e[id]      = specific device with [id]

    NB: Arguments can be either string or integer


### Common (extends base path/topic)

**Engine/Action:**
    
    /system
        /reset          = reset device
        /shutdown       = turn off device
        /channel [int]  = set channel


    /audio
        /play [str] [int] [bool]         = play media: file (volume) (loop)
        /stop                            = stop
        /volume [int]                    = set volume: vol
        /loop [bool]                     = set loop: (0|1)
        /unloop                          = set unloop
        /noteon [int] [int] [int] [bool] = sampler play: bank note (volume) (loop)
        /noteoff [int]                   = sampler stop note if playing: note
        /midi [int] [int] [int]          = raw midi: event note velocity


    /leds
        /all   [int] [int] [int] [int]               = all leds r(gbw)          
        /strip [int] [int] [int] [int] [int]         = set: strip r(gbw)        
        /pixel [int] [int] [int] [int] [int] [int]   = set: strip pixel r(gbw)  

        /master [int]   = set master @0-255
            /full       = set master @255
            /less       = set master -2
            /more       = set master +2
            /tenless    = set master -10
            /tenmore    = set master +10
            /fadeout    = fadeout in 6 seconds
            /fadein     = fadein in 6 seconds

        /mem [int] [int]      = play: macro (master)

        /stop           = all black
        /off            = all black
        /blackout       = all black

        /mod  [name]    = select modulator by name (name is an argument)
        /modi [index]   = select modulator by index (index is an argument)
            /faster     = set selected mod faster
            /slower     = set selected mod slower
            /bigger     = set selected mod bigger (increase amplitude)
            /smaller    = set selected mod smaller (decrease amplitude)

        

    -- OSC only

    /osc
        /ping           = answer with pong       
        /info           = answer with status 

