![https://lh4.googleusercontent.com/-p9cmaVkRkcA/Tek567cfF0I/AAAAAAAAACM/73FuYMfKxew/s288/rds.png](https://lh4.googleusercontent.com/-p9cmaVkRkcA/Tek567cfF0I/AAAAAAAAACM/73FuYMfKxew/s288/rds.png)

The Silabs radio is a cheap USB FM Radio with RDS facilities. This project provides a DLL that makes it easy for you to utilize the radio and RDS functions in your own project. Example VB6 code is supplied.

It's supported in the FreeICE front end and with [RoadRunner](http://guino.home.insightbb.com/roadrunner.html) in a very limited fashion at the moment, but possibly in full capacity in future.

This currently supports the **Silabs reference radio** and **ADSTech Instant FM**. If you have an alternative vendor with a Silabs radio chipset, then please email me your VID and PID code for inclusion, as well as the name of the Audio device. You can then help test to see if it works.

I'm told that the **Brando** radio works, but with no RDS. It's the same with **PCear** - no RDS & also with bad audio quality. However, these radios are still useful for AF & TA purposes. I can use them as a 2nd tuner to check signal strength before we change frequency on the main radio.

Visit the [Digital Car UK Forums](http://www.digital-car.co.uk/forum/showthread.php?t=8867&page=14) for further details or to chat with the developer. My tag is "Beezer" and Guino is also contributing, so feel free to chat to him about this.

**Supported Radios with RDS**

![https://lh5.googleusercontent.com/-LVfRqM9MgMI/Tek56xzw1MI/AAAAAAAAACM/knO0cLmbF-8/s200/silabs.jpg](https://lh5.googleusercontent.com/-LVfRqM9MgMI/Tek56xzw1MI/AAAAAAAAACM/knO0cLmbF-8/s200/silabs.jpg)
![https://lh6.googleusercontent.com/-PwFiAZNEBfQ/Tek56_dl_XI/AAAAAAAAACM/yz_0mAnSIFc/s200/adstech.jpg](https://lh6.googleusercontent.com/-PwFiAZNEBfQ/Tek56_dl_XI/AAAAAAAAACM/yz_0mAnSIFc/s200/adstech.jpg)


**Supported Radios without RDS**

![https://lh3.googleusercontent.com/-J2RbR0K2Dxs/Tek56--6tcI/AAAAAAAAACM/S0o2IECk_5o/s200/brando.jpg](https://lh3.googleusercontent.com/-J2RbR0K2Dxs/Tek56--6tcI/AAAAAAAAACM/S0o2IECk_5o/s200/brando.jpg) ![https://lh5.googleusercontent.com/-GhcHe9TqyVI/Tek56xzgSZI/AAAAAAAAACM/0KlDlF23Hcs/s200/sku_1929_2.jpg](https://lh5.googleusercontent.com/-GhcHe9TqyVI/Tek56xzgSZI/AAAAAAAAACM/0KlDlF23Hcs/s200/sku_1929_2.jpg)

**Example Application**

![https://lh6.googleusercontent.com/-Z4Ghg0XkVLg/Tek56zs1M2I/AAAAAAAAACM/Uw95VvArjpY/s400/example1.jpg](https://lh6.googleusercontent.com/-Z4Ghg0XkVLg/Tek56zs1M2I/AAAAAAAAACM/Uw95VvArjpY/s400/example1.jpg)
![https://lh5.googleusercontent.com/-uvOmPSICwxg/Tek5659YYnI/AAAAAAAAACM/gW4Og2DMrU0/s400/example2.jpg](https://lh5.googleusercontent.com/-uvOmPSICwxg/Tek5659YYnI/AAAAAAAAACM/gW4Og2DMrU0/s400/example2.jpg)

**Improvements**

1] Station Name (PS) working fast and uncorrupted.

2] Radio Text working uncorrupted, updating properly

3] TP, TA, MS, Stereo flags working

4] PI code displayed

5] PTY code & PTY description displayed

6] Signal strength, frequency working

7] last station tuned is remembered (written to radio sram)

8] Smart RDS validation limit based on Signal Strength

9] Tuning speed is now pretty fast at the expense of slight noise during tuning

10] Decoding AF (but not yet auto switch)

11] Allow register of callbacks for alert of Traffic Announcement being played on current tuned station

12] Hiss free mute

13] Country Code (not extended as yet) and Region Code

14] Limited legacy Road Runner compatibility

**Upcoming Features**

1] Would like to get RDS:TMC working. Am investigating. However, this would require a dedicated radio dongle, unless you only listen to stations with TMC.

2] Would like to get EON and AF working, but once again this involves me purchasing & modifying another dongle. This may or may not happen.

