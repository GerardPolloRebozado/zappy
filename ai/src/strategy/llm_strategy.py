import json
import os
import sys
import time
import traceback

from src.utils.logging_levels import logger

sys.path.insert(
    0, os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), ".."))
)

SYSTEM_PROMPT = """You are an autonomous AI playing the multiplayer Zappy game.
Your objectives are to survive by eating food and collect resources to elevate yourself to Level 8.

### Survival & Food Strategy
- You automatically consume food to survive.
- Check your inventory regularly. If your food count is less than 5, prioritize finding and taking food above all else. When the game starts you have 10
- If you see food on the current tile or a nearby tile, move to it and call `take_object` with 'food'.

### Elevation Requirements
To level up, you need a specific number of players on the same tile and specific resources dropped on that tile:
- Level 1 -> 2: 1 player, 1 linemate
- Level 2 -> 3: 2 players, 1 linemate, 1 deraumere, 1 sibur
- Level 3 -> 4: 2 players, 2 linemate, 1 sibur, 2 phiras
- Level 4 -> 5: 4 players, 1 linemate, 1 deraumere, 2 sibur, 1 phiras
- Level 5 -> 6: 4 players, 1 linemate, 2 deraumere, 1 sibur, 3 mendiane
- Level 6 -> 7: 6 players, 1 linemate, 2 deraumere, 3 sibur, 1 phiras
- Level 7 -> 8: 6 players, 2 linemate, 2 deraumere, 2 sibur, 2 mendiane, 2 phiras, 1 thystame

### Rules of Elevation (Very Important)
1. Gather all resources required for the next level in your inventory.
2. Go to a tile. Drop all the required resources for the elevation on that tile using `set_object`. Do NOT drop food.
3. If the elevation level requires more than 1 player, call `broadcast` (e.g. "Elevation level X") to summon teammates. You must wait for them to stand on the exact same tile as you.
4. Once players and resources are correct, call `incantation`. If it returns a level up, you succeeded. If it returns 'ko', something was missing.

### Grid Navigation and Look Response Format
The `look` tool returns a JSON string containing a list of lists of items representing tiles in your vision cone (e.g., `[["player", "food"], ["food"], [], ["linemate"]]`):
- Index 0: Current tile.
- Index 1 to 3: The row in front of you.
- Index 4 to 8: The row further in front, etc.
To move to tile index `i`, use the directions and step commands:
- Index 0: You are already on it.
- Index 1: Left, Forward.
- Index 2: Forward.
- Index 3: Right, Forward.
(Or construct a step-by-step movement pattern to get to other tiles in your vision).

*Note: To pick up/take an object (using `take_object`), you MUST be standing on the exact same tile as that object (Index 0). You cannot pick up objects from adjacent or distant tiles without moving to them first.*

### Inventory Response Format
The `inventory` tool returns a JSON object containing the quantities of all resources in your possession (e.g., `{"food": 10, "linemate": 2, "deraumere": 0, "sibur": 0, "mendiane": 0, "phiras": 0, "thystame": 0}`).

### Actions/Tools
You have tools to navigate, get inventory/look info, take or set objects, broadcast messages, check messages, and perform incantations.

Work step-by-step. In each turn, analyze your current inventory, check if you need food, look at the tiles around you, choose the optimal movement or action, and execute it. You can call multiple tools if appropriate (e.g. moving and then taking an item).
Dont waste the time looking at your inventory all the time, 1 food are 126 seconds to live as 1 time unit is 1 second
"""


def load_dotenv():
    script_dir = os.path.dirname(os.path.realpath(__file__))
    env_path = os.path.abspath(os.path.join(script_dir, "..", "..", ".env"))

    if os.path.exists(env_path):
        logger.info(f"Loading env vars from {env_path}")
        with open(env_path, "r") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                if "=" in line:
                    key, val = line.split("=", 1)
                    key = key.strip()
                    val = val.strip().strip("'\"")
                    os.environ[key] = val


