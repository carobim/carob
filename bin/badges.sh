#!/bin/bash

printf 'Windows '
if [ "$(curl -s https://ci.appveyor.com/api/projects/status/github/carobim/carob\?svg=true | shasum --algorithm 1)" = '585d5778186eee4b196350534e1b80e3c5019d51  -' ]; then
    echo ok
else
    echo err https://ci.appveyor.com/project/pmer/carob
fi

printf 'macOS '
if [ "$(curl -s https://api.travis-ci.com/carobim/carob.svg | shasum --algorithm 1)" = '6a7e16d9fbe5b315f689672a9e5b02a94834961a  -' ]; then
    echo ok
else
    echo err https://travis-ci.com/carobim/carob
fi

printf 'Linux '
if [ "$(curl -s https://circleci.com/gh/carobim/carob.svg\?style=shield | shasum --algorithm 1)" = 'bdf7c7df9edb3fe2cde22a66533d700ef3dd9e77  -' ]; then
    echo ok
else
    echo err https://circleci.com/gh/carobim/carob
fi

printf 'FreeBSD 12 '
if [ "$(curl -s https://builds.sr.ht/~pdm/carob/commits/freebsd-12.yml.svg | shasum --algorithm 1)" = '01d9def108b260716a2a9d77fd70617f1e0065b2  -' ]; then
    echo ok
else
    echo err https://builds.sr.ht/~pdm/carob/commits/freebsd-12.yml
fi

printf 'FreeBSD 13 '
if [ "$(curl -s https://builds.sr.ht/~pdm/carob/commits/freebsd-13.yml.svg | shasum --algorithm 1)" = '01d9def108b260716a2a9d77fd70617f1e0065b2  -' ]; then
    echo ok
else
    echo err https://builds.sr.ht/~pdm/carob/commits/freebsd-13.yml
fi

printf 'NetBSD 9 '
if [ "$(curl -s https://builds.sr.ht/~pdm/carob/commits/netbsd-9.yml.svg | shasum --algorithm 1)" = '01d9def108b260716a2a9d77fd70617f1e0065b2  -' ]; then
    echo ok
else
    echo err https://builds.sr.ht/~pdm/carob/commits/netbsd-9.yml
fi

printf 'SLoC '
curl -s https://tokei.rs/b1/github/carobim/carob\?category=code | sed -e '/"111"/!d' -e '/"14"/!d' -e 's/.*">//' -e 's/<.*//'
