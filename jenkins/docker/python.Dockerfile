FROM ubuntu:22.04

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git curl

# install python
RUN apt-get install -y python3-pip

# install shellcheck and gitlint
RUN apt-get install -y shellcheck \
	&& pip install gitlint

# install poetry
RUN pip install poetry

# sdk dependencies
RUN apt-get install -y zbar-tools libssl-dev

# enable legacy providers in openssl(ripemd160)
RUN sed -i '/^default = default_sect/a legacy = legacy_sect\n' /etc/ssl/openssl.cnf \
	&& sed -i '/^\[default_sect\]/i [legacy_sect]\nactivate = 1\n' /etc/ssl/openssl.cnf \
	&& sed -i 's/^# activate = 1/activate = 1/g' /etc/ssl/openssl.cnf \
	&& cat /etc/ssl/openssl.cnf

# codecov uploader
RUN curl -Os https://uploader.codecov.io/latest/linux/codecov \
	&& chmod +x codecov \
	&& mv codecov /usr/local/bin

# add ubuntu user (used by jenkins)
RUN useradd --uid 1000 -ms /bin/bash ubuntu
ENV PATH=$PATH:/home/ubuntu/.local/bin

WORKDIR /home/ubuntu
