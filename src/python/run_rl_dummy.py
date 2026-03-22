"""
Dummy RL script: simulates the RL loop without any real learning.
Reads state from shared memory, picks random WASD actions, writes them and sets action_ready.
"""

import struct
import time
import random
from multiprocessing import shared_memory, resource_tracker
from shm_state import (
    SHM_NAME,
    OFF_INPUT,
    OFF_ACTION_READY,
    OFF_STATE_READY,
    read_state,
    print_state,
)


def main():
    try:
        shm = shared_memory.SharedMemory(name=SHM_NAME)
    except FileNotFoundError:
        print(f"Shared memory '{SHM_NAME}' not found.")
        print("Make sure the game is running first.")
        return

    resource_tracker.unregister(f"/{SHM_NAME}", "shared_memory")
    buf = shm.buf

    print("RL dummy connected to shared memory.")
    print("Simulating: read state -> random action -> write action, set action_ready=1")
    print("Press Ctrl+C to exit.\n")

    try:
        while True:
            state_ready = struct.unpack_from("B", buf, OFF_STATE_READY)[0]

            if state_ready:
                s = read_state(buf)

                up = random.randint(0, 1)
                down = random.randint(0, 1)
                left = random.randint(0, 1)
                right = random.randint(0, 1)

                struct.pack_into("4B", buf, OFF_INPUT, up, down, left, right)
                struct.pack_into("B", buf, OFF_ACTION_READY, 1)
                struct.pack_into("B", buf, OFF_STATE_READY, 0)

                time.sleep(1 / 30)

                keys = [k for k, v in zip("wsad", [up, down, left, right]) if v]
                print_state(s)
                print(f"  Action:           {keys or 'none'}\n")

            time.sleep(1 / 60)

    except KeyboardInterrupt:
        print("\nExiting.")

    finally:
        shm.close()


if __name__ == "__main__":
    main()
