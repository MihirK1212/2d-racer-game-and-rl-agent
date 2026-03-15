import struct
import time
import keyboard
from multiprocessing import shared_memory

SHM_NAME = "RacerGameSHM"
SHM_SIZE = 56

# struct format
INPUT_FORMAT = "4B"
STATE_FORMAT = "B3x6d"

def is_key_pressed(key):
    return keyboard.is_pressed(key)


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
        shm.close()


if __name__ == "__main__":
    main()