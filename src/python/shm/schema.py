"""
Pydantic schemas for the shared-memory game state.
"""

from pydantic import BaseModel, ConfigDict


class PlayerState(BaseModel):
    model_config = ConfigDict(frozen=True)

    lap: int
    checkpoints: int
    rank: int
    collided: bool
    angle_on_track: float
    total_progress: float
    pos_x: float
    pos_y: float
    speed: float
    dir_x: float
    dir_y: float
    dist_inner: float
    dist_outer: float
    heading_vs_tangent: float
    ray_distances: tuple[float, ...]


class OpponentState(BaseModel):
    model_config = ConfigDict(frozen=True)

    lap: int
    checkpoints: int
    rank: int
    collided: bool
    angle_on_track: float
    total_progress: float
    pos_x: float
    pos_y: float
    speed: float
    dir_x: float
    dir_y: float


class GameState(BaseModel):
    model_config = ConfigDict(frozen=True)

    done: bool
    player: PlayerState
    opponent: OpponentState