class LLMProvider:
    def __init__(self, api_key, model_name=None, base_url=None):
        self.api_key = api_key
        self.model_name = model_name
        self.base_url = base_url or "https://openrouter.ai/api/v1"
        self._init_client()

    def _init_client(self):
        import openai

        self.client = openai.OpenAI(
            base_url=self.base_url,
            api_key=self.api_key,
            default_headers={
                "HTTP-Referer": "https://github.com/GerardPolloRebozado/zappy",
                "X-Title": "Zappy LLM Player",
            },
        )

    def query(self, messages, tools):
        api_messages = [{"role": "system", "content": SYSTEM_PROMPT}]
        for m in messages:
            api_messages.append(m)

        resp = self.client.chat.completions.create(
            model=self.model_name,
            messages=api_messages,
            tools=tools,
            tool_choice="auto" if tools else None,
            temperature=0.2,
        )

        choice = resp.choices[0].message
        tool_calls = []
        if choice.tool_calls:
            for tc in choice.tool_calls:
                args = tc.function.arguments
                if isinstance(args, str):
                    try:
                        args = json.loads(args)
                    except json.JSONDecodeError:
                        pass
                tool_calls.append(
                    {
                        "id": tc.id,
                        "type": "function",
                        "function": {"name": tc.function.name, "arguments": args},
                    }
                )
        return choice.content or "", tool_calls


def select_provider():
    load_dotenv()

    api_key = os.environ.get("OPENROUTER_API_KEY")
    model_name = os.environ.get("MODEL_NAME")
    base_url = os.environ.get("LLM_BASE_URL") or "https://openrouter.ai/api/v1"

    if not api_key:
        raise RuntimeError(
            "OpenRouter requires OPENROUTER_API_KEY or OPENAI_API_KEY env var in .env."
        )

    return LLMProvider(api_key, model_name, base_url)


def get_tools_definition():
    return [
        {
            "type": "function",
            "function": {
                "name": "forward",
                "description": "Move the player forward one tile.",
                "parameters": {"type": "object", "properties": {}},
            },
        },
        {
            "type": "function",
            "function": {
                "name": "right",
                "description": "Turn the player 90 degrees right.",
                "parameters": {"type": "object", "properties": {}},
            },
        },
        {
            "type": "function",
            "function": {
                "name": "left",
                "description": "Turn the player 90 degrees left.",
                "parameters": {"type": "object", "properties": {}},
            },
        },
        {
            "type": "function",
            "function": {
                "name": "look",
                "description": "Look around the current and visible tiles in front of you. Returns a list of lists of items. the view is a cone and increses with your incantation level, tiles are separated by , and items by spaces so if there are two items with just a space it means its the same tile, the first tile is the tile you are standing on",
                "parameters": {"type": "object", "properties": {}},
            },
        },
        {
            "type": "function",
            "function": {
                "name": "inventory",
                "description": "Check the player's inventory counts (food, linemate, deraumere, sibur, mendiane, phiras, thystame).",
                "parameters": {"type": "object", "properties": {}},
            },
        },
        {
            "type": "function",
            "function": {
                "name": "take_object",
                "description": "Pick up an object from the current tile.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "name": {
                            "type": "string",
                            "enum": [
                                "food",
                                "linemate",
                                "deraumere",
                                "sibur",
                                "mendiane",
                                "phiras",
                                "thystame",
                            ],
                            "description": "Name of the object to take",
                        }
                    },
                    "required": ["name"],
                },
            },
        },
        {
            "type": "function",
            "function": {
                "name": "set_object",
                "description": "Drop/set down an object on the current tile.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "name": {
                            "type": "string",
                            "enum": [
                                "food",
                                "linemate",
                                "deraumere",
                                "sibur",
                                "mendiane",
                                "phiras",
                                "thystame",
                            ],
                            "description": "Name of the object to drop",
                        }
                    },
                    "required": ["name"],
                },
            },
        },
        {
            "type": "function",
            "function": {
                "name": "broadcast",
                "description": "Broadcast a text message to all other players.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "text": {
                            "type": "string",
                            "description": "The message to broadcast",
                        }
                    },
                    "required": ["text"],
                },
            },
        },
        {
            "type": "function",
            "function": {
                "name": "connect_nbr",
                "description": "Get the number of unused slots for this team.",
                "parameters": {"type": "object", "properties": {}},
            },
        },
        {
            "type": "function",
            "function": {
                "name": "fork",
                "description": "Fork a player connection (lay an egg).",
                "parameters": {"type": "object", "properties": {}},
            },
        },
        {
            "type": "function",
            "function": {
                "name": "eject",
                "description": "Eject other players from the current tile.",
                "parameters": {"type": "object", "properties": {}},
            },
        },
        {
            "type": "function",
            "function": {
                "name": "incantation",
                "description": "Start the elevation to level up.",
                "parameters": {"type": "object", "properties": {}},
            },
        },
    ]


