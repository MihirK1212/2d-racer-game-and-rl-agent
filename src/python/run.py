import mmap
import struct
import time
import ctypes

SHM_NAME = "RacerGameSHM"
SHM_SIZE = 56

# struct format: 4 input bytes + 1 state flag + 3 pad + 6 doubles
INPUT_FORMAT = "4B"        # input_up, input_down, input_left, input_right
STATE_FORMAT = "B3x6d"    # state_ready, 3 pad bytes, 6 doubles
FULL_FORMAT  = "4B B3x 6d" # won't use this directly, just for reference

# Windows virtual key codes
VK_UP    = 0x26
VK_DOWN  = 0x28
VK_LEFT  = 0x25
VK_RIGHT = 0x27
VK_ESC   = 0x1B

user32 = ctypes.windll.user32

def is_key_pressed(vk_code: int) -> bool:
    return bool(user32.GetAsyncKeyState(vk_code) & 0x8000)


def main():
    # Open the named shared memory created by the C++ game
    try:
        shm = mmap.mmap(-1, SHM_SIZE, tagname=SHM_NAME, access=mmap.ACCESS_WRITE)
    except Exception as e:
        print(f"Failed to open shared memory '{SHM_NAME}': {e}")
        print("Make sure the C++ game is running first.")
        return

    print("Connected to shared memory. Use arrow keys to drive, ESC to quit.")
    print("Press '9' in the game window to send car state here.\n")

    try:
        while True:
            # Read current key states (works globally, even if game window is focused)
            up    = int(is_key_pressed(VK_UP))
            down  = int(is_key_pressed(VK_DOWN))
            left  = int(is_key_pressed(VK_LEFT))
            right = int(is_key_pressed(VK_RIGHT))

            # Write input flags to shared memory (bytes 0-3)
            shm.seek(0)
            shm.write(struct.pack("4B", up, down, left, right))

            # Check if C++ has sent new car state (byte 4)
            shm.seek(4)
            state_ready = struct.unpack("B", shm.read(1))[0]

            if state_ready:
                # Read car state (bytes 8-55: six doubles)
                shm.seek(8)
                pos_x, pos_y, speed, dir_x, dir_y, tangential_accel = struct.unpack("6d", shm.read(48))

                print("─── Car State ───")
                print(f"  Position:     ({pos_x:.3f}, {pos_y:.3f})")
                print(f"  Speed:         {speed:.3f}")
                print(f"  Direction:    ({dir_x:.3f}, {dir_y:.3f})")
                print(f"  Acceleration:  {tangential_accel:.3f}")
                print()

                # Clear the flag so we don't re-read the same state
                shm.seek(4)
                shm.write(struct.pack("B", 0))

            # ~60 Hz to match the game's frame rate
            time.sleep(1 / 60)

            if is_key_pressed(VK_ESC):
                print("ESC pressed, exiting.")
                break

    except KeyboardInterrupt:
        print("\nInterrupted.")
    finally:
        shm.close()


if __name__ == "__main__":
    main()