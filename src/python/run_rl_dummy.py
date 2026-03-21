"""
Dummy RL script: simulates the RL loop without any real learning.
Reads state from shared memory, picks random WASD actions, writes them and sets action_ready.
"""
import struct
import time
import random
from multiprocessing import shared_memory, resource_tracker

SHM_NAME = "RacerGameSHM"
SHM_SIZE = 56

# Same layout as run_manual.py
INPUT_FORMAT = "4B"
STATE_FORMAT = "B3x6d"


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

    print("RL dummy connected to shared memory.")
    print("Simulating: read state -> random action (w/a/s/d) -> write action, set action_ready=1")
    print("Press Ctrl+C to exit.\n")

    try:
        while True:
            # Check if game has written a new state
            state_ready = struct.unpack("B", buffer[5:6])[0]

            if state_ready:
                # 1) Read state (same offsets as run_manual.py)
                pos_x, pos_y, speed, dir_x, dir_y, tangential_accel = struct.unpack(
                    "6d", buffer[11:59]
                )

                # 2) "Decide" action: random combination of w, a, s, d (each 0 or 1)
                up = random.randint(0, 1)
                down = random.randint(0, 1)
                left = random.randint(0, 1)
                right = random.randint(0, 1)

                # 3) Write inputs (bytes 0-3)
                buffer[0:4] = struct.pack("4B", up, down, left, right)

                # 4) Set action_ready flag to 1 so the game reads our action
                buffer[4:5] = struct.pack("B", 1)

                # 5) Acknowledge: clear state_ready so we don't re-use same state
                buffer[5:6] = struct.pack("B", 0)

                keys = []
                if up:
                    keys.append("w")
                if down:
                    keys.append("s")
                if left:
                    keys.append("a")
                if right:
                    keys.append("d")
                print(
                    f"State: pos=({pos_x:.2f},{pos_y:.2f}) speed={speed:.2f}  ->  "
                    f"action: {keys or 'none'}"
                )

            time.sleep(1 / 60)

    except KeyboardInterrupt:
        print("\nExiting.")

    finally:
        shm.close()


if __name__ == "__main__":
    main()
