"""
Train a PPO agent to drive in the 2D racer game.

Usage:
    1. Start the C++ game with externalInputMode=true, stepMode=true
    2. python train.py
    3. Monitor with:  tensorboard --logdir runs/

The trained model is saved to models/racer_ppo.zip (+ periodic checkpoints).
"""

import os
import argparse

from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import CheckpointCallback, BaseCallback
from env import RacerEnv


class EpisodeLogCallback(BaseCallback):
    """Prints a one-liner per episode so you can watch training progress."""

    def __init__(self, verbose=0):
        super().__init__(verbose)
        self._ep_count = 0

    def _on_step(self) -> bool:
        for info in self.locals.get("infos", []):
            if "episode" in info:
                self._ep_count += 1
                ep = info["episode"]
                extra = f"  progress={info.get('total_progress', 0):.0f}deg" if "total_progress" in info else ""
                print(
                    f"  ep {self._ep_count:>5}  "
                    f"reward={ep['r']:>8.1f}  "
                    f"len={ep['l']:>5}{extra}"
                )
        return True


def main():
    parser = argparse.ArgumentParser(description="Train PPO on the 2D Racer")
    parser.add_argument("--timesteps", type=int, default=500_000,
                        help="Total training timesteps")
    parser.add_argument("--save-path", type=str, default="models/racer_ppo",
                        help="Path to save final model (without .zip)")
    parser.add_argument("--log-dir", type=str, default="runs/",
                        help="TensorBoard log directory")
    parser.add_argument("--max-ep-steps", type=int, default=5000,
                        help="Max steps per episode (truncation limit)")
    parser.add_argument("--checkpoint-freq", type=int, default=50_000,
                        help="Save a checkpoint every N timesteps")
    parser.add_argument("--resume", type=str, default=None,
                        help="Path to a model .zip to resume training from")
    args = parser.parse_args()

    env = RacerEnv(max_episode_steps=args.max_ep_steps)

    if args.resume:
        print(f"Resuming training from {args.resume}")
        model = PPO.load(args.resume, env=env, tensorboard_log=args.log_dir)
    else:
        model = PPO(
            "MlpPolicy",
            env,
            verbose=1,
            tensorboard_log=args.log_dir,
            learning_rate=3e-4,
            n_steps=2048,
            batch_size=64,
            n_epochs=10,
            gamma=0.99,
            clip_range=0.2,
        )

    checkpoint_dir = os.path.join(os.path.dirname(args.save_path), "checkpoints")
    callbacks = [
        CheckpointCallback(
            save_freq=args.checkpoint_freq,
            save_path=checkpoint_dir,
            name_prefix="racer_ppo",
        ),
        EpisodeLogCallback(),
    ]

    print(f"Training for {args.timesteps:,} timesteps ...")
    print(f"TensorBoard:  tensorboard --logdir {args.log_dir}")
    print()

    model.learn(
        total_timesteps=args.timesteps,
        callback=callbacks,
        progress_bar=True,
    )

    os.makedirs(os.path.dirname(args.save_path) or ".", exist_ok=True)
    model.save(args.save_path)
    print(f"\nModel saved to {args.save_path}.zip")

    env.close()


if __name__ == "__main__":
    main()
