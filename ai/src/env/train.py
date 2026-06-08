from src.env.ZappyEnv import ZappyEnv
from stable_baselines3 import PPO
from stable_baselines3.common.env_checker import check_env


def main():
    print("Starting ZappyEnv")
    env = ZappyEnv(team_name="TeamAI")

    print("Verifying environment")
    check_env(env)
    print("Env checked")

    print("Implementing PPO")
    model = PPO("MlpPolicy", env, verbose=1)

    print("Training model")
    model.learn(total_timesteps=10000)

    model.save("zappy_ai_model")
    print("Finished and saved on zappy_ai_model.zip")

    env.close()


if __name__ == "__main__":
    main()
