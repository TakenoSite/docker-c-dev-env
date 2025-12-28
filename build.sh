docker build -t c-dev-env .
#linux, mac = PWD => pwd
#docker run -it --rm -v "${PWD}:/workspace" c-dev-env
docker run -it -p 8080:8080 -p 22:22 -p 12345:12345/UDP --rm -v "${PWD}:/workspace" -d c-dev-env
