#!/usr/bin/env bash

pushd docs/
. genManPage.sh
. genHTMLPage.sh
popd
