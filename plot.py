import numpy as np
import matplotlib.pyplot as plt
import serial
import time
from collections import deque

ser = serial.Serial(port="/dev/cu.usbmodem186647801", baudrate=115200, timeout=0.1)
time.sleep(2)

# Buffers
acc_buf = deque(maxlen=600) # ACC good
gyr_buf = deque(maxlen=600)

plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6))

line1, = ax1.plot([], [])
line2, = ax2.plot([], [])

ax1.set_title("Accel")
#ax1.set_xlabel("Time")
ax1.set_ylabel("Velocity")
ax1.set_xlim(0, 600)
ax1.set_ylim(1500, 8200)

ax2.set_title("Gyro")
#ax2.set_xlabel("Time")
ax2.set_ylabel("Change In Rotation")
ax2.set_xlim(0, 600)
ax2.set_ylim(-25, 2700)

last_plot_time = time.time()
REFRESH_SEC = 0.001

while True:
    if ser.in_waiting:
        raw = ser.readline().decode().strip()
        parts = raw.split(",")

        if len(parts) == 6:
            ax, ay, az, gx, gy, gz = map(float, parts)
            acc_buf.append((ax**2 + ay**2 + az**2)**0.5)
            gyr_buf.append((gx**2 + gy**2 + gz**2)**0.5)
        
        else:
            print("ERROR - Serial data not the right size")
            break

    if time.time() - last_plot_time >= REFRESH_SEC:
        line1.set_data(range(len(acc_buf)), acc_buf)
        line2.set_data(range(len(gyr_buf)), gyr_buf)
        plt.pause(0.001)
        last_plot_time = time.time()