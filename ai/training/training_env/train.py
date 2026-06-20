import argparse
from training.training_env.ZappyEnv import ZappyEnv
from stable_baselines3 import PPO
from stable_baselines3.common.env_checker import check_env


from stable_baselines3.common.callbacks import BaseCallback


class TimestepCallback(BaseCallback):
    def __init__(self, total_timesteps, verbose=0):
        super(TimestepCallback, self).__init__(verbose)
        self.step_count = 0
        self.total_timesteps = total_timesteps

    def _on_step(self) -> bool:
        self.step_count += 1
        if self.step_count % 100 == 0 or self.step_count == self.total_timesteps:
            remaining = max(0, self.total_timesteps - self.step_count)
            print(
                f"[Training] Progress: {self.step_count} / {self.total_timesteps} timesteps ({remaining} left)",
                end="\r",
                flush=True,
            )
            if self.step_count % 1000 == 0 or self.step_count == self.total_timesteps:
                print()  # Create a new line occasionally to preserve history
        return True


def main():
    """
    Main entry point for training the Zappy AI.
    Initializes the custom environment, verifies its compliance with Stable Baselines 3,
    and trains a Proximal Policy Optimization (PPO) model.
    """
    parser = argparse.ArgumentParser(description="Train Zappy AI")
    parser.add_argument(
        "--timesteps", type=int, default=50000, help="Total timesteps to train"
    )
    parser.add_argument("--width", type=int, default=10, help="Map width")
    parser.add_argument("--height", type=int, default=10, help="Map height")
    parser.add_argument("--freq", type=int, default=100, help="Simulation frequency")
    parser.add_argument("--team", type=str, default="TeamAI", help="Team name")
    parser.add_argument(
        "--model-name",
        type=str,
        default="zappy_ai_model",
        help="Name of the saved model",
    )
    args = parser.parse_args()

    print(f"Starting ZappyEnv with {args.width}x{args.height} map, freq {args.freq}")
    env = ZappyEnv(
        use_lib=True,
        width=args.width,
        height=args.height,
        freq=args.freq,
        team_name=args.team,
    )

    print("Verifying environment")
    check_env(env)
    print("Env checked")

    print("Implementing PPO")
    model = PPO("MlpPolicy", env, verbose=0)

    print(f"Training model for {args.timesteps} timesteps")

    # Execute the learning loop with the custom callback
    callback = TimestepCallback(total_timesteps=args.timesteps)
    model.learn(total_timesteps=args.timesteps, callback=callback)

    # Save the trained model weights to a zip file
    model.save(args.model_name)
    print(f"Finished and saved on {args.model_name}.zip")

    env.close()


if __name__ == "__main__":
    main()
