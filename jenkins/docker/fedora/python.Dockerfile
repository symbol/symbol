ARG FROM_IMAGE='fedora:40'

FROM ${FROM_IMAGE}

RUN dnf update --assumeyes && dnf install --assumeyes git curl

# install python
RUN dnf install --assumeyes python3 python3-pip python3-virtualenv python3-devel

# install shellcheck
RUN dnf install --assumeyes shellcheck

# sdk dependencies
RUN dnf install --assumeyes zbar openssl openssl-devel make gcc g++ diffutils

# enable legacy providers in openssl3(ripemd160)
RUN if echo "$(openssl version)" | grep -q "^OpenSSL 3"; then \
		sed -i '/^default = default_sect/a legacy = legacy_sect\n' /etc/ssl/openssl.cnf \
		&& sed -i '/^\[default_sect\]/i [legacy_sect]\nactivate = 1\n' /etc/ssl/openssl.cnf \
		&& sed -i 's/^# activate = 1/activate = 1/g' /etc/ssl/openssl.cnf \
		&& cat /etc/ssl/openssl.cnf; \
	fi

# codecov uploader
RUN ARCH=$([ "$(uname -m)" = "x86_64" ] && echo "linux" || echo "aarch64") \
	&& curl -Os "https://uploader.codecov.io/latest/${ARCH}/codecov" \
	&& chmod +x codecov \
	&& mv codecov /usr/local/bin

# add fedora user (used by jenkins)
RUN id -u "fedora" || useradd --uid 1000 -ms /bin/bash fedora
USER fedora
WORKDIR /home/fedora

# create a virtual environment
ENV VIRTUAL_ENV=/home/fedora/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# install poetry and gitlint
RUN python3 -m pip install poetry gitlint wheel setuptools

# install common python packages
RUN python3 -m pip install --upgrade aiohttp colorama coverage cryptography Jinja2 lark ply Pillow pynacl pytest PyYAML pyzbar requests \
	ripemd-hash safe-pysha3 websockets zenlog
