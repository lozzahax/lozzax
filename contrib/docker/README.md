# Dockerfile for end users

For users who take comfort in containerization.

## Benefits

- Distro agnostic: run on any Linux distro with docker installed
- Security isolation: run untrusted software without fear
- Resource isolation: run lozzaxd with a specific amount of CPU and memory
- Simplicity: get up and running with only a few commands

## Quickstart

If you just want to build the container image and get lozzaxd running:

```
$ sh build_base_container.sh
$ sh build_lozzaxd_container.sh
$ docker run -d -p 22022:22022 --name lozzaxd lozzaxd:9.1.0
```

The blockchain and wallet won't be persisted beyond the containers lifetime, so you probably want to continue reading if
you want to run the container long-term.

## Usage details

### Building the container

`build_container.sh` builds a Docker container image based on Ubuntu 20.04 using the most-recent oxen pre-compiled binaries.
The `RELEASE` variable can be updated for any release version. Once built, the container image is stored locally:

```
$ docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
lozzaxd               9.1.0               17046241ccc2        3 minutes ago       159MB
```

The image can be used to run the lozzaxd container locally. The image can also be uploaded to a docker registry so that
other users can use it without needing to build it.

### Run the container

Run the container, storing the blockchain in Docker volume `oxen-blockchain`:

`$ docker run --name lozzaxd -d --mount "source=oxen-blockchain,target=/home/oxen/.oxen,type=volume" lozzaxd:9.1.0`

Same as above, but additionally mounting the wallet directory `/home/<your_user>/Oxen` to `/wallet` in the container:

```
$ docker run --name lozzaxd -d --mount "source=oxen-blockchain,target=/home/oxen/.oxen,type=volume" \
                             --mount "source=/home/<your_user>/Oxen,target=/wallet,type=bind" lozzaxd:9.1.0
```

Check on how the synchronization is going:

```
$ docker logs lozzaxd
```

#### Limiting resource usage

On a shared server, it may be useful to limit the amount of resources the lozzaxd container can consume. The recommended minimum
specs are 4G memory, 2 CPUs, and 30G disk. The memory and CPUs can be limited easily in the `run` command:

```
$ docker run --name lozzaxd -d --mount "source=oxen-blockchain,target=/home/oxen/.oxen,type=volume" \
                             --mount "source=/home/<your_user>/Oxen,target=/wallet,type=bind" \
			     --memory=4g --cpus=2 lozzaxd:9.1.0
```

Note: the blockchain sync speed will be impacted by limiting the CPU allocation.

The disk capacity is mostly used to store the blockchain. It's large, and will grow (slowly) over time. To prevent it from consuming
the OS partition, it is a good idea to use a separate partition for `/var/lib/docker`. Many VPS companies allow for attaching
easily-resizable volumes to a VPS; such a volume can be used to grow the volume as needed.

### Run the wallet CLI

These steps assume the lozzaxd container was started with your walled directory bind mounted to `/wallet` inside the container.

To run against your local lozzaxd:

`$ docker exec -it lozzaxd oxen-wallet-cli --wallet /wallet/wallets/<your_wallet_name>`

To run against a public daemon:

```
$ docker exec -it lozzaxd oxen-wallet-cli --wallet /wallet/wallets/<your_wallet_name> \
                                        --daemon-address public.loki.foundation:22023
```

### Stopping the container

Stop the container from running:

```
$ docker stop lozzaxd
```

After the container is no longer running, it can be deleted. Assuming you mounted the `oxen-blockchain` volume, the
blockchain will *not* be deleted:

```
$ docker rm lozzaxd
```

If you add the `--rm` flag to the `docker run` command, then the container will be automatically removed when it is stopped.

### Removing the oxen-blockchain volume

The `oxen-blockchain` volume has its own lifecycle, and can be mounted as newer versions of the container become available.
However, it can be deleted easily:

```
$ docker volume rm oxen-blockchain
```

Remember, you will need to resync the blockchain if you remove the `oxen-blockchain` volume.
