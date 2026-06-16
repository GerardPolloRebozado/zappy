import time
from stable_baselines3 import PPO
from training.training_env.ZappyEnv import ZappyEnv


def main():
    model = PPO.load("zappy_ai_model")

    env = ZappyEnv(port=8080, team_name="TeamAI")

    obs, info = env.reset()
    done = False

    print("10 seconds to open the gui")

    time.sleep(10)

    turns = 0
    final_points = 0

    while not done:
        action, _states = model.predict(obs, deterministic=True)

        obs, reward, terminated, truncated, info = env.step(action)

        final_points += reward
        turns += 1
        done = terminated or truncated

    print(f"\nEnd game. Turns survived: {turns} | Points: {final_points}")
    env.close()


if __name__ == "__main__":
    main()
