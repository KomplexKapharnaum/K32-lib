# ESP32 framework for KXKM board

## MESSAGING protocol

**/path/engine/action + arguments**

### OSC:
    
**Path:**
    
    /all        = all device receiving the message
    /c[chan]    = all device in [chan]
    /d[addr]    = all device in dmx [addr]
    /e[id]       = specific device with [id]


### MQTT

**Subscriptions:**
    
    k32/all      = all device
    k32/c[chan]  = all device in [chan]
    k32/d[addr]  = all device in dmx [addr]
    k32/e[id]     = specific device with [id]

**Engine/Action:**

    /osc
        /osc/ping           = answer with pong       
        /osc/info           = answer with status 

    /sys
        /sys/off            = turn off device
        /sys/reset          = reset device
        /sys/chan [int]     = set channel
        /sys/dmx  [int]     = set dmx addr 

    /audio
        /audio/play [str] [int] [bool]  = play media (volume) (loop)
        /audio/stop                     = stop
        /audio/volume [int]             = set volume
        /audio/loop [bool]              = set loop
        /audio/noteon [int] [int] [int] = sampler play bank note (volume)
        /audio/noteoff [int]            = sampler stop note if playing

    /leds
        /leds/all   [int] [int] [int] [int]               = leds r(gbw)
        /leds/strip [int] [int] [int] [int] [int]         = leds strip r(gbw)
        /leds/pixel [int] [int] [int] [int] [int] [int]   = led strip pixel r(gbw)
        /leds/play  [string] [int] [int] ... [int]        = play anim param1 param2 ... param16
        /leds/stop                                        = stop
        /leds/blackout                                    = all black