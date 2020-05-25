FROM l.gcr.io/google/bazel:1.2.1

RUN apt update && \
    apt install bzip2 libxml2 -y

RUN curl -sL https://deb.nodesource.com/setup_13.x | bash - && \
    apt install -y nodejs

RUN apt install expect-dev -y

RUN apt install clang-format clang -y

COPY build-filter.sh /build-filter.sh

ENTRYPOINT /build-filter.sh
