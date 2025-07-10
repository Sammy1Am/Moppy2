FROM jlesage/baseimage-gui:ubuntu-22.04-v4@sha256:418464ab511c69dddb1158ac70116a7e8a6b61607e15f070e514262a10180686

ENV LANG=en_US.UTF-8
ENV DARK_MODE=1
ENV KEEP_APP_RUNNING=1
ENV USER_ID=1000
ENV GROUP_ID=1000

RUN \
  echo "*** Install system packages ***" && \
  apt-get update -y && \
  apt-get install -y --no-install-recommends \
    default-jre \
    libxrender1 \
    libxtst6 \
    libxi6 \
    wget \
    unzip \
    ca-certificates \
    language-pack-en && \
  echo "*** Setup Application ***" && \
  set-cont-env APP_NAME "Moppy2" && \
  mkdir /moppy2 && \
  wget -P /tmp/ https://github.com/Sammy1Am/Moppy2/releases/latest/download/MoppyControlGUI-2.1.0.zip && \
  unzip /tmp/MoppyControlGUI-2.1.0.zip && \
  mv MoppyControlGUI-2.1.0/* /moppy2/ && \
  echo "*** Link folders and files ***" && \
  take-ownership /moppy2 && \
  echo "*** cleanup ***" && \
  apt-get -y autoremove && \
  rm -rf \
    /tmp/* \
    /var/lib/apt/lists/* \
    /var/tmp/* \
    /root/.cache

COPY --chmod=555 docker/startapp.sh /startapp.sh

EXPOSE 5800
WORKDIR /moppy2
