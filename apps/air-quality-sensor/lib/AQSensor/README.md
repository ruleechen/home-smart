#SGP30

https://forums.adafruit.com/viewtopic.php?t=135180
Please refer to the datasheet of SGP30: https://www.sensirion.com/file/datasheet_sgp30
The sampling interval is specified with 1s and if you're not compliant with it the baseline algorithm won't perform well.

> After the “sgp30_iaq_init” command, a “sgp30_measure_iaq” command has to be sent in regular intervals of 1s to ensure proper operation of the dynamic baseline compensation algorithm.
