import struct
import time
from pynput import keyboard
from multiprocessing import shared_memory, resource_tracker

SHM_NAME = "RacerGameSHM"
SHM_SIZE = 56

# struct format
INPUT_FORMAT = "4B"
STATE_FORMAT = "B3x6d"

# Updated keys dictionary
keys_pressed = {
    "w": False,
    "s": False,
    "a": False,
    "d": False,
    "esc": False
}

def on_press(key):
    try:
        if key.char == 'w':
            keys_pressed["w"] = True
        elif key.char == 's':
            keys_pressed["s"] = True
        elif key.char == 'a':
            keys_pressed["a"] = True
        elif key.char == 'd':
            keys_pressed["d"] = True
    except AttributeError:
        # Handle special keys like ESC
        if key == keyboard.Key.esc:
            keys_pressed["esc"] = True

def on_release(key):
    try:
        if key.char == 'w':
            keys_pressed["w"] = False
        elif key.char == 's':
            keys_pressed["s"] = False
        elif key.char == 'a':
            keys_pressed["a"] = False
        elif key.char == 'd':
            keys_pressed["d"] = False
    except AttributeError:
        if key == keyboard.Key.esc:
            keys_pressed["esc"] = False

listener = keyboard.Listener(on_press=on_press, on_release=on_release)
listener.start()

def is_key_pressed(key):
    return keys_pressed.get(key, False)

def main():
    try:
        shm = shared_memory.SharedMemory(name=SHM_NAME)
    except FileNotFoundError:
        print(f"Shared memory '{SHM_NAME}' not found.")
        print("Make sure the game is running first.")
        return

    # The C++ game owns the shared memory — prevent Python's resource tracker
    # from destroying it when this process exits.
    resource_tracker.unregister(f"/{SHM_NAME}", "shared_memory")

    buffer = shm.buf

    print("Connected to shared memory.")
    print("Use WASD to drive, ESC to quit.\n")

    try:
        while True:
            # Map WASD to the existing logic
            up    = int(is_key_pressed("w"))
            down  = int(is_key_pressed("s"))
            left  = int(is_key_pressed("a"))
            right = int(is_key_pressed("d"))
            
            # Write inputs (bytes 0-3)
            buffer[0:4] = struct.pack("4B", up, down, left, right)

            # Set action_ready flag to 1
            buffer[4:5] = struct.pack("B", 1)

            # Get state_ready flag
            state_ready = struct.unpack("B", buffer[5:6])[0]

            if state_ready:
                pos_x, pos_y, speed, dir_x, dir_y, tangential_accel = struct.unpack(
                    "6d", buffer[11:59]
                )

                print("─── Car State ───")
                print(f"Position:     ({pos_x:.3f}, {pos_y:.3f})")
                print(f"Speed:         {speed:.3f}")
                print(f"Direction:    ({dir_x:.3f}, {dir_y:.3f})")
                print(f"Acceleration:  {tangential_accel:.3f}")
                print()

                # Set state_ready flag to 0
                buffer[5:6] = struct.pack("B", 0)

            time.sleep(1/60)

            if is_key_pressed("esc"):
                print("ESC pressed, exiting.")
                break

    except KeyboardInterrupt:
        print("\nInterrupted.")

    finally:
        listener.stop()
        shm.close()


if __name__ == "__main__":
    main()