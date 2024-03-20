ARG FROM_IMAGE='ubuntu:22.04'

FROM ${FROM_IMAGE}

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git curl

# mongodb 6.0
RUN apt-get install -y wget gnupg \
	&& wget -qO - https://www.mongodb.org/static/pgp/server-6.0.asc |  gpg --dearmor | tee /usr/share/keyrings/mongodb.gpg > /dev/null \
	&& echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb.gpg ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/6.0 multiverse"  \
	| tee /etc/apt/sources.list.d/mongodb-org-6.0.list \
	&& apt-get update \
	&& apt-get install -y mongodb-org

# nodejs
ENV NODE_OPTIONS="--dns-result-order=ipv4first"
ARG NODEJS_VERSION=20
RUN apt-get install -y ca-certificates curl gnupg \
	&& mkdir -p /etc/apt/keyrings \
	&& curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
	&& echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_${NODEJS_VERSION}.x nodistro main" \
	| tee /etc/apt/sources.list.d/nodesource.list \
	&& apt-get update \
	&& apt-get install -y nodejs

# rest dependencies
RUN apt-get install -y make gcc g++

# install python
RUN apt-get install -y python3-pip python3-venv

# install shellcheck
RUN apt-get install -y shellcheck

# rust dependencies - https://docs.rs/crate/openssl-sys/0.9.19
RUN apt-get install -y libssl-dev pkg-config \
# there is no aarch64 build of binaryen -  https://github.com/WebAssembly/binaryen/issues/5337
	&& if [ "$(uname -m)" = "aarch64" ]; then apt-get install -y binaryen; fi

# codecov uploader
RUN ARCH=$([ "$(uname -m)" = "x86_64" ] && echo "linux" || echo "aarch64") \
	&& curl -Os "https://uploader.codecov.io/latest/${ARCH}/codecov" \
	&& chmod +x codecov \
	&& mv codecov /usr/local/bin

# add ubuntu user (used by jenkins)
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu

# Create the MongoDB data directory
RUN mkdir -p /data/db \
	&& chown -R ubuntu:ubuntu /data

# install rust and wasm-pack
ENV PATH=$PATH:/home/ubuntu/.cargo/bin
ENV CARGO_HOME=/home/ubuntu/.cargo 
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y \
	&& curl https://rustwasm.github.io/wasm-pack/installer/init.sh -sSf | bash -s -- \
	&& chown -R ubuntu:ubuntu /home/ubuntu/.cargo \
	&& rustup default stable

USER ubuntu
WORKDIR /home/ubuntu

# create a virtual environment, which is required by Ubuntu 23.04
ENV VIRTUAL_ENV=/home/ubuntu/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# install common python packages
RUN python3 -m pip install --upgrade gitlint isort lark pycodestyle pylint PyYAML
