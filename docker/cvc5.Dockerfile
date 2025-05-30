ARG PLATFORM=linux/arm64
FROM --platform=${PLATFORM} voidlinux/voidlinux-musl:latest AS base

ENV CFLAGS=-march=x86-64 -O2
ENV CXXFLAGS=$CFLAGS
RUN echo noextract=/etc/hosts > /etc/xbps.d/test.conf
RUN echo "repository=https://repo-default.voidlinux.org/current/musl" > /etc/xbps.d/00-repository-main.conf
RUN xbps-install -ySu xbps
RUN xbps-install -ySu
RUN xbps-install -yS autoconf bash cln-devel cmake gcc git libtool make python3-devel python3-pip texinfo


FROM base AS cvc5

RUN xbps-install -yS 
ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
RUN pip install tomli pyparsing

COPY ../docker/loat-base-image/usr/local/lib/libpoly.a /usr/local/lib/
COPY ../docker/loat-base-image/usr/local/lib/libpolyxx.a /usr/local/lib/
COPY ../docker/loat-base-image/usr/local/include/poly /usr/local/include/poly

RUN git clone https://github.com/cvc5/cvc5.git
WORKDIR cvc5
RUN git checkout cvc5-1.1.2
RUN ./configure.sh --static --no-statistics --auto-download --poly --cln --gpl --no-docs
WORKDIR /cvc5/build
RUN make -j$(nproc)
RUN make install