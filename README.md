# TBC-volume-control
 * Published under the beer-ware license 
 * 
Purpose of this program is to adjust an audio mixer poti, based on the current daytime.
The accurate time is provided by an external realtime clock module.
I'm using this for an installation which should run on a louder volume at day, a more silent volume in the evening and probably no sound at night.
For that purpose, three servo positions and three trigger times (day/evening/night) can be stored in the EEPROM of the microcontroller.
When a trigger time is reached, the program automatically adjusts the volume knob to the desired position.

3D printable design files for the mechanism which connects a Tower Pro SG90 micro servo to a Mackie 402VLZ4 mixer can be found here: 
https://www.youmagine.com/designs/servo-on-audio-mixer
