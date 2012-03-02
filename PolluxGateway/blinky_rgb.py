import time

leds = [{ 'name' : 'lcd_data2', 'gpio' : '72' },
	{ 'name' : 'lcd_data4', 'gpio' : '74' },
	{ 'name' : 'lcd_data6', 'gpio' : '76' }]

for led in leds:
   open('/sys/kernel/debug/omap_mux/%s' % led['name'], 'wb').write("%X" % 7)
   try:
      # check to see if the pin is already exported
      open('/sys/class/gpio/gpio%s/direction' % led['gpio']).read()
   except:
      # it isn't, so export it
      print("exporting GPIO %s" % led['gpio'])
      open('/sys/class/gpio/export', 'w').write(led['gpio'])
 
   # set Port and Pin for output
   open('/sys/class/gpio/gpio%s/direction' % led['gpio'], 'w').write('out')

### 

open('/sys/class/gpio/gpio%s/value' % leds[0]['gpio'], 'w').write("1")
time.sleep(1)
open('/sys/class/gpio/gpio%s/value' % leds[0]['gpio'], 'w').write("0")

open('/sys/class/gpio/gpio%s/value' % leds[1]['gpio'], 'w').write("1")
time.sleep(1)
open('/sys/class/gpio/gpio%s/value' % leds[1]['gpio'], 'w').write("0")

open('/sys/class/gpio/gpio%s/value' % leds[2]['gpio'], 'w').write("1")
time.sleep(1)
open('/sys/class/gpio/gpio%s/value' % leds[2]['gpio'], 'w').write("0")

###

open('/sys/class/gpio/gpio%s/value' % leds[0]['gpio'], 'w').write("1")
open('/sys/class/gpio/gpio%s/value' % leds[1]['gpio'], 'w').write("1")
time.sleep(1)
open('/sys/class/gpio/gpio%s/value' % leds[0]['gpio'], 'w').write("0")
open('/sys/class/gpio/gpio%s/value' % leds[1]['gpio'], 'w').write("0")

open('/sys/class/gpio/gpio%s/value' % leds[0]['gpio'], 'w').write("1")
open('/sys/class/gpio/gpio%s/value' % leds[2]['gpio'], 'w').write("1")
time.sleep(1)
open('/sys/class/gpio/gpio%s/value' % leds[0]['gpio'], 'w').write("0")
open('/sys/class/gpio/gpio%s/value' % leds[2]['gpio'], 'w').write("0")

open('/sys/class/gpio/gpio%s/value' % leds[1]['gpio'], 'w').write("1")
open('/sys/class/gpio/gpio%s/value' % leds[2]['gpio'], 'w').write("1")
time.sleep(1)
open('/sys/class/gpio/gpio%s/value' % leds[1]['gpio'], 'w').write("0")
open('/sys/class/gpio/gpio%s/value' % leds[2]['gpio'], 'w').write("0")

###

open('/sys/class/gpio/gpio%s/value' % leds[0]['gpio'], 'w').write("1")
open('/sys/class/gpio/gpio%s/value' % leds[1]['gpio'], 'w').write("1")
open('/sys/class/gpio/gpio%s/value' % leds[2]['gpio'], 'w').write("1")
time.sleep(1)
open('/sys/class/gpio/gpio%s/value' % leds[0]['gpio'], 'w').write("0")
open('/sys/class/gpio/gpio%s/value' % leds[1]['gpio'], 'w').write("0")
open('/sys/class/gpio/gpio%s/value' % leds[2]['gpio'], 'w').write("0")

 
 
# cleanup - remove GPIO38 folder from file system
for led in leds:
   open('/sys/class/gpio/unexport', 'w').write(led['gpio'])
