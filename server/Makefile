SERVER_BIN = zappy_server
CLI_BIN = zappy_cli
PWD := $(shell pwd)

all: build
build:
	cargo build --bin $(SERVER_BIN)
	cargo build --bin $(CLI_BIN)
	cp target/debug/$(SERVER_BIN) .
	cp target/debug/$(CLI_BIN) .

re: fclean build

clean:
	@cargo clean

fclean: clean
	@rm -f $(SERVER_BIN)
	@rm -f $(CLI_BIN)

.PHONY: build clean fclean re


