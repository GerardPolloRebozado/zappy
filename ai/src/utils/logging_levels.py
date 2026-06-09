import logging

# Default format for logging
LOG_FORMAT = "%(levelname)-8s %(message)s"

# Setup the logging configuration globally
logging.basicConfig(level=logging.DEBUG, format=LOG_FORMAT)

logger = logging.getLogger("zappy_ai")
