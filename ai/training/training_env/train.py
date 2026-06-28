import argparse
import os
import multiprocessing
import time
import logging
from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import BaseCallback
from stable_baselines3.common.env_checker import check_env
from stable_baselines3.common.vec_env import SubprocVecEnv
from stable_baselines3.common.env_util import make_vec_env
from training.training_env.ZappyEnv import ZappyEnv

# Suppress verbose decision-making logs from teammate bots during training
logging.getLogger("zappy_ai").setLevel(logging.CRITICAL)


class TimestepCallback(BaseCallback):
    def __init__(self, total_timesteps, verbose=0):
        super(TimestepCallback, self).__init__(verbose)
        self.total_timesteps = total_timesteps
        self.max_level_seen = 1

    def _on_step(self) -> bool:
        if self.training_env is not None:
            levels = self.training_env.get_attr("player_level")
            if levels:
                current_max = max(levels)
                self.max_level_seen = max(self.max_level_seen, current_max)
                self.logger.record("rollout/max_level", current_max)

        # Print progress every 50 steps of the vector environment loop
        if self.n_calls % 50 == 0 or self.num_timesteps >= self.total_timesteps:
            remaining = max(0, self.total_timesteps - self.num_timesteps)
            print(
                f"[Training] Progress: {self.num_timesteps} / {self.total_timesteps} timesteps ({remaining} left) | Max Level: {self.max_level_seen}",
                end="\r",
                flush=True,
            )
            if self.n_calls % 500 == 0 or self.num_timesteps >= self.total_timesteps:
                print()  # Create a new line occasionally to preserve history
        return True


def main():
    """
    Main entry point for training the Zappy AI.
    Initializes the custom environment, verifies its compliance with Stable Baselines 3,
    and trains a Proximal Policy Optimization (PPO) model.
    """
    start_time = time.time()
    # Force the 'spawn' multiprocessing start method to ensure clean ctypes loading per process
    try:
        multiprocessing.set_start_method("spawn", force=True)
    except RuntimeError:
        pass

    parser = argparse.ArgumentParser(description="Train Zappy AI")
    parser.add_argument(
        "--timesteps", type=int, default=50000, help="Total timesteps to train"
    )
    parser.add_argument("--freq", type=int, default=100, help="Simulation frequency")
    parser.add_argument("--team", type=str, default="TeamAI", help="Team name")
    parser.add_argument(
        "--model-name",
        type=str,
        default="zappy_ai_model",
        help="Name of the saved model",
    )
    cpu_count = os.cpu_count()
    default_envs = max(1, cpu_count // 2) if cpu_count is not None else 2
    parser.add_argument(
        "--envs",
        type=int,
        default=default_envs,
        help=f"Number of parallel environments to run (default: {default_envs})",
    )
    parser.add_argument(
        "--load-model",
        type=str,
        default="",
        help="Name or path of a saved model to continue training from",
    )
    args = parser.parse_args()

    models_dir = os.path.abspath(
        os.path.join(os.path.dirname(os.path.realpath(__file__)), "../models")
    )
    os.makedirs(models_dir, exist_ok=True)

    print("Verifying single environment compliance")
    temp_env = ZappyEnv(
        use_lib=True,
        freq=args.freq,
        team_name=args.team,
    )
    check_env(temp_env)
    temp_env.close()
    print("Env checked")

    print(f"Creating {args.envs} parallel environment(s)")
    if args.envs > 1:
        env = make_vec_env(
            ZappyEnv,
            n_envs=args.envs,
            vec_env_cls=SubprocVecEnv,
            env_kwargs={
                "use_lib": True,
                "freq": args.freq,
                "team_name": args.team,
            },
        )
    else:
        env = make_vec_env(
            ZappyEnv,
            n_envs=1,
            env_kwargs={
                "use_lib": True,
                "freq": args.freq,
                "team_name": args.team,
            },
        )

    print("Implementing PPO")
    # Dynamically scale n_steps per env to keep total rollout batch size at ~2048.
    # SB3 PPO requires n_steps * n_envs >= batch_size (default: 64).
    n_steps = max(64 // args.envs + 1, 2048 // args.envs)
    n_steps = max(1, n_steps)
    print(
        f"PPO configuration: n_steps per env = {n_steps} (Total batch size: {n_steps * args.envs})"
    )
    if args.load_model:
        # Resolve path
        load_path = args.load_model
        if not os.path.dirname(load_path):
            load_path = os.path.join(models_dir, load_path)
        else:
            load_path = os.path.abspath(load_path)

        if not load_path.endswith(".zip") and not os.path.exists(load_path):
            if os.path.exists(load_path + ".zip"):
                load_path = load_path + ".zip"
        print(f"Loading existing model from: {load_path}")
        model = PPO.load(
            load_path,
            env=env,
            verbose=0,
            tensorboard_log="./tensorboard_logs/",
            custom_objects={"n_steps": n_steps},
        )
    else:
        model = PPO(
            "MlpPolicy",
            env,
            verbose=0,
            tensorboard_log="./tensorboard_logs/",
            n_steps=n_steps,
        )

    print(f"Training model for {args.timesteps} timesteps")

    # Execute the learning loop with the custom callback
    callback = TimestepCallback(total_timesteps=args.timesteps)
    model.learn(total_timesteps=args.timesteps, callback=callback)

    # Save the trained model weights to a zip file
    model_path = args.model_name
    if not os.path.dirname(model_path):
        model_path = os.path.join(models_dir, model_path)
    else:
        model_path = os.path.abspath(model_path)

    model.save(model_path)
    if not model_path.endswith(".zip"):
        model_path += ".zip"
    print(f"Finished and saved on {model_path}")

    elapsed_time = time.time() - start_time
    minutes = int(elapsed_time // 60)
    seconds = int(elapsed_time % 60)
    print(
        f"\n[Training] Total execution time: {minutes}m {seconds}s ({elapsed_time:.2f} seconds)"
    )

    env.close()


if __name__ == "__main__":
    main()
