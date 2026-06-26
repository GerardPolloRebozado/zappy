from enum import Enum


class BroadcastDict(Enum):
    COME = "COME"
    FIND = "FIND"
    DONT_COME = "DCOME"
    INCANT = "INCANT"


class BroadcastHandler:
    def __init__(self, team_name, secret_key):
        self.team_name = team_name
        self.secret_key = secret_key

    def build_message(self, intent: BroadcastDict, params: str = "") -> str:
        """Build a safe message using the team's secret key."""
        return f"{self.team_name}|{self.secret_key}|{intent.value}|{params}"

    def parse_message(self, raw_message: str):
        """Decypher the message to ensure it belongs to our team."""
        parts = raw_message.split("|")
        if (
            len(parts) >= 3
            and parts[0] == self.team_name
            and parts[1] == self.secret_key
        ):
            return parts[2], parts[3] if len(parts) > 3 else ""
        return None, None

    def calculate_heuristic(self, direction: int, raw_message: str) -> dict:
        """Heuristic calculations to determine the importance of the broadcast."""
        action, params = self.parse_message(raw_message)

        if not action:
            return {"score": 0, "task": "IGNORE"}

        match action:
            case BroadcastDict.INCANT.value:
                return {"score": 100, "task": "MOVE_TO_DIR", "dir": direction}
            case BroadcastDict.COME.value:
                return {"score": 70, "task": "MOVE_TO_DIR", "dir": direction}
            case BroadcastDict.DONT_COME.value:
                return {"score": 50, "task": "FLEE_FROM_DIR", "dir": direction}
            case BroadcastDict.FIND.value:
                return {"score": 30, "task": "SEARCH_RESOURCE", "target": params}

        return {"score": 0, "task": "IGNORE"}
