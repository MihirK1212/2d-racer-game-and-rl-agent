"""
Dummy RL script: simulates the RL loop without any real learning.
Reads state from shared memory, picks random WASD actions, writes them and sets action_ready.
"""
import struct
import time
import random
from multiprocessing import shared_memory, resource_tracker

SHM_NAME = "RacerGameSHM"
SHM_SIZE = 160

# ── Byte offsets (must match #pragma pack(push,1) SharedGameData) ──
OFF_INPUT        = 0
OFF_ACTION_READY = 4
OFF_STATE_READY  = 5
OFF_RESET_FLAG   = 6
OFF_DONE_FLAG    = 7

OFF_PLAYER_U8    = 8
OFF_PLAYER_DBL   = 16
OFF_OPP_U8       = 96
OFF_OPP_DBL      = 104


def read_state(buf):
    lap, cp, rank, collided = struct.unpack_from("4B", buf, OFF_PLAYER_U8)
    (angle_on_track, total_progress, pos_x, pos_y, speed,
     dir_x, dir_y, dist_inner, dist_outer,
     heading_vs_tangent) = struct.unpack_from("10d", buf, OFF_PLAYER_DBL)

    opp_lap, opp_cp, opp_rank, opp_collided = struct.unpack_from("4B", buf, OFF_OPP_U8)
    (opp_angle, opp_progress, opp_x, opp_y, opp_speed,
     opp_dx, opp_dy) = struct.unpack_from("7d", buf, OFF_OPP_DBL)

    done = struct.unpack_from("B", buf, OFF_DONE_FLAG)[0]

    return {
        "done": done,
        "lap": lap, "checkpoints": cp, "rank": rank, "collided": collided,
        "angle_on_track": angle_on_track, "total_progress": total_progress,
        "pos_x": pos_x, "pos_y": pos_y, "speed": speed,
        "dir_x": dir_x, "dir_y": dir_y,
        "dist_inner": dist_inner, "dist_outer": dist_outer,
        "heading_vs_tangent": heading_vs_tangent,
        "opp_lap": opp_lap, "opp_checkpoints": opp_cp,
        "opp_rank": opp_rank, "opp_collided": opp_collided,
        "opp_angle": opp_angle, "opp_progress": opp_progress,
        "opp_x": opp_x, "opp_y": opp_y, "opp_speed": opp_speed,
        "opp_dx": opp_dx, "opp_dy": opp_dy,
    }


def print_state(s, action_keys):
    print("═══════════════════ Game State ═══════════════════")
    print(f"  Done flag:       {s['done']}")
    print("─── Player ───")
    print(f"  Lap: {s['lap']}   Checkpoints: {s['checkpoints']}   Rank: {s['rank']}   Collided: {s['collided']}")
    print(f"  Position:        ({s['pos_x']:.3f}, {s['pos_y']:.3f})")
    print(f"  Speed:            {s['speed']:.3f}")
    print(f"  Direction:       ({s['dir_x']:.3f}, {s['dir_y']:.3f})")
    print(f"  Angle on track:   {s['angle_on_track']:.3f}")
    print(f"  Total progress:   {s['total_progress']:.3f}")
    print(f"  Dist inner wall:  {s['dist_inner']:.3f}")
    print(f"  Dist outer wall:  {s['dist_outer']:.3f}")
    print(f"  Heading vs tang:  {s['heading_vs_tangent']:.3f}")
    print("─── Opponent ───")
    print(f"  Lap: {s['opp_lap']}   Checkpoints: {s['opp_checkpoints']}   Rank: {s['opp_rank']}   Collided: {s['opp_collided']}")
    print(f"  Position:        ({s['opp_x']:.3f}, {s['opp_y']:.3f})")
    print(f"  Speed:            {s['opp_speed']:.3f}")
    print(f"  Direction:       ({s['opp_dx']:.3f}, {s['opp_dy']:.3f})")
    print(f"  Angle on track:   {s['opp_angle']:.3f}")
    print(f"  Total progress:   {s['opp_progress']:.3f}")
    print(f"  Action:           {action_keys or 'none'}")
    print()


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

                up    = random.randint(0, 1)
                down  = random.randint(0, 1)
                left  = random.randint(0, 1)
                right = random.randint(0, 1)

                struct.pack_into("4B", buf, OFF_INPUT, up, down, left, right)
                struct.pack_into("B", buf, OFF_ACTION_READY, 1)
                struct.pack_into("B", buf, OFF_STATE_READY, 0)

                time.sleep(1/30)

                keys = []
                if up:    keys.append("w")
                if down:  keys.append("s")
                if left:  keys.append("a")
                if right: keys.append("d")

                print_state(s, keys)

            time.sleep(1 / 60)

    except KeyboardInterrupt:
        print("\nExiting.")

    finally:
        shm.close()


if __name__ == "__main__":
    main()
