# Use the Alpine Linux image
FROM alpine:latest

# Set the working directory
WORKDIR /root
RUN apk add binutils
# Run a shell by default
CMD ["/bin/ash"]

