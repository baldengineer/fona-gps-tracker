In order to connect a FONA (SIM808) to cellular network with Hologram.io, I had to modify the Adafruit FONA library. I'm not a C++ programmer so I don't know the right way to do any of this stuff. I do know that once I added the "janesFONA" method(?) and called that, my module was very happy.

I'm not including the "full" library here. Instead, here is what you need to add to the official library.

It's found here: https://github.com/adafruit/Adafruit_FONA