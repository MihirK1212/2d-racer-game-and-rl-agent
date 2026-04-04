"""
Usage:
    1. Start the C++ game (externalInputMode=true; stepMode=true for
       deterministic replay, or stepMode=false for real-time at 60fps)
    2. python play.py                           # defaults to models/racer_ppo
       python play.py --model models/racer_ppo  # explicit path
"""

import argparse

from stable_baselines3 import PPO
from env import RacerEnv

def main():
    parser = argparse.ArgumentParser(description="Play with a trained model")
    parser.add_argument("--model", type=str, default="models/racer_ppo",
                        help="Path to saved model (without .zip)")
    parser.add_argument("--episodes", type=int, default=10,
                        help="Number of episodes to run")
    parser.add_argument("--max-ep-steps", type=int, default=5000)
    args = parser.parse_args()

    env = RacerEnv(max_episode_steps=args.max_ep_steps)
    model = PPO.load(args.model)

    print(f"Loaded model from {args.model}")
    print(f"Running {args.episodes} episode(s) ...\n")

    for ep in range(args.episodes):
        obs, _ = env.reset()
        total_reward = 0.0
        steps = 0

        while True:
            action, _ = model.predict(obs, deterministic=True)
            obs, reward, terminated, truncated, info = env.step(int(action))
            total_reward += reward
            steps += 1

            if terminated or truncated:
                print(
                    f"Episode {ep + 1:>3}:  "
                    f"reward={total_reward:>8.1f}  "
                    f"steps={steps:>5}  "
                    f"laps={info.get('lap', '?')}  "
                    f"progress={info.get('total_progress', 0):.0f}deg"
                )
                break

    env.close()


if __name__ == "__main__":
    main()
