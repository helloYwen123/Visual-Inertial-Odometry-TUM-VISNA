# Course Instructor Notes

Notes on how to conduct the course

## Overview

The different phases of the courses:

* Announcement and Matching
* Preparation
* Exercise Phase
* Project Phase
* Presentation and Report
* Evaluation
* Grading

## Announcement and Matching

See: http://docmatching.in.tum.de/index.php/scheduler-course-organizers

* tell Sabine you will do the course so she can publish course info on
  TUMonline (check if info / links need updating; also english
  version)
* create mailinglist (or ask Quirin to to it)
* tell Quirin you need 20 student accounts; tell him you want to use the self-registration module
* create matrix chat room for instructors
* create google docs folder; copy lectures / pre-meeting / project slides
* copy & update website from previous semester (make sure info on
  time&date of courses is up-to-date, ideally including already
  correct dates for pre-meeting, first tutor session, final
  presentation and report)
* create BBB room for pre-meeting and announce it on website / TUMonline
* conduct the pre-meeting; add slides and recording to website afterwards
* rank students; make sure to update to 12 students in the matching system
* enter matching result in TUMonline (send files from matching system to Sabine)
* send [info email](./EmailTemplates.md#After_entering_matching_result_on_TUMonline)

## Preparation

Things to do before the first week of the semester.

TL/DR:
* Create BBB room for virtual tutor session
* Create student chat room on matrix
* Archive old student accounts
* Setup student account registration
* Update main code repository: 
  * TODOs, links, emails, dates
  * Supported Ubuntu / macOS versions (ci, fix compiler warnings, etc...)
  * external dependency version (submodules)
* Create student repository
* Create student accounts / groups / repositories on gitlab
* Update website:
  * Lectures: video links, lecture slides
  * materials page (pw-protected)
    * BBB room for virtual tutor sessions
    * matrix student chat
    * chair (gitlab) account registration
* Email students with material page password and info what to do.
    
### Tutor Session BBB Room

* Create BBB room with name "Vision-based Navigation SS21" on
  https://bbb.in.tum.de/
* Invite other instructors to join the room
* Add link to materials page on website (pw-protected)

### Student Matrix Chat

* Create room on matrix.in.tum.de:
  * Name `VISNAV SS21 Student`
  * Primary address `#visnav_ss21_student:in.tum.de` (URL:
    https://matrix.in.tum.de/#/room/#visnav_ss21_student:in.tum.de)
  * Not encrypted
  * Allow non-guests to enter with link
  * Members can read all past history
  * Make all course instructors admins
* Add link / info to materials page on website (pw-protected)

### Student Account Registration

* You need to tell Quirin beforehand that you want to use the
  registration system.
* Under the link https://adm9.in.tum.de/course/s21/visnav students
  that you have registered can enter their matriculation number. What
  happens is:
  * They get assigned a student account from the pool of accounts
    assigned to the course
  * Their email and name are set on LDAP (as you registered it)
  * Using a custom hook, we make sure the assigned gitlab account is
    also updated with their name. This hook also sends the info to the
    course instructors.
  * They get an email with their credentials
* To configure the system, you need to login on the adm9 page and go
  to https://adm9.in.tum.de/course/s21/visnav
* To register students, you need to upload a csv file that you can
  export from the course participants in TUMonline (csv, not excel).
  You can also download the current csv file from the system, even if
  it is empty, to get a template with the required fields. You can
  then edit this file and reupload. For example, you can add the
  course instructors for testing (just use something arbitrary for
  matriculation number and an email that is not yet on gitlab, like
  demmeln+visnav_ws12@in.tum.de (you can put anything after(with semester info
  ex. +visnav_ws21@in.tum.de to prevent using same email-address) "+" and it
  will still get delivered to your in.tum email)).
* After uploading the csv file from tumonline, it might have some
  strange characters. Make sure to download from the system once more,
  delete any strange (whitespace) characters in a text editor, then
  upload again.
* If you are testing registration and want to re-register your test
  account, you can "clear" the registration by editing the LDAP user
  fields. E.g. if I got assigned the account s0020, I go to
  https://adm9.in.tum.de/userman/s0020 (or ask Quirin to do it if you
  don't have rights) and clear the fields "Your displayed name"
  (replace with something like "s0020"), "Mail Address",
  "Kommentar". Then you can re-register.
* Setup the registration hook that sets the information in gitlab.
  * Make sure you have created the student accounts on gitlab before
    you test registration with this hook.
  * You need a gitlab admin API token. If you don't have admin rights
    (most likely) ask Quirin if he would be willing to create one for
    you for this purpose. Guard this well!
    * https://gitlab.vision.in.tum.de/-/profile/personal_access_tokens
    * select `api` and `sudo`
    * give meaningful name like `visnav registration hook` set
      expiration to something soon, like 2 weeks after the course
      start to make sure it gets disabled as soon as it is not needed
      any more.
  * The whole setup for the hook, including setting file access rights
    is explained in `scripts/visnav-hook/hook.sh`. Make sure to update
    the email address under `TO` in the script.
  * There is also a `test-token.sh` script to check if the access of
    the gitlab API with the TOKEN file works.
* Add the embed link to materials page on website (pw-protected).

### Repositories Overview

The main course repository is
https://gitlab.vision.in.tum.de/teaching/visnav. This contains the
master version of the code, but it is meant only for instructors, not
students! In particular, it containes sensitive information including:
* solutions to all exercises
* experimental code that may be relevant for projects (global SfM)
* exercise sheet latex sources
* todo notes
* administrative scripts in the `scritps` folder
* organizational documents (including some senstitive data) in `org`

For this reason, before every semester, we create a fresh student
repo, such as https://gitlab.vision.in.tum.de/visnav_ss20/visnav_ss20,
which is stripped down copy of the latest content of the main
repsitory.  Each student then gets it's own clone of this student
repo. Since the student repos are forked from the main repo, you
should not push any additional branches there (just `master` and
`upstream`). The visibility for students is such that they cannot see
the code in the main student repo (to not be confusing, since they
each have their own copy). They can however access the issue tracker
and we want to encourage students to use the common issue tracker
rather than opening issues in their own repos.

### Update Main Repository

* Check `TODO.md`
  * Check "TODO" notes in code, tests or tex files.
* Update `README.md`
* Update files in `exercise_sheets`
  * clean temporary files: `make wipe`
  * `visnav.sty`: `\mailingList`, `\maketitle`, `\generalNotice`,
    `\submissionInstructions`
  * `ex[1-5].tex`: submission date (watch out for public holidays /
    lecture free days (e.g. whitsun in May) when determining the
    exercise sheet deadlines; you can find all dates here: 
    https://www.tum.de/en/studies/application/dates-and-deadlines/dates-periods-and-deadlines/)
  * `ex1.tex`: `visnav_ss21` in URL; wpdate cmake parts from actual
    `CMakeLists.txt` file and `test/CMakeLists.txt` if that was
    changed
  * build pdfs: `make`
  * once satisfied and pdfs are proof-read, also commit pdfs to
    repository
* Update documentation in `wiki/Setup.md`:
  * update supported / tested OS versions
  * update `visnav_ss21` in URLs
  * update qtcreator and other tool versions and URLs
* Check if ci needs to be updated for new versions:
  * add jobs for new version (Ubuntu LTS, macos, ...); remove obsolete
    versions
  * update docker images and add new Ubuntu LTS versions (see
    Dockerfiles in the `docker` folder)
* Check if external dependencies need to be updated, e.g. to fix
  compilation errors / warnings with newer compilers
  * update (selected) submodules in `thirdparty` and `test/googletest`
* Check compilation; fix errors warnings on all supported platforms
  and default compilers.
* Test functionality of all programs and unit tests.
* Once everything is finalized, tag the revision:
  * `git tag ss20_initial`
  * `git push --tags`

### Create Student Version Repository

Here we setup the git repository for students. This is a "snapshot
copy" of the main repository with only the files that students should
see. We start with previous year's repository, update the files more
or less manually, and in the end squash all commits. This has proved
easier than recreating it from scratch, in particular also b/c of
submodules.

* Clone last semseter's repository with all submodules into a folder
  with already updated name.
  ```
  git clone --recursive git@gitlab.vision.in.tum.de:visnav_ws20/visnav_ws20.git visnav_ss21
  ```
* Update the remote URL. (You don't need to create this group / repo
  manually on gitlab. We do this with a script, see below.)
  ```
  git remote set-url origin git@gitlab.vision.in.tum.de:visnav_ss21/visnav_ss21.git
  ```
* Now update the relevant files from the main repository. I've found
  it easiest to use an interactive diff program that you can point to
  whole folders. You can use the free `meld`. On macOS I use
  BeyondCompare (it's commercial). Assuming you have both checkouts in
  the same folder:
  ```
  meld ./visnav ./visnav_ss21
  ```
  Make sure to exclude in particular the following files / folders:
  * any temporary or generated files / folders like `build`
  * `.git`, `scripts`, `org`, `docker`, `data/V1_01_easy`
  * `thirdparty` (if submodules have been updated, we update
    `.gitmodules` and then the folders manually)
  * `.style.yapf`, `TODO.md`
  Some other files need special consideration:
  * `.gitlab-ci.yml`: The files should stay different. The student
    version should only have 1 compile job enabled + the format check
    (to save CI resources). The `focal-exclude-parts` job should not
    be present at all.
  * `test/CMakeLists.txt`: the unit tests should all be disabled /
    commented.
  * `src`, `include`: You can usually copy over changed files. The
    solution marked by ifdefs can be stripped with a script (see
    below)
  * `exercise_sheets`: only copy pdfs; ignore `.gitignore`
* If `.gitmodules` changed, you may need to update gitmodules. Run
  ```
  git submodule sync --recursive
  git submodule update --init --recursive
  ```
  This should initialize newly added submodules and update remote URLs
  (if changed). Then, check the submodules one by one and make sure
  they have the same commit checked out as in the main
  repository. Once updated, commit the changed submodules right away
  (before running `./build_submodules.sh` to avoid the commits being
  reset again).
  If submodules got removed, you might need to manually remove them.
* Once you have copied everything you want, you can use the
  `scripts/strip_solution.sh` script to remove code blocks in the
  ifdefs that corresponds to solutions, experimental code, or data
  generation of unit tests. The script is not copied to the student
  repo, i.e. it sits in the main repo, but you need to run if from the
  folder of the student repo. There is a check for the remote of the
  repo where it is run in. Update URL to the new semester in the
  script. Then run it:
  ```
  cd visnav_ss21/
  ../visnav/scripts/strip_solution.sh
  ```
  The script also runs clang format.
* You can then still check the diff for any changes that should not
  make it into the student repo. We recommend interactively staging
  changes with a git gui client (e.g. `gitg`). Once you are happy,
  commit the changes.
* Now we squash the commits into one with interactive rebase:
  ```
  git rebase --root -i
  ```
  For the oldest commit, select `reword`, for all others select 
  `fixup`. When commiting, you can change the commit message for
  the base commit, you can put something like `version ss21`.
* You can adjust the commit and author date, e.g.:
  ```
  git commit --amend --date="Sun Apr 18 18:18 2021 +0200"
  GIT_COMMITTER_DATE="Sun Apr 18 18:18 2021 +0200" git commit --amend
  ```
* Remove the remote branches, and create a new local upstream branch.
  ```
  git branch -rd origin/master origin/upstream origin/HEAD
  git branch upstream
  ```

### Gitlab Script

We have a python script to manage gitlab repos, projects, users,
etc. Here we describe the setup to get it working. In specific
sections we then describe the usage to add accounts, archive accounts,
setup per-user groups and projects with correct access permissions.

* You need to have Python 3.6 or newer and install python dependencies (via pip), 
  in particular `python-gitlab`, `click`, `tabulate`, `munch`. If a package is missing, 
  check the imports at the top of the file
  `scripts/visnav_gitlab.py` or see which packages fail to import when
  you run the script.
  **Example:** 
  ```
  python3 -m pip install -U --user python-gitlab click tabulate munch
  ```
* You need to configure the access to our gitlab for the
  `python-gitlab` package. See also the documentation:
  https://python-gitlab.readthedocs.io/en/stable/cli.html#configuration. Create
  a file `~/.python-gitlab.cfg` (in your home folder) with content:
  ```
  [global]
  default = gitlab9
  ssl_verify = true
  #timeout = 5

  [gitlab9]
  url = https://gitlab.vision.in.tum.de/
  private_token = XXXXXXXXXXXXXXXXX
  api_version = 4
  ```
  The token needs to be replaced with an sudo+api token from a gitlab
  admin user (talk to Quirin). This token is a sensitive credential!
  We recommend to set the expiration of the token to a short period,
  like 1 or 2 weeks.

### Archiving Student Accounts

Before creating and configuring student accounts for the new semester, it is 
important that existing gitlab accounts from the year before are archived or
deleted. Usually this should be done right after deleting the corresponding
LDAP accounts, which typically happens around 1 month before the semester start.

Archiving accounts will rename them with a prefix `obsolete_YYYYMMDD_...` with the 
current date. It will change the status to `blocked`, remove any (ldap) "identities"
and ssh keys from the user and change the email address. For visnav accounts it will
moreover rename any remaining groups b/c of a bug in gitlab that would block the
namespace from being reclaimed in the next semester, leading to suffixed usernames like
`w00231` instead of `w0023`. Accounts that are considered "unused" (no meaningful activity)
are deleted instead of archived.

> **Note:** We will archive ALL practical course accounts from the corresponding 
> semester, not just visnav accounts. Typically, Quirin should perform this. Talk
> to him if it has not been done yet.

See "Gitlab Script" above for how to install dependencies and
configure access. `cd` to the `scripts` folder.

To list currently active gitlab practical course accounts:

```
$ ./visnav_gitlab.py users list
    id  username    name                                          semester    state    archived    visnav    ldap_uid    email                                        created_on    last_activity_on    unused    #push      #membership  warnings
--  ----  ----------  --------------------------------------------  ----------  -------  ----------  --------  ----------  -------------------------------------------  ------------  ------------------  --------  -------  -------------  ----------
0   389  s0039       visnav s0039                                  ss21        active   False       True      s0039       temp-email-for-oauth-s0039@gitlab.localhost  2021-04-19                        True      0                    2
1   388  s0038       visnav s0038                                  ss21        active   False       True      s0038       temp-email-for-oauth-s0038@gitlab.localhost  2021-04-19                        True      0                    2
2   387  s0037       visnav s0037                                  ss21        active   False       True      s0037       temp-email-for-oauth-s0037@gitlab.localhost  2021-04-19                        True      0                    2
[...]
32   350  w0056       Huimin Zeng                                   ws20        active   False       False     w0056       huimin.zeng@tum.de                           2021-02-16    2021-02-16          False     1                    2
33   349  w0058       Haokun Chen                                   ws20        active   False       False     w0058       haokun.chen@tum.de                           2021-02-09    2021-02-16          False     4                    1
34   347  w1754       winter semester acc#1754                      ws20        active   False       False     w1754       ge97zun@mytum.de                             2021-01-29    2021-01-29          True      0                    0
35   339  w0039       visnav w0039                                  ws20        active   False       True      w0039       temp-email-for-oauth-w0039@gitlab.localhost  2020-11-09                        True      0                    2
36   338  w0038       Rümeyza Dinçer Erdem (w0038)                  ws20        active   False       True      w0038       temp-email-for-oauth-w0038@gitlab.localhost  2020-11-09                        True      0                    2
[...]
```

In this example, we see accounts for the (ongoing) summer semester and the previous
winter semester and we want to archive the winter accounts. In the table, you should
(by default) see only active and not-archived accounts. You can additionally see
if the account is used for visnav or another course, and if it is considered "unused"
(which means it either has not been logged into at all (no "last activity"), or the 
only events are "joined" or "left" events, and in particular no "push" events).

The `archive` command can now be used to archive old accounts:

```
$ ./visnav_gitlab.py users archive --help
Usage: visnav_gitlab.py users archive [OPTIONS]

  archive old users

Options:
  --update-only-one  Update only one row; then like dry-run for the rest.
  -n, --dry-run      Don't actually modify anything in gitlab.
  --semester TEXT    Semester up to which to archive users. Default: Current
                     semester -2.

  --username TEXT    Archive single user by username.
  --help             Show this message and exit.
```

You should specify the corresponding `semester` and try first with `--dry-run`,
to see what changes would be applied. You can also any time archive a single user
with the `--username` argument.
(This can be useful at an earlier time, e.g. if a student that participated in a 
practical course, starts a thesis project and gets a proper student account, but
his email address in gitlab is still associated with the practical course account.)

```
$ ./visnav_gitlab.py users archive --semester ws20 -n
      id  username    name                               sem    last_activity    actions
--  ----  ----------  ---------------------------------  -----  ---------------  -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 0   357  w0013       Samuel Weber                       ws20   2021-03-31       rename to obsolete_20210923_w0013, block, remove identities (ldapmain), delete emails (sam.weber@tum.de)
 1   352  w0057       Chenguang Huang                    ws20   2021-02-19       rename to obsolete_20210923_w0057, block, remove identities (ldapmain), delete emails (ge73ven@mytum.de)
 2   351  w0051       Zhenrui Yue                        ws20   2021-02-16       delete (created 2021-02-16, last-activity 2021-02-16, 0 push events, 2 total events, email yuezrhb@gmail.com))
[...]
13   331  w0031       David Test (w0031)                 ws20   2021-08-18       rename to obsolete_20210923_w0031, block, remove identities (ldapmain), delete emails (schubdav+visnav_ws20@in.tum.de, temp-email-for-oauth-w0031@gitlab.localhost), rename groups (visnav_ws20/w0031 -> visnav_ws20/w0031_), delete 1 ssh keys
14   330  w0030       Niko Test (w0030)                  ws20   2020-11-14       delete (created 2020-11-09, last-activity 2020-11-14, 0 push events, 2 total events, email demmeln+visnav_ws20@in.tum.de)), rename groups (visnav_ws20/w0030 -> visnav_ws20/w0030_)
15   326  w0012       Idil Sülö                          ws20   2021-08-06       rename to obsolete_20210923_w0012, block, remove identities (ldapmain), delete emails (idil.sulo@tum.de), delete 2 ssh keys
[...]
```

You can see what actions would be applied to the accounts (rename, block, ...). "Unused" 
accounts will get deleted instead of archived. This is just meant to clean up accounts
that really have not content (never pushed any code or opened any issue, etc).
If you are happy with the list of accounts, you can remove the `--dry-run` flag and
perform the archiving. You will get asked for confirmation for every account that is
being deleted ("unsused" accounts). If you reply "no", the account will get archived instead.

```
$ ./visnav_gitlab.py users archive --semester ws20
rename to obsolete_20210923_w0013 (Obsolete (20210923) Samuel Weber)
rename to obsolete_20210923_w0057 (Obsolete (20210923) Chenguang Huang)
Do you really want to delete user w0051 (created 2021-02-16, last-activity 2021-02-16, 0 push events, 2 total events, email yuezrhb@gmail.com))? [y/N]: y
rename to obsolete_20210923_w0056 (Obsolete (20210923) Huimin Zeng)
rename to obsolete_20210923_w0058 (Obsolete (20210923) Haokun Chen)
Do you really want to delete user w1754 (created 2021-01-29, last-activity 2021-01-29, 0 push events, 0 total events, email ge97zun@mytum.de))? [y/N]: y
Do you really want to delete user w0039 (created 2020-11-09, last-activity None, 0 push events, 2 total events, email temp-email-for-oauth-w0039@gitlab.localhost))? [y/N]: y
Do you really want to delete user w0038 (created 2020-11-09, last-activity None, 0 push events, 2 total events, email temp-email-for-oauth-w0038@gitlab.localhost))? [y/N]: y
rename to obsolete_20210923_w0037 (Obsolete (20210923) Maximilian Listl (w0037))
[...]
```

You can now run `./visnav_gitlab.py users list` again and should only see the remaining
active accounts. You can also list all old, archived practical course accounts by
supplying the `--archived` argument:

```
$ ./visnav_gitlab.py users list --archived
       id  username                 name                                                   semester    state    archived    visnav    ldap_uid    email                                            created_on    last_activity_on    unused    #push      #membership  warnings
---  ----  -----------------------  -----------------------------------------------------  ----------  -------  ----------  --------  ----------  -----------------------------------------------  ------------  ------------------  --------  -------  -------------  ----------
  0   389  s0039                    visnav s0039                                           ss21        active   False       True      s0039       temp-email-for-oauth-s0039@gitlab.localhost      2021-04-19                        True      0                    2
  1   388  s0038                    visnav s0038                                           ss21        active   False       True      s0038       temp-email-for-oauth-s0038@gitlab.localhost      2021-04-19                        True      0                    2
  2   387  s0037                    visnav s0037                                           ss21        active   False       True      s0037       temp-email-for-oauth-s0037@gitlab.localhost      2021-04-19                        True      0                    2
  3   386  s0036                    visnav s0036                                           ss21        active   False       True      s0036       temp-email-for-oauth-s0036@gitlab.localhost      2021-04-19                        True      0                    2
[...]
```

### Create Student Accounts on gitlab

Gitlab users get created automatically if a LDAP user logs
in. However, we want to prepare their access to the visnav
repositories and also use the registration hook to make sure their
display name reflects the students actual name.

* See "Gitlab Script" above for how to install dependencies and
  configure access. `cd` to the `scripts` folder.
* Study the help: 
  ```
  ./visnav_gitlab.py users setup-visnav --help
  ```
* Select the right range of accounts assigned by Quirin. Then try with
  `-n` (dry run), for example:
  ```
  ./visnav_gitlab.py users setup-visnav --usernames s0020-s0039 -n
  ```
* If the output looks good, rerun without `-n`.
* Now gitlab accounts should be ready for access by the registration
  hook and for assignment of permissions when setting up student
  groups/repos.

### Setup Student Groups / Repos

* See "Gitlab Script" above for how to install dependencies and
  configure access. `cd` to the `scripts` folder.
* To initially setup the main student group and student repository,
  you have this interactive setup. The initial Owner of the main group
  will be the user who's gitlab token you are using. If that is not your
  own user, make sure to pass the `--add-owner` argument to add your own
  gitlab user (and fellow course instructors) to Owners. Without that, you
  won't be able to push to the repository. As part of the setup you will have
  to push the student repository you prepared (see "Create Student
  Version Repository"). Make sure to push both `master` and `upstream`
  branches. DONT PUSH ANY OTHER BRANCHES LIKE SOLUTIONS, since
  students will get a copy of this repo.
  ```
  $ ./visnav_gitlab.py groups setup --semester ss21 --add-owner chui,klenk
  Main visnav group visnav_ss21 not found. Do you want to create it? [y/N]: y
  Creating group visnav_ss21
  Found group visnav_ss21 (id: 529, runners_token: XXXXXXXXXXXXXXXXXXXX)
  No group runners found.
  Main visnav project visnav_ss21/visnav_ss21 not found. Do you want to create it? [y/N]: y
  Creating project visnav_ss21/visnav_ss21
  Found project visnav_ss21/visnav_ss21 (id: 712)
  Main project members: demmeln (Owner)
  Main group subgroups:
  Visnav project visnav_ss21/visnav_ss21 is empty. Please push to git@gitlab.vision.in.tum.de:visnav_ss21/visnav_ss21.git
  Retry? [Y/n]: y
  Project branches:
      name      commit    protected    default
  --  --------  --------  -----------  ---------
   0  master    71b8f95c  True         True
   1  upstream  71b8f95c  False        False
  Branch upstream is unprotected. Change? [y/N]: y
  ```
* You can also see registered runners. We just need to add them to the
  main group. All projects and subgroups will have access to these
  runners.
  * (**This step is probably not needed, as we now have gitlab-wide
    shared runners, that work fine for Linux jobs using docker.** You
    still need to add the mac runners.) Ask Quirin to set up (Linux)
    runners for you. You need to send him the following info:
    ```
    internal
    visnav_ss21
    XXXXXXXXXXXXXXXXXXXX
    ```
    The runner token you can find in the main `visnav_ss21` group's
    setting page, or for your convenience in the output of the setup
    script. After Quirin added the runnrrs, consider disabling the
    following runners, since they seem to frequently fail due to too
    low memory / old CPU: `atcremers16`, `atcremers25`
  * macOS runners you need to add yourself.
    * Login to the Mac Mini server (ask someone for PW): `ssh cvai@mac01`
    * Register the runner for the host (Mojave). Replace the TOKEN as needed:
      ```
      for TOKEN in XXXXXXXXXXXXXXXXXXXX; do gitlab-runner register --non-interactive -r $TOKEN -u https://gitlab.vision.in.tum.de/ --executor shell --tag-list macos,shell,`sw_vers -productVersion | cut -f 1,2 -d .`,`echo $HOSTNAME | cut -d "." -f1`; done
      ```
    * From here, log into the Catalina virtual machine:
      ```
      cd vagrant/macos-catalina/
      vagrant ssh
      ```
      Execute the same command as on the host to register the runner.
  * Add the other course instructors as owners:
    ```
    $ ./visnav_gitlab.py groups setup --semester ss21 --add-owner schubdav,chui
    Found group visnav_ss21 (id: 529, runners_token: XXXXXXXXXXXXXXXXXXXX)
    Main group visnav_ss21 members: demmeln (Owner)
    Add schubdav as owner? [y/N]: y
    Adding owner schubdav
    Main group visnav_ss21 members: demmeln (Owner), schubdav (Owner)
    Main group visnav_ss21 members: demmeln (Owner), schubdav (Owner)
    Add chui as owner? [y/N]: y
    Adding owner chui
    Main group visnav_ss21 members: demmeln (Owner), schubdav (Owner), chui (Owner)
    Group runners:
          id  description              active    status
    --  ----  -----------------------  --------  --------
     0   566  mac01-vm-catalina.local  True      online
     1   565  mac01.vision.in.tum.de   True      online
    Found project visnav_ss21/visnav_ss21 (id: 712)
    Main project members: demmeln (Owner), schubdav (Owner), chui (Owner)
    Main group subgroups:
    Project branches:
        name      commit    protected    default
    --  --------  --------  -----------  ---------
     0  master    71b8f95c  True         True
     1  upstream  71b8f95c  True         False
    ```
  * Tell all course instructors that they should go to the main group
    (https://gitlab.vision.in.tum.de/visnav_ss21) and change the
    notification settings to "Watch" to ensure they get email
    notifications when students open issue or merge requests.
  * the `--unprotect-branches` / `--protect-branches` options are
    useful if you notice an issue with the repository, and you would
    like to force push a new squashed commit.
  * To setup per-student subgroups, repos and access rights, you can
    use the `--setup-users` option. Best test this first with one or
    two accounts (the ones you use for testing). This will fork the
    current main student repo (but then cut the fork
    relationship). Once the repo is created, it cannot directly be
    synched / updated from the main repo, if you push corrections
    there. You have to update student repos manually (but there is
    also a script to aid with that).
    ```
    $ ./visnav_gitlab.py groups setup --semester ss21 --setup-users s0020
    Found group visnav_ss21 (id: 529, runners_token: XXXXXXXXXXXXXXXXXXXX)
    Group runners:
          id  description              active    status
    --  ----  -----------------------  --------  --------
     0   566  mac01-vm-catalina.local  True      online
     1   565  mac01.vision.in.tum.de   True      online
    Found project visnav_ss21/visnav_ss21 (id: 712)
    Main project members: demmeln (Owner), schubdav (Owner), chui (Owner)
    Main group subgroups:
    Project branches:
        name      commit    protected    default
    --  --------  --------  -----------  ---------
     0  master    71b8f95c  True         True
     1  upstream  71b8f95c  True         False
    Setting up users: s0020
    Setting up for user s0020 (id: 370)
    Creating user subgroup
    Created visnav_ss21/s0020 (id: 530)
    Creating user project
    Forking project visnav_ss21/visnav_ss21 to visnav_ss21/s0020/visnav and configuring.... done
    Created visnav_ss21/s0020/visnav (id: 713)
    Project branches:
        name      commit    protected    default
    --  --------  --------  -----------  ---------
     0  master    71b8f95c  True         True
     1  upstream  71b8f95c  True         False
    Adding user s0020 to visnav_ss21/s0020/visnav as Developer
    Adding user s0020 to visnav_ss21/visnav_ss21 as Guest
    ```

### Updating Student Repos With Corrections

Sometimes, after the initial version was pushed, or to address issues
during the semester, we need to provide students with an update. We
have a script that aids with updating all the student repos and
merging the changes into the student's master branches.

#### Update w/o Force Push

You can just push additional commits for small corrections before the
course start. Also, any updates during the course once students
already starting cloning and working on the repo, you should not apply
with force-push

* First, prepare the updates on your clone of the
  `visnav_ss21/visnav_ss21` repo. Push to `master` and
  `upstream`. Both branches should now be on the same commit.
* In the main repo there is a script `update_student_repos.sh` to help
  updaing multiple repos with new commits. This is also useful if you
  just want to add all student repos as remotes in case you want to
  check out a student's branch easily locally, w/o cloning one copy of
  the repo for every user.
* The script is meant to be called from within the *student* repo
  clone, not the *main* repo.
* The script is quite dumb. Read it and make sure you understand what
  is going on. You need to update the `SEMESTER` variable, and also
  `student_accounts`. It's recommended to always first try with one of
  the test accounts before doing it for all student accounts.
* You also need to uncomment `UPDATE_UPSTREAM_BRANCH=1` and
  `MERGE_MASTER_BRANCH=1` depending on which step you want to perform.
  The `upstream` branch on student repos should always be the pristine
  original state of the code, w/o student changes, while the `master`
  branch will have student changes from their submitted exercises.
* When it merges to the `master` branches, the interactive window will
  pop up with the commit message. You need to save and close the
  editor for it to continue. If there are merge conflicts, you need to
  resolve them manually. (To minimize the chance of conflicts here we
  ask students to try to not make unneccessary code changes outside of
  the parts required by the exercises. Of course they can add custom
  helpers etc.)


#### Update w/ Force Push

If you made a mistake like leaking part of the solution to the student
repo and you notice before the course starts, you can do the above
update, but with squashing commits and force pushing. The
`--unprotect-branches` / `--protect-branches` options of the
`visnav_gitlab.py groups setup` will be helpful, since protected
branches you cannot force push.

Detailed instructions: **TODO**

### Update Website

* Lectures: video links, lecture slides
* materials page (pw-protected)
  * copy the `material` subpage from the previous semester
  * Under `Select Action > Page Settings`, make sure a password is set
    to something like `visnav21ss`
  * Consider changing the password of the previous semester's material
    page (b/c the pw is easy to guess)
  * BBB room for virtual tutor sessions
  * matrix student chat
  * chair (gitlab) account registration

### Email Students

Once everything is prepared, email students and let them know how to
access the lectures, the materials page (include PW), and whatever
else they need to know before the first tutor session.

* **TODO** Email template.

* When you send emails to students, make sure to add additional
  recipients BCC, Jason and Regine (I think you should be able to
  select people that are part of the course in TUMonline from the
  list). Unselect Daniel, if he is selected.
* After sending this email, add note to the website that the info was
  sent, and that people should contact us if they didn't get it.
* [email template](./EmailTemplates.md#After_preparation_is_finished)


## Exercise Phase

### Conducting Virtual Tutor Session

#### General
* Check names of attendeeds (no unknown people; take note of
  attendance; it's volunatry, but in case someone complains about the
  grade, it can be useful to have a record of missed tutor sessions)
* Don't forget to start the recording of the session. Announce the
  fact that the recording will start at the beginning.
* While / after discussing the new exercise sheet, consider running a
  demo to point out what they should see when completing the exercises
  on that sheet.
* After the tutor session, add the link to the BBB recording to the
  webpage (can take a few hours to become available on BBB).
* **TODO**

#### First Session
* Round of introductions. Offer students to say 1-2 sentences about
  themsleves and show their face on camera (voluntary of course). Turn
  recording off during that time (or start only after).
* **TODO**
