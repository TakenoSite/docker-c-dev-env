docker build -t c-dev-env .

#linux, mac = PWD => pwd

docker run -it --rm -v "${PWD}:/workspace" c-dev-env
