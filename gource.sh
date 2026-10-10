#!/bin/sh

`gource --git-log-command` > /tmp/my-git-project.log
#gource -1920x1024 --highlight-all-users -s .05 --hide-filenames --user-scale 2 --crop horizontal --date-format "%d %B %Y" --stop-at-end  --output-ppm-stream  - --output-framerate 30 .  /tmp/my-git-project.log | ffmpeg -y -r 30 -f image2pipe -vcodec ppm -i - -b:v 3000K -vcodec mpeg4 /tmp/aqbanking-gource.mp4
gource -1920x1024 --highlight-all-users -s .001 --hide-filenames --user-scale 2 --crop horizontal --date-format "%d %B %Y" --stop-at-end  --output-ppm-stream  - --output-framerate 30 .  /tmp/my-git-project.log | ffmpeg -y -r 30 -f image2pipe -vcodec ppm -i - -b:v 3000K -vcodec mpeg4 /tmp/aqbanking-gource.mp4
