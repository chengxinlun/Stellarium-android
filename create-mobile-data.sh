#!/bin/bash

# A quick bash script that generates a lower quality version of the data.

trap "echo Exited!; exit;" SIGINT SIGTERM

mkdir -p mobileData

# I couldn't find a proper imagemagik command to do as good as PIL/phatch.
# So here is an ugly python script to scale down an image.
function resize {
python << END
from PIL import Image
im = Image.open("$1")
size = [x / $2 for x in im.size]
im.resize(size, Image.ANTIALIAS).save("$1")
END
}

function resize-all {
scale=$1
shift;
for i in $@; do
    echo "scale (x 1/$scale) and compress image $i"
    [[ $scale != 1 ]] && resize $i $scale
    # Call pngquant, only for non grayscale images.
    if ! file $i | grep -q grayscale; then
        pngquant --ext .png -f $i;
    fi
done
}

# Generate scaled down compressed nebulae textures.
rsync -avz --exclude "*~" nebulae mobileData/
resize-all 4 mobileData/nebulae/default/*.png

# Generate scaled down compressed textures.
rsync -avz --exclude "*~" textures mobileData/
resize-all 2 mobileData/textures/*.png

# Keep the milky way texture full size.
cp textures/milkyway.png mobileData/textures/

# Copy stars data up to level 2.
rsync -avz --exclude "*~" --exclude "stars_3*" stars mobileData/

# Copy some of the landscapes.
LANDSCAPES="guereins hurricane ocean saturn"
for land in $LANDSCAPES; do
    rsync -avz --exclude "*~" landscapes/$land mobileData/landscapes
    resize-all 2 mobileData/landscapes/$land/*.png
done

# Copy some of the sky cultures.
CULTURES="aztec  chinese  egyptian  inuit  korean  lakota  maori  navajo
          norse  polynesian  sami  tupi  western"
for culture in $CULTURES; do
    rsync -avz --exclude "*~" skycultures/$culture mobileData/skycultures
    resize-all 2 mobileData/skycultures/$culture/*.png
done

# Create translation files
LANGS="en fr de es it zh_CN zh_TW ru"
mkdir -p mobileData/translations/stellarium
mkdir -p mobileData/translations/stellarium-skycultures
for lang in $LANGS; do
    lconvert -i po/stellarium/$lang.po \
             -o mobileData/translations/stellarium/$lang.qm
    lconvert -i po/stellarium-skycultures/$lang.po \
             -o mobileData/translations/stellarium-skycultures/$lang.qm
done

# Copy data dir
rsync -avz --exclude "*~" --exclude "data/qml" --exclude "data/gui" --exclude "data/icons" --exclude "data/maemo" --exclude "data/shaders" --exclude "data/base_locations.txt" --exclude "data/DejaVu*" --exclude "data/splash.bmp" --exclude "data/stellarium.ico" --exclude "data/Icon.icns" data mobileData/
