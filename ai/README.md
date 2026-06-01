# AI

This is the AI project for the Multis - Zappy repository.

## Compile
### Prepare the build directory (recomended)
```bash
mkdir -p build && cd build
```
### Configure and compile
If inside build/
```bash
cmake ..
make
```
Else, from the root
```bash
cmake
make
```

### Run client
You can run ./zappy_ai from the root directory or ./build/zappy_ai
```bash
python3 client.py -p [port] -n [team_name] -ip [ip address]
```

### Run tests
From inside build/
```bash
ctest --output-on-failure
```
