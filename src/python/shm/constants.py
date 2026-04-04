"""
Shared memory layout constants — must match the C++ SharedGameData struct.
"""

SHM_NAME = "RacerGameSHM"
SHM_SIZE = 216
NUM_RAYCASTS = 7

# Byte offsets
OFF_INPUT = 0           # 4 × uint8: up, down, left, right
OFF_ACTION_READY = 4    # uint8
OFF_STATE_READY = 5     # uint8
OFF_RESET_FLAG = 6      # uint8
OFF_DONE_FLAG = 7       # uint8

OFF_PLAYER_U8 = 8       # 4 × uint8: lap, checkpoints, rank, collided
OFF_PLAYER_DBL = 16     # (10 + NUM_RAYCASTS) × double
OFF_OPP_U8 = 152        # 4 × uint8: same layout as player
OFF_OPP_DBL = 160       # 7 × double
