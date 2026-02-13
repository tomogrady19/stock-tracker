# ============================
# Stage 1 — Build
# ============================
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy entire backend source
COPY . .

# Build
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release


# ============================
# Stage 2 — Runtime
# ============================
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libcurl4 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary from builder
COPY --from=builder /app/build /app/build

# Elastic Beanstalk sets PORT via env.
ENV PORT=8080

EXPOSE 8080

CMD ["./build/stockc"]