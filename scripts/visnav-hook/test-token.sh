#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

HEADER="Private-Token: `cat "$DIR"/TOKEN`"
URL=https://gitlab.vision.in.tum.de/api/v4

REPLY=`curl --request GET --header "$HEADER"  $URL/users?per_page=1 -sS`

if echo $REPLY | grep invalid_token &> /dev/null; then
    echo "Failed to access API ($URL): $REPLY"
else
    echo "Successfully accessed API ($URL)"
fi
