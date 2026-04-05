"""
Dummy RL script: simulates the RL loop without any real learning.
Reads state from shared memory, picks random WASD actions, writes them
and sets action_ready.
"""

import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parent.parent))

import time
import random

from shm import ShmAccessor, print_state


def main():
    try:
        shm = ShmAccessor()
    except FileNotFoundError:
        print("Shared memory not found. Make sure the game is running first.")
        return

    print("RL dummy connected to shared memory.")
    print("Simulating: read state -> random action -> write action, set action_ready=1")
    print("Press Ctrl+C to exit.\n")

    try:
        while True:
            if not shm.is_state_ready():
                time.sleep(1 / 60)
                continue

            state = shm.read_state()

            if state.done:
                shm.set_input(0, 0, 0, 0)
                shm.set_action_ready(False)
                shm.set_reset_flag(True)
                shm.set_state_ready(False)
                print("  Done flag detected — reset triggered.\n")
                continue

            up = random.randint(0, 1)
            down = random.randint(0, 1)
            left = random.randint(0, 1)
            right = random.randint(0, 1)

            shm.set_input(up, down, left, right)
            shm.set_action_ready(True)
            shm.set_state_ready(False)

            keys = [k for k, v in zip("wsad", [up, down, left, right]) if v]
            print_state(state)
            print(f"  Action:           {keys or 'none'}\n")

    except KeyboardInterrupt:
        print("\nExiting.")

    finally:
        shm.close()


if __name__ == "__main__":
    main()
