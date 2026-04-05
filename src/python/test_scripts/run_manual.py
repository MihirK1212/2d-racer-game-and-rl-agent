import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parent.parent))

import time

from pynput import keyboard

from shm import ShmAccessor, print_state

keys_pressed = {"w": False, "s": False, "a": False, "d": False, "esc": False}


def on_press(key):
    try:
        if key.char in keys_pressed:
            keys_pressed[key.char] = True
    except AttributeError:
        if key == keyboard.Key.esc:
            keys_pressed["esc"] = True


def on_release(key):
    try:
        if key.char in keys_pressed:
            keys_pressed[key.char] = False
    except AttributeError:
        if key == keyboard.Key.esc:
            keys_pressed["esc"] = False


listener = keyboard.Listener(on_press=on_press, on_release=on_release)
listener.start()


def main():
    try:
        shm = ShmAccessor()
    except FileNotFoundError:
        print("Shared memory not found. Make sure the game is running first.")
        return

    print("Connected to shared memory.")
    print("Use WASD to drive, ESC to quit.\n")

    try:
        while True:
            up = int(keys_pressed["w"])
            down = int(keys_pressed["s"])
            left = int(keys_pressed["a"])
            right = int(keys_pressed["d"])

            shm.set_input(up, down, left, right)
            shm.set_action_ready(True)

            if shm.is_state_ready():
                state = shm.read_state()
                print_state(state)
                shm.set_state_ready(False)

            time.sleep(1 / 60)

            if keys_pressed["esc"]:
                print("ESC pressed, exiting.")
                break

    except KeyboardInterrupt:
        print("\nInterrupted.")

    finally:
        listener.stop()
        shm.close()


if __name__ == "__main__":
    main()
