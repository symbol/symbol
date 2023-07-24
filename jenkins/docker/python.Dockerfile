ARG FROM_IMAGE='ubuntu:22.04'

FROM ${FROM_IMAGE}

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git curl

# install python
RUN apt-get install -y python3-pip python3-venv

# install shellcheck
RUN apt-get install -y shellcheck

# sdk dependencies
RUN apt-get install -y zbar-tools libssl-dev

# enable legacy providers in openssl3(ripemd160)
RUN if echo "$(openssl version)" |  grep -q "^OpenSSL 3"; then \
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

# add ubuntu user (used by jenkins)
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu
USER ubuntu
WORKDIR /home/ubuntu

# create a virtual environment, which is required by Ubuntu 23.04
ENV VIRTUAL_ENV=/home/ubuntu/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# install poetry and gitlint
RUN pip install poetry gitlint wheel
