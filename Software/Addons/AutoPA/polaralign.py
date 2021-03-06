#!/usr/bin/env python3
import math
from astropy.coordinates import SkyCoord, EarthLocation, AltAz, Angle
from astropy import units as u
from astropy.time import Time
from astropy.utils import iers
import sys, subprocess

def polarcalc(mylat, mylong, myelev, observing_time, p1RA, p1DEC, p2RA, p2DEC, p3RA, p3DEC):
    #iers.conf.auto_download = False
    #iers.conf.auto_max_age = None
    
    #Create time object based on given time
    observing_time = Time(observing_time)

    #Create location object based on lat/long/elev
    observing_location = EarthLocation(lat=mylat*u.deg, lon=mylong*u.deg, height=myelev*u.m)

    #Create coordinate objects for each point
    p1 = SkyCoord(p1RA, p1DEC, unit='deg')
    p2 = SkyCoord(p2RA, p2DEC, unit='deg')
    p3 = SkyCoord(p3RA, p3DEC, unit='deg')
    p1X = (90 - p1.dec.degree) * math.cos(p1.ra.radian)
    p1Y = (90 - p1.dec.degree) * math.sin(p1.ra.radian)
    p2X = (90 - p2.dec.degree) * math.cos(p2.ra.radian)
    p2Y = (90 - p2.dec.degree) * math.sin(p2.ra.radian)
    p3X = (90 - p3.dec.degree) * math.cos(p3.ra.radian)
    p3Y = (90 - p3.dec.degree) * math.sin(p3.ra.radian)

    #Calculate center of circle using three points in the complex plane. DEC is treated as unitless for the purposes of the calculation.
    x, y, z = complex(p1X,p1Y), complex(p2X,p2Y), complex(p3X,p3Y)
    w = z-x
    w /= y-x
    c = (x-y)*(w-abs(w)**2)/2j/w.imag-x
    resultX = -c.real
    resultY = c.imag

    #Convert X/Y values of circle into RA/DEC
    resultDEC = (90 - math.sqrt(resultX**2 + resultY**2))
    resultRA = math.atan2(resultY, resultX)*360 / (2*math.pi)
    if resultRA < 0:
            resultRA = (180-abs(resultRA))+180

    #Create coordinate object for current alignment offset
    offset = SkyCoord(resultRA, resultDEC, frame='itrs', unit='deg', representation_type='spherical', obstime=Time(observing_time))
    print(f"Current alignment in RA/DEC: {Angle(resultRA*u.deg).to_string(u.hour, precision=2)}/{Angle(resultDEC*u.deg).to_string(u.degree, precision=2)}.")

    #Create coordinate object for pole
    pole = SkyCoord(0, 90, frame='itrs', unit='deg', representation_type='spherical', obstime=Time(observing_time))
    
    #Create coordinate object for pole
    poleAzAlt = pole.transform_to(AltAz(obstime=Time(observing_time),location=observing_location))
    print(f"True polar alignment in Az./Alt.: 0h00m00s/{poleAzAlt.alt.to_string(u.degree, precision=2)}.")

    #Transform current alignment to Alt/Az coordinate system
    offsetAzAlt = offset.transform_to(AltAz(obstime=Time(observing_time),location=observing_location))
    print(f"Current alignment in Az./Alt.: {offsetAzAlt.az.to_string(u.hour, precision=2)}/{offsetAzAlt.alt.to_string(u.degree, precision=2)}.")

    #Calculate offset deltas from pole
    #Normalize the azimuth values to between -180 and 180 degrees prior to determining offset.
    errorAz = (((poleAzAlt.az.deg + 180) % 360 - 180)-((offsetAzAlt.az.deg + 180) % 360 - 180))*60
    print(f"Azimuth error correction is: {errorAz:.4f} arcminutes.")
    errorAlt = (poleAzAlt.alt.deg-offsetAzAlt.alt.deg)*60
    print(f"Altitude error correction is: {errorAlt:.4f} arcminutes.")
    
    return errorAz, errorAlt

#Latitude in degrees
mylat = float(sys.argv[1])

#Longitude in degrees
mylong = float(sys.argv[2])

#Elevation in meters
myelev = float(sys.argv[3])

#YYYY-MM-DD HH:MM:SS format
time = sys.argv[4]

#All RA/DEC values must be in compatible format to Astropy.coordinates library.
#Preferrably degrees, but 00h00m00.0s and 00d00m00.0s should also work
p1RA = float(sys.argv[5])
p1DEC = float(sys.argv[6])
p2RA = float(sys.argv[7])
p2DEC = float(sys.argv[8])
p3RA = float(sys.argv[9])
p3DEC = float(sys.argv[10])

#Serial port address for Arduino, typically /dev/ttyACM0 in Astroberry, possibly /dev/ttyACM1
if len(sys.argv) <= 11:
    serialport = "/dev/ttyACM0"
else:
    serialport = sys.argv[11]

result = polarcalc(mylat, mylong, myelev, time, p1RA, p1DEC, p2RA, p2DEC, p3RA, p3DEC)

#Verify error correction can be handled by AutoPA hardware (assuming it is in home/centered position)
moveAz = "N"
if abs(result[0]) > 120:
    moveAz = input("Azimuth error may be out of bounds of hardware capabilities if not in home position. Continue? (Y/N): ") 
else:
    moveAz = "Y"
if moveAz.upper() == "Y":
    #Call process to move azimuth using elevated privileges  to override any existing serial connection
    subprocess.call(['sudo', './altaz.py', "az", str(result[0]), serialport])

moveAlt = "N"
if result[1] > 168:
    moveAz = input("Altitude error may be out of bounds of hardware capabilities if not in home position. Continue? (Y/N): ")
elif result[1] > 432:
    moveAz = input("Altitude error may be out of bounds of hardware capabilities if not in home position. Continue? (Y/N): ")
else:
    moveAlt = "Y"
if moveAlt.upper() == "Y":
    #Call process to move altitude using elevated privileges to override any existing serial connection
    subprocess.call(['sudo', './altaz.py', "alt", str(result[1]), serialport])