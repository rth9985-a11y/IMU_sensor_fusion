import numpy as np
import matplotlib.pyplot as plt
import serial
import time
from collections import deque

ser = serial.Serial(port="/dev/cu.usbmodem186647801", baudrate=921600, timeout=0.1)
time.sleep(2)

# Buffers
pitch_buf = deque(maxlen=600)
roll_buf = deque(maxlen=600)

gx_buf = deque(maxlen=600)
gy_buf = deque(maxlen=600)
gz_buf = deque(maxlen=600)

plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6))

# Create line objects 
line_roll, = ax1.plot([], [])
line_pitch, = ax1.plot([], [])

linegx, = ax2.plot([], [])
linegy, = ax2.plot([], [])
linegz, = ax2.plot([], [])

ax1.set_title("Pitch + Roll")
#ax1.set_xlabel("Time")
ax1.set_ylabel("Angle relative to gravity")
ax1.set_xlim(0, 600)
ax1.set_ylim(-180, 180)

ax2.set_title("Gyro")
#ax2.set_xlabel("Time")
ax2.set_ylabel("Angular Velocity (deg/s)")
ax2.set_xlim(0, 600)
ax2.set_ylim(-180, 180)

last_plot_time = time.time()
REFRESH_SEC = 0.001

while True:
    if ser.in_waiting:
        raw = ser.readline().decode().strip()
        parts = raw.split(",")

        if len(parts) == 5:

            roll, pitch, gx, gy, gz = map(float, parts)

            roll_buf.append(roll)
            pitch_buf.append(pitch)

            gx_buf.append(gx)
            gy_buf.append(gy)
            gz_buf.append(gz)

        else:
            print("ERROR - Serial data not the right size")
            break

    if time.time() - last_plot_time >= REFRESH_SEC:
        line_roll.set_data(range(len(roll_buf)), roll_buf)
        line_pitch.set_data(range(len(pitch_buf)), pitch_buf)

        linegx.set_data(range(len(gx_buf)), gx_buf)
        linegy.set_data(range(len(gy_buf)), gy_buf)
        linegz.set_data(range(len(gz_buf)), gz_buf)

        plt.pause(REFRESH_SEC)

        last_plot_time = time.time()