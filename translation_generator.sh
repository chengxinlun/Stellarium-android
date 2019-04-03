#!/bin/bash

# Create translation files
# .po files in po/
# Convert them to qm
LANGS="en fr de es it zh_CN zh_TW ru"
mkdir -p mobileData/translations/stellarium
mkdir -p mobileData/translations/stellarium-skycultures
for lang in $LANGS; do
    lconvert -i po/stellarium/$lang.po \
             -o mobileData/translations/stellarium/$lang.qm
    lconvert -i po/stellarium-skycultures/$lang.po \
             -o mobileData/translations/stellarium-skycultures/$lang.qm
done
