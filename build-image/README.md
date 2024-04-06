# Cross-compiler

This build image is provided to ease building the server, as the Pi Zero 2s lack of memory and performance make it very
tedious.

## Build
Build the image using `docker build -t cross-compiler .`

## Troubleshooting
If you experience build issues, output in docker build is trimmed, the following command(s) should show all of it:
```bash
docker buildx create --use --name larger_log --driver-opt env.BUILDKIT_STEP_LOG_MAX_SIZE=50000000
docker buildx build --load -t cross-compiler --progress plain .
```
