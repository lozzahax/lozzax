RELEASE=9.1.0
docker build -t lozzaxd:${RELEASE} -f Dockerfile.lozzaxd --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g) .
