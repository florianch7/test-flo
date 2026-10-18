"""
Simple drawing program using the Adafruit display library.
"""

import xpt2046_circuitpython
import time
import busio
import digitalio
from board import SCK, MOSI, MISO, CE0, D6, D22, D23, D24, D25 , D26

from adafruit_rgb_display import color565
import adafruit_rgb_display.ili9341 as ili9341


# Pin config
CS_PIN =CE0 
DC_PIN = D25
LED_PIN = D23
RST_PIN = D24

T_CS_PIN = D6
T_IRQ_PIN = D26

# Set up SPI bus using hardware SPI
spi = busio.SPI(clock=SCK, MOSI=MOSI, MISO=MISO)
# Turn on the LED backlight
led = digitalio.DigitalInOut(LED_PIN)
led.direction = digitalio.Direction.OUTPUT
led.value = True
# Create the ILI9341 display
display = ili9341.ILI9341(spi, cs=digitalio.DigitalInOut(CS_PIN),
                          dc=digitalio.DigitalInOut(DC_PIN), 
                          rst=digitalio.DigitalInOut(RST_PIN))
# Create touch controller
#  The Adafruit library changes the baudrate to 16M whenever it runs.
#  The touchscreen yields very inaccurate readings at this rate. We'll
#  bump it back down to 100K.
touch = xpt2046_circuitpython.Touch(
    spi, 
    cs = digitalio.DigitalInOut(T_CS_PIN),
    interrupt = digitalio.DigitalInOut(T_IRQ_PIN),
    force_baudrate = 100000
)

w = display.width
h = display.height
s = 4
s2 = 2

try:
    # Clear the screen
    display.fill(color565(0, 0, 0))
    display.fill_rectangle(0, 0, 20, 20, color565(255, 0, 0))
    display.fill_rectangle(w-20, 0, 20, 20, color565(0, 255, 0))
    display.fill_rectangle(0, h-20, 20, 20, color565(0, 0, 255))
    display.fill_rectangle(w-20, h-20, 20, 20, color565(255, 255, 255))
    time.sleep(0.1) 
    
    while True:
        # Check if we have an interrupt signal
        if touch.is_pressed():
            # Get the coordinates for this touch.
            try:
                x, y = touch.get_coordinates()
                # The Y reading is flipped to what the Adafruit library expects.
                # y = touch.height - y
                x = touch.width - x 
                print(x,y)

                display.fill_rectangle(max(0, x - s2), max(0, y - s2), s, s, color565(255, 255, 255))
            except xpt2046_circuitpython.exceptions.ReadFailedException:
                pass
        else:
            # Delay for a bit
            time.sleep(0.1)

except KeyboardInterrupt:
    led.value = False

