ARG FROM_IMAGE='fedora:40'

FROM ${FROM_IMAGE}

RUN dnf update --assumeyes && dnf install --assumeyes git curl which

# nodejs
ENV NODE_OPTIONS="--dns-result-order=ipv4first"
RUN dnf install --assumeyes nodejs

# dependencies
RUN dnf install --assumeyes make gcc g++ diffutils

# install python
RUN dnf install --assumeyes python3 python3-pip

# install shellcheck
RUN dnf install --assumeyes shellcheck

# rust dependencies - https://docs.rs/crate/openssl-sys/0.9.19
RUN dnf install --assumeyes openssl openssl-devel pkg-config \
# there is no aarch64 build of binaryen -  https://github.com/WebAssembly/binaryen/issues/5337
	&& if [ "$(uname -m)" = "aarch64" ]; then dnf install --assumeyes binaryen; fi

# codecov uploader
RUN ARCH=$([ "$(uname -m)" = "x86_64" ] && echo "linux" || echo "aarch64") \
	&& curl -Os "https://uploader.codecov.io/latest/${ARCH}/codecov" \
	&& chmod +x codecov \
	&& mv codecov /usr/local/bin

# add fedora user (used by jenkins)
RUN id -u "fedora" || useradd --uid 1000 -ms /bin/bash fedora

# install rust and wasm-pack
ENV PATH=$PATH:/home/fedora/.cargo/bin
ENV CARGO_HOME=/home/fedora/.cargo
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y \
	&& curl https://rustwasm.github.io/wasm-pack/installer/init.sh -sSf | bash -s -- \
	&& chown -R fedora:fedora /home/fedora/.cargo

USER fedora
WORKDIR /home/fedora

# create a virtual environment
ENV VIRTUAL_ENV=/home/fedora/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# install common python packages
RUN python3 -m pip install --upgrade gitlint isort lark pycodestyle pylint PyYAML
