import unittest
import os
from training.training_env.ZappyEnv import ZappyEnv


class TestZappyEnv(unittest.TestCase):
    def test_environment_lifecycle(self):
        print("\n[Test Env] Starting training_env")

        os.path.abspath(
            os.path.join(
                os.path.dirname(__file__), "../../cmake-build-debug/zappy_server"
            )
        )

        env = ZappyEnv(team_name="TeamAI")

        obs, info = env.reset()

        self.assertEqual(obs.shape, (657,), "The array doesn't have 657 slots")

        print("[Test Env] Playing 5 turns")
        for turn in range(5):
            action = env.action_space.sample()
            obs, reward, terminated, truncated, info = env.step(action)

            self.assertIsInstance(reward, float, "The reward should be a float")
            self.assertIsInstance(terminated, bool, "Terminated should be a boolean")

            if terminated:
                break

        print("[Test Env] Clossing conexions")
        env.close()


if __name__ == "__main__":
    unittest.main()
