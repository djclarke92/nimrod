#!/bin/bash
# Nimrod script to archive/delete cctv video files
# Run from crontab at 4:20am or some other time

BASE=/var/cctv
DAYS=45

cd $BASE
if [ $? != 0 ]; then
        echo "Error cd'ing to $BASE, terminating"
        exit 1
fi

df -k $BASE

LIST=`ls -1`
for DIR in $LIST; do

        echo ""
        echo "archiving files in $DIR"

        if [ -d $DIR/*/record ]; then

                cd $DIR/*/record
                if [ ! -d archive ]; then
                        mkdir archive
                fi

                # move files into archive directory
                find . -maxdepth 1 -daystart -mtime +1 -type f -exec mv {} archive \;

                cd $BASE
        fi

done

# delete files older than 3 months
echo ""
echo "deleting files older than $DAYS days"
find $BASE -mtime +$DAYS -type f -delete \( -name "*.jpg" -or -name "*.mkv" -or -name "*.mp4" \)

echo ""
df -k $BASE
