from training.training_env.ZappyEnv import ZappyEnv
from stable_baselines3 import PPO
from stable_baselines3.common.env_checker import check_env


def main():
    """
    Main entry point for training the Zappy AI.
    Initializes the custom environment, verifies its compliance with Stable Baselines 3,
    and trains a Proximal Policy Optimization (PPO) model.
    """
    print("Starting ZappyEnv")
    env = ZappyEnv(team_name="TeamAI")

    print("Verifying environment")
    check_env(env)
    print("Env checked")

    print("Implementing PPO")
    model = PPO("MlpPolicy", env, verbose=1)

    print("Training model")
    # Execute the learning loop for the specified number of timesteps
    model.learn(total_timesteps=1000000)

    # Save the trained model weights to a zip file
    model.save("zappy_ai_model")
    print("Finished and saved on zappy_ai_model.zip")

    env.close()


if __name__ == "__main__":
    main()
