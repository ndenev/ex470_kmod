ex470_kmod
==========

HP EX470 Media Smart Server Enclosure Control KMOD for FreeBSD

At the moment when loaded this module exposes the HDD Tray leds as led(4)
devices under /dev/led/
Also it hides the internal 256MB flash, to protect it from accidental damage, as it may 
prevent the machine from booting.
