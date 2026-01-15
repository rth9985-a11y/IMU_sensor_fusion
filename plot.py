import numpy as np
import matplotlib.pyplot as plt
import serial
import time
from collections import deque

ser = serial.Serial(port="/dev/cu.usbmodem186647801", baudrate=921600, timeout=0.1)
time.sleep(2)

# Buffers
ax_buf = deque(maxlen=600)
ay_buf = deque(maxlen=600)
az_buf = deque(maxlen=600)

gx_buf = deque(maxlen=600)
gy_buf = deque(maxlen=600)
gz_buf = deque(maxlen=600)

plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6))

# Create line objects 
lineax, = ax1.plot([], [])
lineay, = ax1.plot([], [])
lineaz, = ax1.plot([], [])

linegx, = ax2.plot([], [])
linegy, = ax2.plot([], [])
linegz, = ax2.plot([], [])

ax1.set_title("Accel")
#ax1.set_xlabel("Time")
ax1.set_ylabel("Velocity")
ax1.set_xlim(0, 600)
ax1.set_ylim(-10000, 10000)

ax2.set_title("Gyro")
#ax2.set_xlabel("Time")
ax2.set_ylabel("Change In Rotation")
ax2.set_xlim(0, 600)
ax2.set_ylim(-3500, 3500)

last_plot_time = time.time()
REFRESH_SEC = 0.001

while True:
    if ser.in_waiting:
        raw = ser.readline().decode().strip()
        parts = raw.split(",")

        if len(parts) == 6:

            ax, ay, az, gx, gy, gz = map(float, parts)

            ax_buf.append(ax)
            ay_buf.append(ay)
            az_buf.append(az)

            gx_buf.append(gx)
            gy_buf.append(gy)
            gz_buf.append(gz)

        else:
            print("ERROR - Serial data not the right size")
            break

    if time.time() - last_plot_time >= REFRESH_SEC:
        lineax.set_data(range(len(ax_buf)), ax_buf)
        lineay.set_data(range(len(ay_buf)), ay_buf)
        lineaz.set_data(range(len(az_buf)), az_buf)

        linegx.set_data(range(len(gx_buf)), gx_buf)
        linegy.set_data(range(len(gy_buf)), gy_buf)
        linegz.set_data(range(len(gz_buf)), gz_buf)

        plt.pause(REFRESH_SEC)

        last_plot_time = time.time()