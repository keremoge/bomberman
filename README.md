# Bomberman (C++ / ncurses)

Terminal Bomberman for Linux (Alpine friendly).

## Alpine build
```sh
apk add g++ make ncurses-dev
make
./bomberman
```

## Docker
```sh
docker build -t bomberman .
docker run -it --rm bomberman
```

## Controls
Arrows / WASD move, SPACE drops a bomb (max 2), Q quits.
Destroy bricks (`%%`) and all enemies (`XX`) to clear a level.
