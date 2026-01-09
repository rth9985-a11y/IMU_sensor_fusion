import numpy as np
import matplotlib.pyplot as plt
import serial
import time
from collections import deque

ser = serial.Serial(port="PORT NAME HERE", baudrate=115200, timeout=0.1)
time.sleep(2)

# Buffers
acc_buf = deque(maxlen=200)
gyr_buf = deque(maxlen=200)

plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6))

line1, = ax1.plot([], [])
line2, = ax2.plot([], [])

ax1.set_title("Gyro data")
ax1.set_xlabel("Time")
ax1.set_ylabel("Change in roataion")
ax1.set_xlim(0, 200)
ax1.set_ylim(-500, 500)

ax2.set_title("Accel data")
ax2.set_xlabel("Time")
ax2.set_ylabel("Change in velocity")
ax2.set_xlim(0, 200)
ax2.set_ylim(-20, 20)

last_plot_time = time.time()
REFRESH_SEC = 1.0

while True:
    if ser.in_waiting:
        raw = ser.readline().decode().strip()
        parts = raw.split(",")

        if len(parts) == 6:
            ax, ay, az, gx, gy, gz = map(float, parts)
            acc_buf.append((ax**2 + ay**2 + az**2)**0.5)
            gyr_buf.append((gx**2 + gy**2 + gz**2)**0.5)

    if time.time() - last_plot_time >= 1.0:
        line1.set_data(range(len(acc_buf)), acc_buf)
        line2.set_data(range(len(gyr_buf)), gyr_buf)
        plt.pause(0.001)
        last_plot_time = time.time()