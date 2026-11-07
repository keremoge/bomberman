FROM alpine:3.20 AS build
RUN apk add --no-cache g++ make ncurses-dev
WORKDIR /src
COPY main.cpp Makefile ./
RUN make

FROM alpine:3.20
RUN apk add --no-cache ncurses libstdc++
COPY --from=build /src/bomberman /usr/local/bin/bomberman
ENV TERM=xterm-256color
CMD ["bomberman"]
