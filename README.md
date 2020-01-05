# weak-web222-led
RGB LED controlled via ESP8266-based Web Server 

* web server based on ESP8266
* using static private IP address, useful for port forwarding from WLAN router
* home page `/index.html` intentionally without function
* interaction via `/help.html`
* additional feature: logging request resulting in 404 error, see `/logg` 

hardware: 
  - wemos d1 mini with rgb led shield
    * https://wiki.wemos.cc/products:d1:d1_mini
    * https://wiki.wemos.cc/products:d1_mini_shields:rgb_led_shield
  
software: 
  - Arduino IDE
    * additional data upload for static web server files, see https://github.com/esp8266/arduino-esp8266fs-plugin
    
setup:
  - clone repository
  - set up the Arduino IDE
  - edit `weak-web222-led.ino`
    * set wifi credentials in [line 101](/weak-web222-led.ino#L101)
    * set your static ip for local network in [line 253](/weak-web222-led.ino#L253)
    