def run_llm(client):
    """
    Main loop to handle server communication using an LLM via OpenRouter.
    :param client: The AI client instance
    :return: None
    """
    try:
        provider = select_provider()
    except Exception as e:
        logger.error(f"Failed to initialize LLM provider: {e}")
        traceback.print_exc()
        return

    logger.info(f"Starting LLM Loop using OpenRouter (Model: {provider.model_name})")

    messages = []
    turn = 1

    def execute_tool(name, args):
        if client.is_dead:
            return "dead"

        try:
            if name == "forward":
                return str(client.forward())
            elif name == "right":
                return str(client.right())
            elif name == "left":
                return str(client.left())
            elif name == "look":
                return json.dumps(client.look())
            elif name == "inventory":
                res = client.inventory()
                if hasattr(res, "__dict__"):
                    return json.dumps(res.__dict__)
                return str(res)
            elif name == "take_object":
                return str(client.take(args.get("name")))
            elif name == "set_object":
                return str(client.set(args.get("name")))
            elif name == "broadcast":
                return str(client.broadcast(args.get("text")))
            elif name == "connect_nbr":
                return str(client.connect_nbr())
            elif name == "fork":
                return str(client.fork())
            elif name == "eject":
                return str(client.eject())
            elif name == "incantation":
                return str(client.incantation())
            else:
                return f"Error: Unknown tool {name}"
        except Exception as e:
            return f"Error: {e}"

    tools = get_tools_definition()

    try:
        while not client.is_dead:
            logger.info(f"\n--- Turn {turn} (Level: {client.level}) ---")

            # Check for async game events/messages and feed them to LLM context
            if client.messages:
                incoming = []
                while client.messages:
                    msg = client.messages.pop(0)
                    logger.info(
                        f"Asynchronous Broadcast from {msg['direction']}: {msg['text']}"
                    )
                    incoming.append(f"From direction {msg['direction']}: {msg['text']}")

                messages.append(
                    {
                        "role": "user",
                        "content": "[SYSTEM NOTICE] You received the following broadcasts:\n"
                        + "\n".join(incoming),
                    }
                )

            # Query the LLM
            logger.info("Querying LLM for next action...")
            try:
                text_content, tool_calls = provider.query(messages, tools)
            except Exception as e:
                logger.error(f"LLM query error: {e}")
                traceback.print_exc()
                time.sleep(2)
                continue

            if text_content:
                logger.info(f"LLM Thoughts: {text_content}")
                messages.append({"role": "assistant", "content": text_content})

            if not tool_calls:
                logger.warning(
                    "LLM returned no actions. Forcing a look to stay active."
                )
                tool_calls = [
                    {
                        "id": "force_look",
                        "type": "function",
                        "function": {"name": "look", "arguments": {}},
                    }
                ]

            for tool_call in tool_calls:
                tool_name = tool_call["function"]["name"]
                args = tool_call["function"].get("arguments", {})

                logger.info(f"Executing tool: {tool_name} with arguments: {args}")
                result = execute_tool(tool_name, args)
                logger.info(f"Tool response: {result}")

                messages.append(
                    {
                        "role": "tool",
                        "name": tool_name,
                        "tool_call_id": tool_call["id"],
                        "content": result,
                    }
                )

                if result == "dead" or client.is_dead:
                    logger.error("Player has died!")
                    break

            if len(messages) > 30:
                messages = messages[-20:]

            turn += 1
            time.sleep(0.5)

    except KeyboardInterrupt:
        logger.info("Interrupt received, exiting loop.")
    finally:
        client.close()
