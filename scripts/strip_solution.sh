#!/usr/bin/env bash

# Run this from the public student repo to strip out the solutions
# after updating from the main visnav repo.

#set -x
set -e

# check if we are in the right repo
if [[ ! `git remote get-url origin` == "git@gitlab.vision.in.tum.de:visnav_ss21/visnav_ss21.git" ]]; then
    echo "Wrong repo? origin is `git remote get-url origin`"
    exit 1
fi

# strip using unifdef
echo "stripping all files"
find ./include ./src ./test/src -iname "*.hpp" -or -iname "*.h" -or -iname "*.cpp" | while read f
do
    if ! unifdef \
         -DEXCLUDE_PARTS_EX1 \
         -DEXCLUDE_PARTS_EX2 \
         -DEXCLUDE_PARTS_EX3 \
         -DEXCLUDE_PARTS_EX4 \
         -DEXCLUDE_PARTS_EX5 \
         -DEXCLUDE_EXPERIMENTAL \
         -DEXCLUDE_TEST_DATA_CREATION \
         -o "$f" "$f"; then
        echo "stripped: $f"
    fi
done

# remove the //#define EXCLUDE_PARTS_... block
COMMON_HEADER=include/visnav/common_types.h
echo "Checking $COMMON_HEADER"
sed -i '' '/\/\/#define EXCLUDE_PARTS_EX1/,/^$/d' "$COMMON_HEADER"

# format all files (to reindent comments)
echo "reformatting all files"
find ./include ./src ./test/src -iname "*.hpp" -or -iname "*.h" -or -iname "*.cpp" | xargs clang-format -i
