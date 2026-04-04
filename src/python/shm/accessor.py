"""
Single access-point for all shared-memory reads and writes.
"""

from __future__ import annotations

import struct
import time
from multiprocessing import shared_memory, resource_tracker

from .constants import (
    SHM_NAME,
    NUM_RAYCASTS,
    OFF_INPUT,
    OFF_ACTION_READY,
    OFF_STATE_READY,
    OFF_RESET_FLAG,
    OFF_DONE_FLAG,
    OFF_PLAYER_U8,
    OFF_PLAYER_DBL,
    OFF_OPP_U8,
    OFF_OPP_DBL,
)
from .schema import GameState, PlayerState, OpponentState

_PLAYER_DBL_MEM_SIZE = f"{10 + NUM_RAYCASTS}d"


class ShmAccessor:
    """Read/write handle to the C++ racer engine's shared memory block."""

    def __init__(self) -> None:
        self._shm = shared_memory.SharedMemory(name=SHM_NAME)
        resource_tracker.unregister(f"/{SHM_NAME}", "shared_memory")
        self._buf = self._shm.buf

    def read_state(self) -> GameState:
        buf = self._buf

        lap, cp, rank, collided = struct.unpack_from("4B", buf, OFF_PLAYER_U8)
        doubles = struct.unpack_from(_PLAYER_DBL_MEM_SIZE, buf, OFF_PLAYER_DBL)
        (
            angle_on_track, total_progress, pos_x, pos_y, speed,
            dir_x, dir_y, dist_inner, dist_outer, heading_vs_tangent,
        ) = doubles[:10]
        ray_distances = doubles[10:]

        opp_lap, opp_cp, opp_rank, opp_collided = struct.unpack_from("4B", buf, OFF_OPP_U8)
        (
            opp_angle, opp_progress, opp_x, opp_y,
            opp_speed, opp_dx, opp_dy,
        ) = struct.unpack_from("7d", buf, OFF_OPP_DBL)

        done = struct.unpack_from("B", buf, OFF_DONE_FLAG)[0]

        return GameState(
            done=bool(done),
            player=PlayerState(
                lap=lap, checkpoints=cp, rank=rank, collided=bool(collided),
                angle_on_track=angle_on_track, total_progress=total_progress,
                pos_x=pos_x, pos_y=pos_y, speed=speed,
                dir_x=dir_x, dir_y=dir_y,
                dist_inner=dist_inner, dist_outer=dist_outer,
                heading_vs_tangent=heading_vs_tangent,
                ray_distances=ray_distances,
            ),
            opponent=OpponentState(
                lap=opp_lap, checkpoints=opp_cp, rank=opp_rank,
                collided=bool(opp_collided),
                angle_on_track=opp_angle, total_progress=opp_progress,
                pos_x=opp_x, pos_y=opp_y, speed=opp_speed,
                dir_x=opp_dx, dir_y=opp_dy,
            ),
        )

    def is_state_ready(self) -> bool:
        return bool(struct.unpack_from("B", self._buf, OFF_STATE_READY)[0])

    def set_state_ready(self, ready: bool) -> None:
        struct.pack_into("B", self._buf, OFF_STATE_READY, int(ready))

    def set_action_ready(self, ready: bool) -> None:
        struct.pack_into("B", self._buf, OFF_ACTION_READY, int(ready))

    def set_reset_flag(self, reset: bool) -> None:
        struct.pack_into("B", self._buf, OFF_RESET_FLAG, int(reset))

    def set_input(self, up: int, down: int, left: int, right: int) -> None:
        struct.pack_into("4B", self._buf, OFF_INPUT, up, down, left, right)

    def wait_for_state(self, timeout_s: float = 30.0) -> None:
        """Busy-wait until the engine sets state_ready, or raise TimeoutError."""
        t0 = time.monotonic()
        while not self.is_state_ready():
            if time.monotonic() - t0 > timeout_s:
                raise TimeoutError(
                    f"C++ engine did not set state_ready within {timeout_s}s. "
                    "Is the game running in step mode?"
                )

    def close(self) -> None:
        if self._shm is not None:
            self._shm.close()
            self._shm = None

    def __enter__(self) -> ShmAccessor:
        return self

    def __exit__(self, *_: object) -> None:
        self.close()
