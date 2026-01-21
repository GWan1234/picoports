#!/usr/bin/env sh

inkscape Key.inkscape.svg --export-area-drawing --export-type=png --export-dpi=250 --export-filename=Key.png
inkscape Pinout.inkscape.svg --export-area-drawing --export-type=png --export-dpi=250 --export-filename=Pinout.png

convert -size 1589x1052 xc:white \
  \( Key.png -gravity northwest -geometry +35+35 \) -composite \
  \( Pinout.png -gravity east -geometry +25+0 \) -composite \
  \( -annotate +35+35 "Pins are 3.3 V compatible." -gravity southwest -family "Noto Sans" -pointsize 28 -fill black -style normal \) \
  Pinout+Key.png

convert -size 1144x574 xc:white \
  \( Key.png -resize 80% -gravity northwest -geometry +26+9 \) -composite \
  \( Pinout.png -resize 54% -gravity east -geometry +70+0 \) -composite \
  -bordercolor white -border 68x33 \
  Social_Preview.png
