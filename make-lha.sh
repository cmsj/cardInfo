#!/bin/bash

set -eux

ARCHIVE="cardInfo.lha"

cd ..
rm -f "${ARCHIVE}"
cp cardInfo/Readme cardInfo.readme
cp cardInfo/.folder_info cardInfo.info

rm -f cardInfo.lha

lha -ao5 ${ARCHIVE} \
	cardInfo/cardInfo \
	cardInfo/cardInfo.info \
	cardInfo/cardInfo.c \
	cardInfo/cardInfo.guide \
	cardInfo/cardInfo.guide.info \
	cardInfo/Readme \
	cardInfo/Readme.info \
	cardInfo/LICENSE \
	cardInfo/examples \
	cardInfo/examples.info \
	cardInfo/Makefile \
	cardInfo.info

echo ""
echo ""

lha l ${ARCHIVE}
