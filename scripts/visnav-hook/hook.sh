#!/usr/bin/env bash
#
# Preparation:
#
#     Update the "TO" email address below to the latest course email.
#
# SETUP:
#
#     mkdir -p /storage/www/user/demmeln/visnav-hook
#     cd /storage/www/user/demmeln/visnav-hook
#     touch registration.log
#     touch TOKEN
#     chmod og-rwx TOKEN
#     chmod og-rwx ./
#
# Copy hook.sh and .htaccess
# Fill TOKEN with access token for gitlab admin api
#
#     nfs4_setfacl -a A::www-data@informatik.tu-muenchen.de:R TOKEN
#     nfs4_setfacl -a A::www-data@informatik.tu-muenchen.de:RX ./
#     nfs4_setfacl -a A::www-data@informatik.tu-muenchen.de:RX hook.sh
#     nfs4_setfacl -a A::www-data@informatik.tu-muenchen.de:RW registration.log
#     
# If nfs version=3, use the following instead
#     # setfacl -m u:www-data:r TOKEN
#     # setfacl -m u:www-data:rx .
#     # setfacl -m u:www-data:rx hook.sh
#     # setfacl -m u:www-data:rw registration.log
#
# Set hook path to /storage/www/user/demmeln/visnav-hook/hook.sh (on https://adm9.in.tum.de/course/s21/visnav)


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"


##############################
# set gitlab name for user

# Note: Unfortunately we cannot use the user's credential to change it's own profile.
# See: https://gitlab.com/gitlab-org/gitlab/-/issues/17843
#TOKEN=`curl --request POST --url https://gitlab.vision.in.tum.de/oauth/token --header 'content-type: application/json' --data "{ \"grant_type\": \"password\", \"username\": \"$TEST_USER\", \"password\": \"$TEST_PW\"}" -s -S | jq -r .access_token`
#curl --request GET --header "Authorization: Bearer $TOKEN" https://gitlab.vision.in.tum.de/api/v4/users?username=s0060


# use admin token to change username
HEADER="Private-Token: `cat "$DIR"/TOKEN`"
URL=https://gitlab.vision.in.tum.de/api/v4

USERID=$(curl --request GET --header "$HEADER"  $URL/users?username=$HOOK_USERNAME -sS | jq -r .[0].id)

curl -G --request PUT --header "$HEADER"  $URL/users/$USERID --data-urlencode "name=$HOOK_NAME ($HOOK_USERNAME)" -sS > /dev/null


##############################
# send email to course instructors

TO=visnav_ws2021@vision.in.tum.de
#TO=demmeln@in.tum.de  # <-- uncomment for testing
FROM="Visnav Account Registration"
SUBJECT="[VISNAV] user $HOOK_USERNAME registered"
MESSAGE="Visnav user $HOOK_NAME with email $HOOK_EMAIL registered as $HOOK_USERNAME with password $HOOK_PASSWORD (gitlab-id: $USERID)."

( echo "Subject: $SUBJECT"; echo "$MESSAGE" ) | sendmail -F "$FROM" $TO


#############################
# save log file
echo "`date -Iseconds` - $MESSAGE" >> "$DIR"/registration.log


#############################
# TODO MAYBE:
# - send email to student with instructions on how to change pw, login to gitlab, and course website
# - echo the same information to be shown on the webpage


