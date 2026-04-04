"""
Simple display helpers for GameState.
"""

from .schema import GameState

_RAY_LABELS = ["-90°", "-60°", "-30°", "0°", "+30°", "+60°", "+90°"]


def print_state(state: GameState) -> None:
    p = state.player
    o = state.opponent

    print("═══════════════════ Game State ═══════════════════")
    print(f"  Done flag:       {state.done}")

    print("─── Player ───")
    print(f"  Lap: {p.lap}   Checkpoints: {p.checkpoints}   Rank: {p.rank}   Collided: {p.collided}")
    print(f"  Position:        ({p.pos_x:.3f}, {p.pos_y:.3f})")
    print(f"  Speed:            {p.speed:.3f}")
    print(f"  Direction:       ({p.dir_x:.3f}, {p.dir_y:.3f})")
    print(f"  Angle on track:   {p.angle_on_track:.3f}")
    print(f"  Total progress:   {p.total_progress:.3f}")
    print(f"  Dist inner wall:  {p.dist_inner:.3f}")
    print(f"  Dist outer wall:  {p.dist_outer:.3f}")
    print(f"  Heading vs tang:  {p.heading_vs_tangent:.3f}")
    ray_str = "  ".join(
        f"{_RAY_LABELS[i]}:{p.ray_distances[i]:.1f}"
        for i in range(len(p.ray_distances))
    )
    print(f"  Raycasts:         {ray_str}")

    print("─── Opponent ───")
    print(f"  Lap: {o.lap}   Checkpoints: {o.checkpoints}   Rank: {o.rank}   Collided: {o.collided}")
    print(f"  Position:        ({o.pos_x:.3f}, {o.pos_y:.3f})")
    print(f"  Speed:            {o.speed:.3f}")
    print(f"  Direction:       ({o.dir_x:.3f}, {o.dir_y:.3f})")
    print(f"  Angle on track:   {o.angle_on_track:.3f}")
    print(f"  Total progress:   {o.total_progress:.3f}")
    print()
