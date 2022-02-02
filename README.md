# ArduinoMicSocket

Sending Mic Data To Pc

I connected a MAX9814 microphone to an Arduino, 
the code reads the audio data, formats it to make it compatible with pyaudio,
and sends it to the socket server, to be outputed by a speaker.

I used this video to fix a bug in my code.

https://www.youtube.com/watch?v=to3JDwU7r2U

i used an esp32 and connected the out pin of the MAX9814 to pin 36 on the esp32
