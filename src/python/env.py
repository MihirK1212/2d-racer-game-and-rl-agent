"""
Gymnasium environment for the 2D Racer game.

Communicates with the C++ engine via POSIX shared memory in lock-step mode.
Start the C++ game with externalInputMode=true, stepMode=true BEFORE
instantiating this environment.

Observation (12 floats):
    speed, heading_vs_tangent, dist_inner, dist_outer, 7x raycasts, collided

Action (Discrete 9):
    0=nothing  1=fwd  2=back  3=left  4=fwd+left
    5=back+left  6=right  7=fwd+right  8=back+right

Reward:
    +progress_delta (degrees around track, primary signal)
    -5.0  per collision frame
    -0.01 per step (time penalty)
    +100  per lap completed
"""

import gymnasium as gym
import numpy as np
from gymnasium import spaces

from shm import ShmAccessor, GameState

ACTION_TABLE = [
    (0, 0, 0, 0),  # 0: nothing
    (1, 0, 0, 0),  # 1: forward
    (0, 1, 0, 0),  # 2: backward
    (0, 0, 1, 0),  # 3: left
    (1, 0, 1, 0),  # 4: forward + left
    (0, 1, 1, 0),  # 5: backward + left
    (0, 0, 0, 1),  # 6: right
    (1, 0, 0, 1),  # 7: forward + right
    (0, 1, 0, 1),  # 8: backward + right
]

_MAX_SPEED = 20.0
_WALL_CLIP = 20.0
_RAY_CLIP = 50.0

_COLLISION_PENALTY = 5.0
_TIME_PENALTY = 0.01
_LAP_BONUS = 100.0


class RacerEnv(gym.Env):
    """Lock-step Gymnasium wrapper over the C++ 2D racer engine."""

    metadata = {"render_modes": ["human"]}

    def __init__(self, max_episode_steps=5000, render_mode=None):
        super().__init__()
        self.render_mode = render_mode
        self._max_steps = max_episode_steps

        self.observation_space = spaces.Box(
            low=-1.0, high=1.0, shape=(12,), dtype=np.float32
        )
        self.action_space = spaces.Discrete(len(ACTION_TABLE))

        self._shm = ShmAccessor()

        self._prev_progress = 0.0
        self._prev_laps = 0
        self._steps = 0

    @staticmethod
    def _obs_from_state(s: GameState) -> np.ndarray:
        p = s.player
        return np.array(
            [
                p.speed / _MAX_SPEED,
                p.heading_vs_tangent / np.pi,
                min(p.dist_inner, _WALL_CLIP) / _WALL_CLIP,
                min(p.dist_outer, _WALL_CLIP) / _WALL_CLIP,
                *[min(d, _RAY_CLIP) / _RAY_CLIP for d in p.ray_distances],
                float(p.collided),
            ],
            dtype=np.float32,
        )

    def _compute_reward(self, s: GameState) -> float:
        p = s.player
        progress_delta = p.total_progress - self._prev_progress
        reward = progress_delta

        if p.collided:
            reward -= _COLLISION_PENALTY

        reward -= _TIME_PENALTY

        laps_gained = p.lap - self._prev_laps
        if laps_gained > 0:
            reward += _LAP_BONUS * laps_gained

        self._prev_progress = p.total_progress
        self._prev_laps = p.lap
        return reward

    def reset(self, *, seed=None, options=None):
        super().reset(seed=seed)

        self._shm.set_state_ready(False)
        self._shm.set_input(0, 0, 0, 0)
        self._shm.set_action_ready(False)
        self._shm.set_reset_flag(True)

        self._shm.wait_for_state()
        s = self._shm.read_state()
        self._shm.set_state_ready(False)

        self._prev_progress = s.player.total_progress
        self._prev_laps = s.player.lap
        self._steps = 0

        return self._obs_from_state(s), {}

    def step(self, action: int):
        up, down, left, right = ACTION_TABLE[action]

        self._shm.set_state_ready(False)
        self._shm.set_input(up, down, left, right)
        self._shm.set_action_ready(True)

        self._shm.wait_for_state()
        s = self._shm.read_state()

        obs = self._obs_from_state(s)
        reward = self._compute_reward(s)

        terminated = s.done
        self._steps += 1
        truncated = self._steps >= self._max_steps

        info = {
            "total_progress": s.player.total_progress,
            "lap": s.player.lap,
            "speed": s.player.speed,
        }

        return obs, reward, terminated, truncated, info

    def close(self):
        self._shm.close()
