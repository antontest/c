#!/bin/bash
FILENAME="$1"
perl -pi -e 's/\xc6\x83\xf8\x07\x00\x00\x01/\xc6\x83\xf8\x07\x00\x00\x00/g' "$FILENAME"
perl -pi -e 's/\xc6\x80\xf8\x07\x00\x00\x01/\xc6\x80\xf8\x07\x00\x00\x00/g' "$FILENAME"
perl -pi -e 's/\x80\xbf\xd1\x07\x08\x00\x00\x75\x1a/\x80\xbf\xd1\x07\x08\x00\x01\x75\x1a/g' "$FILENAME"