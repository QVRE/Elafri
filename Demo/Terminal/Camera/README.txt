Camera point-hashing/tracking algorithm - Noveber 6 2021

This is a bit of a rough code mostly thrown together and altered until it "just works".
The code has not been polished ever since it was written except for some extra comments.
WARNING: Only supports YUYV format on cameras and will attempt to set it, else fail.

Usage: Returns video of your camera to screen and you can click on a point and it will
attempt to track it via "hashing". I just randomly had this idea of hashing a pixel in video
and wanted to implement it
