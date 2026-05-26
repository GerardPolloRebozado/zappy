SERVER_BIN = myteams_server
CLI_BIN = myteams_cli
PWD := $(shell pwd)
LDFLAGS = -L $(PWD)/libs/myteams -l myteams
all: build
build:
	RUSTFLAGS="$(LDFLAGS) -C rpath=no" cargo build --bin $(SERVER_BIN)
	RUSTFLAGS="$(LDFLAGS) -C rpath=no" cargo build --bin $(CLI_BIN)
	cp target/debug/$(SERVER_BIN) .
	cp target/debug/$(CLI_BIN) .

re: fclean build

docs:
	typst compile docs/main.typ

clean:
	@cargo clean

fclean: clean
	@rm -rf bin/
	@rm -rf docs/main.pdf

.PHONY: docs build clean fclean re


