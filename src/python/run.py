import struct
import time
from pynput import keyboard
from multiprocessing import shared_memory

SHM_NAME = "RacerGameSHM"
SHM_SIZE = 56

# struct format
INPUT_FORMAT = "4B"
STATE_FORMAT = "B3x6d"


keys_pressed = {
    "up": False,
    "down": False,
    "left": False,
    "right": False,
    "esc": False
}

def on_press(key):
    if key == keyboard.Key.up:
        keys_pressed["up"] = True
    elif key == keyboard.Key.down:
        keys_pressed["down"] = True
    elif key == keyboard.Key.left:
        keys_pressed["left"] = True
    elif key == keyboard.Key.right:
        keys_pressed["right"] = True
    elif key == keyboard.Key.esc:
        keys_pressed["esc"] = True

def on_release(key):
    if key == keyboard.Key.up:
        keys_pressed["up"] = False
    elif key == keyboard.Key.down:
        keys_pressed["down"] = False
    elif key == keyboard.Key.left:
        keys_pressed["left"] = False
    elif key == keyboard.Key.right:
        keys_pressed["right"] = False
    elif key == keyboard.Key.esc:
        keys_pressed["esc"] = False

listener = keyboard.Listener(on_press=on_press, on_release=on_release)
listener.start()

def is_key_pressed(key):
    return keys_pressed[key]


def main():
    try:
        shm = shared_memory.SharedMemory(name=SHM_NAME)
    except FileNotFoundError:
        print(f"Shared memory '{SHM_NAME}' not found.")
        print("Make sure the game is running first.")
        return

    buffer = shm.buf

    print("Connected to shared memory.")
    print("Use arrow keys to drive, ESC to quit.\n")

    try:
        while True:

            up    = int(is_key_pressed("up"))
            down  = int(is_key_pressed("down"))
            left  = int(is_key_pressed("left"))
            right = int(is_key_pressed("right"))

            # Write inputs (bytes 0-3)
            buffer[0:4] = struct.pack("4B", up, down, left, right)

            # Check state_ready flag
            state_ready = struct.unpack("B", buffer[4:5])[0]

            if state_ready:

                pos_x, pos_y, speed, dir_x, dir_y, tangential_accel = struct.unpack(
                    "6d", buffer[8:56]
                )

                print("─── Car State ───")
                print(f"Position:     ({pos_x:.3f}, {pos_y:.3f})")
                print(f"Speed:         {speed:.3f}")
                print(f"Direction:    ({dir_x:.3f}, {dir_y:.3f})")
                print(f"Acceleration:  {tangential_accel:.3f}")
                print()

                # reset flag
                buffer[4:5] = struct.pack("B", 0)

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