import logging
import os
from datetime import datetime


class ColorFormatter(logging.Formatter):
    # ANSI escape codes for colors
    BLUE = "\x1b[34m"
    YELLOW = "\x1b[33m"
    RED = "\x1b[31m"
    GREEN = "\x1b[32m"
    RESET = "\x1b[0m"

    COLORS = {
        logging.DEBUG: BLUE,
        logging.INFO: GREEN,
        logging.WARNING: YELLOW,
        logging.ERROR: RED,
    }

    def format(self, record):
        log_color = self.COLORS.get(record.levelno, self.RESET)
        date_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # Format the level name with color
        level_name = f"{log_color}{record.levelname}{self.RESET}"

        header = f"[{date_str} {level_name:^16} zappy_ai]"

        return f"{header} {record.getMessage()}"


_level = getattr(
    logging, os.environ.get("ZAPPY_LOG_LEVEL", "DEBUG").upper(), logging.DEBUG
)

logger = logging.getLogger("zappy_ai")
logger.setLevel(_level)

ch = logging.StreamHandler()
ch.setLevel(_level)

ch.setFormatter(ColorFormatter())

if not logger.handlers:
    logger.addHandler(ch)
    logger.propagate = False
