import time
import math
import random
from datetime import datetime, timedelta
from Xlib import X, XK, display
from Xlib.ext import xtest

# Set up display
d = display.Display()
root = d.screen().root

# Timers
now = lambda: datetime.now()
steer_pause_until = now()
last_q = now() - timedelta(minutes=16)
last_e = now() - timedelta(minutes=11)
last_clear = now() - timedelta(seconds=6)
last_heal = now() - timedelta(seconds=16)
last_innervate = now() - timedelta(minutes=7)
last_seal = now() - timedelta(seconds=25)

# Movement flags
is_turning_left = False
is_turning_right = False
is_moving_forward = False
is_healing = False
has_target = False

# Pixel offsets
baseX, baseY = 63, 99
Y_BOX_OFFSET = 9
COMBAT_BOX_OFFSET = 19
HP_BOX_OFFSET = 29
TARGET_BOX_OFFSET = 38
MANA_BOX_OFFSET = 57
FACING_BOX_OFFSET = 67
POSITION_BOX_OFFSET = 77
AGGRO_BOX_OFFSET = 86

# Utility structures
class RGB:
    def __init__(self, r, g, b): self.r, self.g, self.b = r, g, b

class Point:
    def __init__(self, x, y): self.x, self.y = x, y

def decode_component(val):
    return (val / 255.0) * 100.0

def decode_heading_rad(red):
    pct = decode_component(red) / 100.0
    return pct * 2.0 * math.pi

def normalize_angle(angle):
    while angle > math.pi: angle -= 2 * math.pi
    while angle < -math.pi: angle += 2 * math.pi
    return angle

def get_pixel_color(x, y):
    img = root.get_image(x, y, 1, 1, X.ZPixmap, 0xffffffff)
    pixel = int.from_bytes(img.data, byteorder='little')
    r = (pixel >> 16) & 0xff
    g = (pixel >> 8) & 0xff
    b = pixel & 0xff
    return RGB(r, g, b)

def box_is_red(c): return c.r > 200 and c.g < 80 and c.b < 80
def box_is_green(c): return c.g > 200 and c.r < 80 and c.b < 80

def press_key(keysym, hold=True, delay_ms=0):
    keycode = d.keysym_to_keycode(keysym)
    xtest.fake_input(d, X.KeyPress, keycode)
    d.flush()
    if hold and delay_ms > 0:
        time.sleep(delay_ms / 1000)
        xtest.fake_input(d, X.KeyRelease, keycode)
    elif not hold:
        time.sleep(delay_ms / 1000)
        xtest.fake_input(d, X.KeyRelease, keycode)
    d.flush()

def release_key(keysym):
    keycode = d.keysym_to_keycode(keysym)
    xtest.fake_input(d, X.KeyRelease, keycode)
    d.flush()

def press_combo(modifier, key, delay_ms=0):
    mod = d.keysym_to_keycode(modifier)
    k = d.keysym_to_keycode(key)
    xtest.fake_input(d, X.KeyPress, mod)
    xtest.fake_input(d, X.KeyPress, k)
    xtest.fake_input(d, X.KeyRelease, k)
    xtest.fake_input(d, X.KeyRelease, mod)
    d.flush()
    if delay_ms > 0:
        time.sleep(delay_ms / 1000)

def right_click(x, y):
    xtest.fake_input(d, X.MotionNotify, x=x, y=y)
    xtest.fake_input(d, X.ButtonPress, 3)
    xtest.fake_input(d, X.ButtonRelease, 3)
    d.flush()

def wait_for_gcd():
    time.sleep(1.6)

# Define path
path = [
    Point(53.3333, 10.9804),
    Point(52.9412, 10.9804),
    Point(53.3333, 11.3725),
    Point(53.7255, 11.7647),
    Point(53.7255, 12.549),
    Point(53.3333, 12.549),
    Point(53.3333, 13.3333),
    Point(53.3333, 13.3333),
    Point(53.3333, 13.7255),
    Point(52.9412, 13.7255),
    Point(52.549, 14.1176),
    Point(52.549, 14.902),
    Point(52.549, 15.6863),
    Point(52.9412, 15.6863),
    Point(52.9412, 15.2941),
    Point(53.3333, 15.2941),
    Point(53.7255, 15.6863),
    Point(54.5098, 15.6863),
    Point(54.902, 15.2941),
    Point(55.2941, 15.2941),
    Point(55.6863, 14.5098),
    Point(55.2941, 14.1176),
    Point(55.2941, 13.3333),
    Point(55.6863, 12.549),
    Point(55.2941, 12.549),
    Point(54.902, 13.7255),
    Point(54.5098, 13.3333),
    Point(54.902, 12.9412),
    Point(54.902, 12.1569),
    Point(54.1176, 11.7647),
]

def main():
    print("Starting in 3...")
    time.sleep(1)
    print("2...")
    time.sleep(1)
    print("1...")
    time.sleep(1)

    WAYPOINT_RADIUS = 0.15

    while True:
        for wp in path:
            while True:
                x_box = get_pixel_color(baseX, baseY)
                y_box = get_pixel_color(baseX + Y_BOX_OFFSET, baseY)
                combat_box = get_pixel_color(baseX + COMBAT_BOX_OFFSET, baseY)
                hp_box = get_pixel_color(baseX + HP_BOX_OFFSET, baseY)
                target_box = get_pixel_color(baseX + TARGET_BOX_OFFSET, baseY)

                x = decode_component(x_box.r)
                y = decode_component(y_box.g)
                hp = decode_component(hp_box.b)
                in_combat = combat_box.r > 200
                has_target = target_box.r > 200

                print(f"Current: {x:.2f},{y:.2f} Target: {wp.x},{wp.y} HP: {hp:.1f}% Combat: {'YES' if in_combat else 'NO'}")

                dx = abs(x - wp.x)
                dy = abs(y - wp.y)

                if dx < WAYPOINT_RADIUS and dy < WAYPOINT_RADIUS:
                    print("Reached waypoint")
                    break

                # Placeholder: move_towards() would be implemented similarly
                press_key(XK.XK_Up, hold=True)
                time.sleep(0.1)

if __name__ == "__main__":
    main()
