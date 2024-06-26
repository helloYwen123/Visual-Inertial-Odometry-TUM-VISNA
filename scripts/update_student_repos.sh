#!/usr/bin/env bash

# Run this from the public visnav_ss21 repo
# (https://gitlab.vision.in.tum.de/visnav_ss21/visnav_ss21). It pulls
# the master branch and pushes it too all student repos. It will fail
# if the push is not a clean fast-forward, necessitating manual
# intervention to merge.

set -x
set -e

SEMESTER=ss21

# select what to do
#UPDATE_UPSTREAM_BRANCH=1
#MERGE_MASTER_BRANCH=1

#student_accounts="xingwei"
student_accounts="s0020 s0021 s0022"
#student_accounts="w0030 w0031 w0032 w0033 w0034 w0035 w0036 w0037 w0038 w0039"
#student_accounts="s0060 s0061 s0062 s0063 s0064 s0065 s0066 s0067 s0068 s0069 s0070 s0071 s0072 s0073 s0074 s0075 s0076"



function get_student_url()
{
    echo git@gitlab.vision.in.tum.de:visnav_$SEMESTER/$1/visnav.git
}


# check if we are in the right repo
if [[ ! `git remote get-url origin` == "git@gitlab.vision.in.tum.de:visnav_$SEMESTER/visnav_$SEMESTER.git" ]]; then
    echo "Wrong repo? origin is `git remote get-url origin`"
    exit 1
fi



for s in $student_accounts; do
    if [[ `git remote get-url $s 2> /dev/null` != "" ]]; then
        if [[ ! `git remote get-url $s` == `get_student_url $s` ]]; then
            echo "Wrong url for remote $s: `git remote get-url $s`"
            exit 1
        fi
    else
        git remote add $s `get_student_url $s`
    fi
    git remote update -p $s
done


git remote update -p origin

git checkout upstream

git pull --ff-only origin upstream

# update all upstream branches
if [[ $UPDATE_UPSTREAM_BRANCH == 1 ]]; then
    for s in $student_accounts; do
        #git push $s upstream:master
        git push $s upstream 
    done
fi

# merge upstream in their master branches
if [[ $MERGE_MASTER_BRANCH == 1 ]]; then
    for s in $student_accounts; do
        git branch ${s}_master $s/master 2> /dev/null || echo "branch ${s}_master exists"
        git checkout ${s}_master
        git pull --ff-only
        # TODO if merge not successfull --> pause and wait for user to resolve, then continue
        git merge upstream
        git push -u ${s} ${s}_master:master
    done
fi
