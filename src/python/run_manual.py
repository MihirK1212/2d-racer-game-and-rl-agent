import struct
import time
from pynput import keyboard
from multiprocessing import shared_memory, resource_tracker
from shm_state import (
    SHM_NAME,
    OFF_INPUT,
    OFF_ACTION_READY,
    OFF_STATE_READY,
    read_state,
    print_state,
)

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
        shm = shared_memory.SharedMemory(name=SHM_NAME)
    except FileNotFoundError:
        print(f"Shared memory '{SHM_NAME}' not found.")
        print("Make sure the game is running first.")
        return

    resource_tracker.unregister(f"/{SHM_NAME}", "shared_memory")
    buf = shm.buf

    print("Connected to shared memory.")
    print("Use WASD to drive, ESC to quit.\n")

    try:
        while True:
            up = int(keys_pressed["w"])
            down = int(keys_pressed["s"])
            left = int(keys_pressed["a"])
            right = int(keys_pressed["d"])

            struct.pack_into("4B", buf, OFF_INPUT, up, down, left, right)
            struct.pack_into("B", buf, OFF_ACTION_READY, 1)

            state_ready = struct.unpack_from("B", buf, OFF_STATE_READY)[0]
            if state_ready:
                s = read_state(buf)
                print_state(s)
                struct.pack_into("B", buf, OFF_STATE_READY, 0)

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
