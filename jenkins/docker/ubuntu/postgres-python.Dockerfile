FROM postgres:15-bookworm

# install dependencies (install tzdata first to prevent 'geographic area' prompt)
RUN apt-get update \
	&& apt-get install -y ca-certificates curl git libpq-dev libssl-dev python3-pip python3-venv shellcheck tzdata \
	&& update-ca-certificates

# codecov uploader
RUN ARCH=$([ "$(uname -m)" = "x86_64" ] && echo "linux" || echo "aarch64") \
	&& curl -Os "https://uploader.codecov.io/latest/${ARCH}/codecov" \
	&& chmod +x codecov \
	&& mv codecov /usr/local/bin

# add ubuntu user
RUN useradd --uid 1000 -ms /bin/bash ubuntu
ENV PATH=$PATH:/home/ubuntu/.local/bin
USER ubuntu
WORKDIR /home/ubuntu

# create a virtual environment
ENV VIRTUAL_ENV=/home/ubuntu/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

RUN pip install gitlint
