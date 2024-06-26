#!/usr/bin/env python3

import gitlab
import re
import click
import csv
import random
import string
import itertools
import click
import sys
import time

from tabulate import tabulate
from datetime import date, datetime
from functools import total_ordering
from pprint import pprint
from munch import munchify


def random_password():
    return ''.join(random.choices(string.ascii_letters + string.digits, k=64))


def access_level_to_string(gitlab_access_level):
    if gitlab_access_level == gitlab.GUEST_ACCESS:
        return "Guest"
    elif gitlab_access_level == gitlab.REPORTER_ACCESS:
        return "Reporter"
    elif gitlab_access_level == gitlab.DEVELOPER_ACCESS:
        return "Developer"
    elif gitlab_access_level == gitlab.MAINTAINER_ACCESS:
        return "Maintainer"
    elif gitlab_access_level == gitlab.OWNER_ACCESS:
        return "Owner"
    else:
        return "Unknown"


def abort_cli(message=None):
    if message:
        print(message)
    raise click.Abort()


def filter_none(some_list):
    return [x for x in some_list if x is not None]


class AttributeTable:

    def __init__(self, attributes):
        self.attributes = attributes

    def print(self, objects, showindex="always"):
        print(tabulate(self.get_attribute_matrix(objects), headers=self.get_header(), showindex=showindex))

    def get_attribute_matrix(self, objects):
        return [[self._format_cell(self._my_get_attr(o, a)) for a in self.attributes] for o in objects]

    def get_header(self):
        return [a[0] if isinstance(a, tuple) else a for a in self.attributes]

    @staticmethod
    def _format_cell(obj):
        if isinstance(obj, list):
            return ", ".join(obj)
        return obj

    @staticmethod
    def _my_get_attr(user, a):
        if isinstance(a, tuple):
            _, a = a
        if callable(a):
            return a(user)
        else:
            return getattr(user, a)


@total_ordering
class Semester:
    """value-type for semester with ordering and conversions"""

    def __init__(self, semester):
        self._sem_str = None
        if isinstance(semester, str):
            self.set_from_string(semester)
        elif isinstance(semester, date):
            self.set_from_date(semester)
        elif isinstance(semester, tuple):
            self.set_from_date_with_prefix(semester[0], semester[1])
        else:
            raise ValueError(f"Invalid type of {semester}")

    @staticmethod
    def current():
        return Semester(date.today())

    def next_semester(self, delta=1):
        d = self.start_date()
        year = d.year + delta // 2
        month = d.month
        if delta % 2 == 1:
            if self.is_ws():
                year += 1
            month = (month + 6) % 12
        return Semester(date(year, month, d.day))

    def set_from_string(self, s):
        if re.match(r'[ws]s[0-9]{2}', s):
            self._sem_str = s
        else:
            raise ValueError(f"Invalid semester string {s}")

    def set_from_date(self, d):
        if 4 <= d.month < 10:
            prefix = 'ss'
        else:
            prefix = 'ws'
        self.set_from_date_with_prefix(prefix, d)

    def set_from_date_with_prefix(self, prefix, d):
        year = d.year
        if prefix == 'ws':
            if d.month <= 6:
                year = year - 1
        elif prefix == 'ss':
            pass
        else:
            raise ValueError(f"Invalid semester prefix {prefix}")
        if not (2000 <= year < 2100):
            raise ValueError(f"Year {year} out of range")
        self._sem_str = f"{prefix}{year % 100}"

    def prefix(self):
        return self._sem_str[0:2]

    def year(self):
        return 2000 + int(self._sem_str[2:4])

    def is_ws(self):
        return self.prefix() == "ws"

    def is_ss(self):
        return not self.is_ws()

    def start_date(self):
        month = 10 if self.is_ws() else 4
        return date(self.year(), month, 1)

    def year_string(self):
        if self.prefix() == "ws":
            return "{}/{}".format(self.year(), (self.year() % 100) + 1)
        else:
            return "{}".format(self.year())

    def semester_type(self):
        if self.prefix() == "ws":
            return "Winter Semester"
        else:
            return "Summer Semester"

    def short_name(self):
        return "{} {}".format(self.prefix().upper(), self.year_string())

    def full_name(self):
        return "{} {}".format(self.semester_type(), self.year_string())

    def __repr__(self):
        return f'{self.__class__.__name__}({self._sem_str!r})'

    def __str__(self):
        return self._sem_str

    def __eq__(self, other):
        if not isinstance(other, Semester):
            return NotImplemented
        return self._sem_str == other._sem_str

    def __lt__(self, other):
        if not isinstance(other, Semester):
            return NotImplemented
        return self.start_date() < other.start_date()


class User:

    def __init__(self, gitlab_user):
        # avoid issues with __setattr__ override
        object.__setattr__(self, "gitlab_user", gitlab_user)

    def __getattr__(self, name):
        # if name wasn't found in this class, defer to gitlab_user
        return getattr(self.gitlab_user, name)

    def __setattr__(self, name, value):
        # if name isn't found in this class, defer to gitlab_user
        try:
            object.__getattribute__(self, name)
            object.__setattr__(self, name, value)
        except AttributeError:
            setattr(self.gitlab_user, name, value)

    @property
    def is_practical_course_account(self):
        return bool(re.search('[ws][0-9]{4}', self.username))

    @property
    def is_archived(self):
        return bool(re.match(r'obsolete_.*', self.username))

    @property
    def username_base(self):
        match = re.search(r'[ws][0-9]{4}', self.username)
        if match:
            return match.group(0)
        else:
            return None

    @property
    def ldap_uid(self):
        uids = [i["extern_uid"] for i in self.identities if i["provider"].startswith("ldap") and i["extern_uid"] != ""]
        if not uids:
            return None
        elif len(uids) == 1:
            uid = uids[0]
            m = re.match(r'uid=([a-z0-9_]+),', uid)
            if not m:
                return "ERROR_PARSE_UID"
            else:
                return m.group(1)
        else:
            return "ERROR_MULTIPLE"

    @property
    def all_emails(self):
        emails = [self.email]
        for e in self.emails.list(all=True):
            emails.append(e.email)
        return emails

    @property
    def all_emails_confirmation(self):
        emails = [(self.email, self.confirmed_at)]
        for e in self.emails.list(all=True):
            emails.append((e.email, e.confirmed_at))
        return emails

    @property
    def all_emails_verbose(self):
        return "\n".join("{} ({})".format(*p) for p in self.all_emails_confirmation)

    @staticmethod
    def is_ldap_temp_email(email):
        return bool(re.match(r"temp-email-for-oauth-.+@gitlab\.localhost", email))

    @staticmethod
    def is_archived_email(email):
        return bool(re.match(r"obsolete_[0-9]+_.+@gitlab\.vision\.in\.tum\.de", email))

    @property
    def has_ldap_temp_email(self):
        return any(map(self.is_ldap_temp_email, self.all_emails))

    @property
    def push_count(self):
        count = len(self.events.list(per_page=20, action="pushed"))
        if count < 20:
            return count
        else:
            return "20+"

    @property
    def is_unused(self):
        if not self.last_activity_on:
            # no activity --> definitely unused
            return True
        elif not self.events.list(action="pushed") \
                and set(e.action_name for e in self.events.list(all=True)) <= set(["joined", "left"]):
            # check first push events; if there is one, definitely not unused
            # if there is none, check all events; if it's only "joined" and "left" events, count as unused
            return True
        else:
            return False

    @property
    def membership_count(self):
        return len(self.memberships.list(all=True))

    @property
    def created_on(self):
        return self.created_at[0:10]

    @property
    def semester(self):
        if self.username_base:
            created = datetime.strptime(self.created_on, "%Y-%m-%d").date()
            if self.username_base[0] == 'w':
                return Semester(("ws", created))
            elif self.username_base[0] == 's':
                return Semester(("ss", created))
            else:
                return None
        else:
            return None

    @property
    def is_old(self):
        return bool(self.semester and (self.semester <= Semester.current().next_semester(-2)))

    @property
    def is_visnav(self):
        for m in self.memberships.list(all=True):
            if m.source_name.startswith("visnav_"):
                return True
        return False

    @property
    def has_username_base_namespace(self):
        for n in self.manager.gitlab.namespaces.list(search=self.username_base, all=True):
            if n.kind == "group" and n.path == self.username_base and str(self.semester) in n.full_path:
                return True
        return False

    @property
    def warnings(self):
        w = []

        uid = self.ldap_uid
        if uid and uid != self.username:
            w.append("LDAPUID_MISMATCH")

        emails = self.all_emails
        if len(emails) != len(set(emails)):
            w.append("DUPLICATE_EMAILS")

        if self.is_archived:
            if self.state != "blocked":
                w.append("ARCHIVED_NOT_BLOCKED")

            if self.has_ldap_temp_email:
                w.append("ARCHIVED_HAS_LDAP_TEMP_EMAIL")

            if self.has_username_base_namespace:
                w.append("ARCHIVED_HAS_GROUP")

            if self.identities:
                if any([i["extern_uid"] for i in self.identities]):
                    w.append("ARCHIVED_HAS_ID")
                else:
                    w.append("ARCHIVED_HAS_EMPTY_ID")

            if len(self.all_emails) > 1:
                w.append("ARCHIVED_HAS_EMAILS")

            if not self.is_archived_email(self.email):
                w.append("ARCHIVED_WRONG_EMAIL")

            if self.keys.list(all=True):
                w.append("ARCHIVED_HAS_SSH")
        else:
            if self.state != "active":
                w.append("BLOCKED")

            if self.is_old:
                w.append("OLD")

        return w


class Project:

    def __init__(self, gitlab_project):
        # avoid issues with __setattr__ override
        object.__setattr__(self, "gitlab_project", gitlab_project)

    def __getattr__(self, name):
        # if name wasn't found in this class, defer to gitlab_project
        return getattr(self.gitlab_project, name)

    def __setattr__(self, name, value):
        # if name isn't found in this class, defer to gitlab_project
        try:
            object.__getattribute__(self, name)
            object.__setattr__(self, name, value)
        except AttributeError:
            setattr(self.gitlab_project, name, value)

    @property
    def username(self):
        prefix = f"visnav_{self.semester}"
        match = re.match(prefix + r'/([^/]+)/[^/]+', self.path_with_namespace)
        if match:
            return match.group(1)
        else:
            return None

    @property
    def username_base(self):
        u = self.username
        if u is None:
            return None
        match = re.match(r'([ws][0-9]{4})', u)
        if match:
            return match.group(1)
        else:
            return u

    @property
    def is_visnav_main_repo(self):
        """visnav main per-semester project"""
        return self.path_with_namespace == "visnav_{0}/visnav_{0}".format(self.semester)

    @property
    def is_visnav_practical_account_repo(self):
        """visnav project practical account user"""
        u = self.username_base
        if u is not None:
            return bool(re.match('[ws][0-9]{4}', u))
        else:
            return False

    @property
    def is_visnav_regular_account_repo(self):
        """visnav project regular account user"""
        u = self.username_base
        if u is not None:
            return not re.match('[ws][0-9]{4}', u)
        else:
            return False

    @property
    def created_on(self):
        return self.created_at[0:10]

    @property
    def last_activity_on(self):
        return self.last_activity_at[0:10]

    @property
    def semester(self):
        match = re.search(r'[ws]s[0-9]{2}', self.path_with_namespace)
        if match:
            return Semester(match.group(0))
        else:
            return None

    @property
    def is_old(self):
        return bool(self.semester and (self.semester <= Semester.current().next_semester(-2)))


class Group:

    def __init__(self, gitlab_group):
        # avoid issues with __setattr__ override
        object.__setattr__(self, "gitlab_group", gitlab_group)

    def __getattr__(self, name):
        # if name wasn't found in this class, defer to gitlab_group
        return getattr(self.gitlab_group, name)

    def __setattr__(self, name, value):
        # if name isn't found in this class, defer to gitlab_group
        try:
            object.__getattribute__(self, name)
            object.__setattr__(self, name, value)
        except AttributeError:
            setattr(self.gitlab_group, name, value)

    @property
    def username(self):
        prefix = f"visnav_{self.semester}"
        match = re.match(prefix + r'/([^/]+)', self.full_path)
        if match:
            return match.group(1)
        else:
            return None

    @property
    def username_base(self):
        u = self.username
        if u is None:
            return None
        match = re.match(r'([ws][0-9]{4})', u)
        if match:
            return match.group(1)
        else:
            return u

    @property
    def is_visnav_main_group(self):
        """visnav main per-semester group"""
        return self.full_path == "visnav_{0}".format(self.semester)

    @property
    def is_visnav_practical_account_group(self):
        """visnav project practical account group"""
        return bool(self.username and re.match('[ws][0-9]{4}', self.username_base))

    @property
    def is_visnav_regular_account_group(self):
        """visnav project regular account user"""
        return bool(self.username and not re.match('[ws][0-9]{4}', self.username_base))

    @property
    def semester(self):
        match = re.search(r'[ws]s[0-9]{2}', self.full_path)
        if match:
            return Semester(match.group(0))
        else:
            return None

    @property
    def is_old(self):
        return bool(self.semester and (self.semester <= Semester.current().next_semester(-2)))

    @property
    def warnings(self):
        w = []

        if self.is_old and self.is_visnav_practical_account_group and self.name == self.username_base:
            w.append("OLD_NOT_RENAMED")

        return w


class Actions:

    def __init__(self, state):
        self.state = state

    def maybe_delete_unused_user(self, user):
        log = []
        if user.is_unused and user.is_practical_course_account:
            info = "created {}, last-activity {}, {} push events, {} total events, email {})".format(
                user.created_on, user.last_activity_on, len(user.events.list(action="pushed", all=True)),
                len(user.events.list(all=True)), user.email)
            log.append("delete ({})".format(info))
            if not self.state.dry_run:
                if click.confirm(
                        'Do you really want to delete unused user {} ({})? If "no", it will get archived.'.format(
                            user.username, info)):
                    user.delete()
                else:
                    return []
        return log

    def maybe_block(self, user):
        log = []
        if user.state != "blocked":
            log.append("block")
            if not self.state.dry_run:
                user.block()
        return log

    def rename_user_obsolete(self, user):
        log = []
        if "obsolete_" not in user.username:
            archive_date = date.today().strftime("%Y%m%d")
            new_username = "obsolete_{}_{}".format(archive_date, user.username_base)
            new_name = "Obsolete ({}) {}".format(archive_date, user.name)
            log.append("rename to {}".format(new_username))
            if not self.state.dry_run:
                user.name = new_name
                user.username = new_username
                user.save()
                print("rename to {} ({})".format(new_username, new_name))
        return log

    def set_email_obsolete(self, user):
        log = []
        main_email_ok = user.is_archived_email(user.email)

        to_delete = [e for e in user.all_emails if not user.is_archived_email(e)]

        if to_delete:
            log.append("delete emails ({})".format(", ".join(to_delete)))

        if not self.state.dry_run:
            if not main_email_ok:
                match = re.match(r'obsolete_([0-9]+)_', user.username)
                if match:
                    archive_date = match.group(1)
                else:
                    archive_date = date.today().strftime("%Y%m%d")
                user.email = "obsolete_{}_{}@gitlab.vision.in.tum.de".format(archive_date, user.username_base)
                user.skip_reconfirmation = True
                user.save()
            for e in user.emails.list(all=True):
                e.delete()
        return log

    def rename_visnav_groups(self, user):
        log = []
        renames = []
        for g in self.state.gl.groups.list(search=user.username_base, all=True):
            if g.path == user.username_base and str(user.semester) in g.full_path:
                renames.append("{} -> {}_".format(g.full_path, g.full_path))
                if not self.state.dry_run:
                    g.path = g.path + "_"
                    g.name = g.name + "_"
                    g.save()
        if renames:
            log.append("rename groups ({})".format(", ".join(renames)))
        return log

    def delete_identities(self, user):
        log = []
        ids = []
        for i in user.identities:
            info = i["provider"]
            if not i["extern_uid"]:
                info += "(empty)"
            ids.append(info)
        if not self.state.dry_run:
            #user.identities = []
            #user.save()
            for i in user.identities:
                self.state.gl.http_delete("/users/{}/identities/{}".format(user.get_id(), i["provider"]))
        if ids:
            log.append("remove identities ({})".format(", ".join(ids)))
        return log

    def delete_ssh_keys(self, user):
        log = []
        keys = user.keys.list(all=True)
        if keys:
            log.append("delete {} ssh keys".format(len(keys)))
        if not self.state.dry_run:
            for k in keys:
                k.delete()
        return log

    def create_ldap_user(self, username, name):

        if self.state.gl.users.list(username=username):
            return ["ERROR: user {} exists".format(username)]

        log = ["create user {} ({})".format(username, name)]

        if not self.state.dry_run:
            email = 'temp-email-for-oauth-{}@gitlab.localhost'.format(username)
            ldap_uid = 'uid={},ou=praktikanten,ou=users,dc=inf9,dc=informatik,dc=tu-muenchen,dc=de'.format(username)
            user_data = {
                'username': username,
                'name': name,
                'password': random_password(),
                'email': email,
                'skip_confirmation': True,
                'extern_uid': ldap_uid,
                'provider': 'ldapmain',
            }
            self.state.gl.users.create(user_data)

        return log

    def _fix_archived_user_helper(self, user):
        log = []

        log.extend(self.maybe_block(user))
        log.extend(self.delete_identities(user))  # delete identities before trying to change emails
        log.extend(self.set_email_obsolete(user))
        log.extend(self.rename_visnav_groups(user))
        log.extend(self.delete_ssh_keys(user))

        return log

    def fix_archived_user(self, user):
        log = []

        delete_log = self.maybe_delete_unused_user(user)

        if delete_log:
            log.extend(delete_log)
            log.extend(self.rename_visnav_groups(user))
        else:
            self._fix_archived_user_helper(user)

        return log

    def archive_user(self, user):
        log = []

        delete_log = self.maybe_delete_unused_user(user)

        if delete_log:
            log.extend(delete_log)
            log.extend(self.rename_visnav_groups(user))
        else:
            log.extend(self.rename_user_obsolete(user))
            log.extend(self._fix_archived_user_helper(user))

        return log

    def rename_group_old_visnav(self, group):
        log = []
        if group.is_old and group.is_visnav_practical_account_group and (group.name == group.username_base or
                                                                         group.path == group.username_base):
            if group.name == group.username_base:
                new_name = group.name + "_"
            if group.path == group.username_base:
                new_path = group.path + "_"
            log.append(f"rename to {new_name} ({new_path})")
            if not self.state.dry_run:
                group.name = new_name
                group.path = new_path
                group.save()
        return log

    def fix_visnav_group(self, group):
        log = []
        log.extend(self.rename_group_old_visnav(group))
        return log


class Helpers:

    def __init__(self, state):
        self.state = state

    def get_all_users(self, filter=None, max_results=None):
        users = [User(u) for u in self.state.gl.users.list(all=True)]
        if filter:
            users = [u for u in users if filter(u)]
        if max_results and len(users) > max_results:
            users = users[:max_results]
        return users

    def get_user_with_username(self, username):
        users = [User(u) for u in self.state.gl.users.list(username=username)]
        if len(users) == 0:
            return None
        elif len(users) == 1:
            return users[0]
        else:
            raise RuntimeError(f"Multiple users with username {username}")

    def get_group_with_full_path(self, full_path):
        groups = [g for g in self.state.gl.groups.list(search=full_path, all=True) if g.full_path == full_path]
        if len(groups) == 0:
            return None
        elif len(groups) == 1:
            return groups[0]
        else:
            raise RuntimeError(f"Multiple groups with full_path {full_path}")

    def get_project_with_path(self, path, parent=None):
        if parent:
            projects = [p for p in parent.projects.list(search=path, all=True) if p.path == path]
        else:
            projects = [p for p in self.state.gl.projects.list(search=path, all=True) if p.path == path]
        if len(projects) == 0:
            return None
        elif len(projects) == 1:
            return self.state.gl.projects.get(projects[0].id)
        else:
            raise RuntimeError(f"Multiple projects with path {path}")

    def get_toplevel_visnav_groups(self):
        groups = self.state.gl.groups.list(all=True, search="visnav")
        # filter for actual top-level visnav groups
        groups = [g for g in groups if re.match(r'visnav_[ws]s[0-9]{2}', g.name) and g.parent_id is None]
        return list(sorted(groups, key=lambda x: x.name))

    def get_all_visnav_groups(self, with_managers=True):
        toplevel_groups = self.get_toplevel_visnav_groups()

        groups = []
        for tlg in toplevel_groups:
            groups.append(Group(tlg))
            if with_managers:
                groups.extend([Group(self.state.gl.groups.get(g.id)) for g in tlg.subgroups.list(all=True)])
            else:
                groups.extend([Group(g) for g in tlg.subgroups.list(all=True)])

        return sorted(groups,
                      key=lambda x:
                      (x.semester, not x.is_visnav_main_group, not x.is_visnav_practical_account_group, x.full_path))

    def get_all_visnav_projects(self):
        toplevel_groups = self.get_toplevel_visnav_groups()

        projects = []
        for tlg in toplevel_groups:
            projects.extend([Project(p) for p in tlg.projects.list(include_subgroups=True, all=True)])

        return sorted(
            projects,
            key=lambda x:
            (x.semester, not x.is_visnav_main_repo, not x.is_visnav_practical_account_repo, x.path_with_namespace))

    def apply_actions_users(self, users, per_user_fun):

        headers = [
            "id",
            "username",
            "name",
            "sem",
            "last_activity",
            "actions",
        ]

        table = []
        for u in users:
            row = [u.id, u.username, u.name, u.semester, u.last_activity_on]
            actions = per_user_fun(u)
            if actions:
                if self.state.update_only_one:
                    self.state.dry_run = True
            row.append(", ".join(actions))
            table.append(row)

        print(tabulate(table, headers=headers, showindex="always"))

    def apply_actions_groups(self, groups, per_group_fun):
        headers = [
            "id",
            "semester",
            "name",
            "full_path",
            "username_base",
            "visibility",
            "is_old",
            "actions",
        ]

        table = []
        for g in groups:
            row = [g.id, g.semester, g.name, g.full_path, g.username_base, g.visibility, g.is_old]
            actions = per_group_fun(g)
            if actions:
                if self.state.update_only_one:
                    self.state.dry_run = True
            row.append(", ".join(actions))
            table.append(row)

        print(tabulate(table, headers=headers, showindex="always"))

    @staticmethod
    def username_spec_to_list(usernames):
        result = []
        for username in usernames.split(","):
            if '-' in username:
                result.extend(Helpers.username_range_to_list(username))
            else:
                result.append(username)
        return result

    @staticmethod
    def username_range_to_list(username_range):
        first, last = username_range.split("-")
        prefix = first[0]
        if prefix != last[0]:
            raise ValueError(f"Invalid username range {username_range}")
        first = int(first[1:])
        last = int(last[1:])
        return ["{}{:04d}".format(prefix, i) for i in range(first, last + 1)]

    def create_visnav_main_group(self, semester):
        name = f"visnav_{semester}"
        print(f"Creating group {name}")
        if not self.state.dry_run:
            props = dict(
                name=name,
                path=name,
                description=f"Visual Navigation Practical Course {semester.full_name()}\r\n",
                visibility="private",
            )
            group = self.state.gl.groups.create(props)
            return group
        return None

    def get_group_runners(self, group_id):
        return munchify(self.state.gl.http_list(f"/groups/{group_id}/runners/", {}, all=True))

    def create_visnav_main_project(self, main_group):
        print(f"Creating project {main_group.path}/{main_group.path}")
        semester = Semester(main_group.path[-4:])
        if not self.state.dry_run:
            props = dict(
                name=main_group.path,
                path=main_group.path,
                namespace_id=main_group.id,
                description=
                f"Common project for shared issues (Practical Course: Visual Based Navigation, {semester.full_name()})\r\n",
                visibility="private",
            )
            project = self.state.gl.projects.create(props)
            return project
        return None

    def create_visnav_student_project(self, main_project, usergroup, protect_branches):

        post_data = dict(namespace_id=usergroup.id, path="visnav", name="visnav")
        print(f"Forking project {main_project.path_with_namespace} to " +
              f"{usergroup.full_path}/visnav and configuring",
              end='',
              flush=True)
        fork = None
        if not self.state.dry_run:
            # fork
            response = self.state.gl.http_post(f"/projects/{main_project.id}/fork", post_data=post_data)
            fork_id = response['id']
            # wait for import to finish
            while True:
                fork = self.state.gl.projects.get(fork_id)
                if fork.import_status == "finished":
                    break
                else:
                    print('.', end='', flush=True)
                    time.sleep(1)
            # break fork relation and customize project
            semester = Project(main_project).semester
            fork.delete_fork_relation()
            fork.description = (f"Personal project of user {usergroup.path} for code and exercise submission " +
                                f"(Practical Course: Vision Based Navigation, {semester.full_name()})\r\n")
            fork.save()
            fork = self.state.gl.projects.get(fork_id)

            for b in fork.branches.list(all=True):
                if b.name in ["master", "upstream"]:
                    if protect_branches:
                        b.protect()
                    else:
                        b.unprotect()
                else:
                    b.delete()
        print(' done')
        if fork is not None:
            print(f"Created {fork.path_with_namespace} (id: {fork.id})")
        return fork

    def check_visnav_repo_and_branches(self, project, protect_branches, confirm=True):
        while project.empty_repo:
            print(f"Visnav project {project.path_with_namespace} is empty. " +
                  f"Please push to {project.ssh_url_to_repo}")
            if click.confirm("Retry?", default=True):
                project = self.state.gl.projects.get(project.id)
            else:
                abort_cli()
        while True:
            branches = project.branches.list(all=True)
            print("Project branches:")
            AttributeTable(["name", ("commit", lambda x: x.commit['short_id']), "protected", "default"]).print(branches)
            desired_branches = ["master", "upstream"]
            if not set(desired_branches).issubset(set([b.name for b in branches])):
                print(f"Missing branches (desired: {desired_branches}). Please push to {project.ssh_url_to_repo}")
                if click.confirm("Retry?", default=True):
                    continue
                else:
                    abort_cli()
            else:
                for b in branches:
                    if b.name in desired_branches:
                        if b.protected != protect_branches:
                            protection_state = 'protected' if b.protected else 'unprotected'
                            if not confirm or click.confirm(f"Branch {b.name} is {protection_state}. Change?"):
                                if not self.state.dry_run:
                                    if protect_branches:
                                        b.protect()
                                    else:
                                        b.unprotect()
            break

    def ensure_project_permission(self, project, user, access_level):

        members = {p.id: p for p in project.members.list(all=True)}

        if user.id not in members:
            print(f"Adding user {user.username} to {project.path_with_namespace} " +
                  f"as {access_level_to_string(access_level)}")
            if not self.state.dry_run:
                project.members.create(dict(user_id=user.id, access_level=access_level))
        else:
            m = members[user.id]
            if m.access_level < access_level:
                print(f"Elevating user {user.username} in {project.path_with_namespace} " +
                      f"to {access_level_to_string(access_level)}")
                if not self.state.dry_run:
                    m.access_level = access_level
                    m.save()
            else:
                print(f"User {user.username} is already member of {project.path_with_namespace} " +
                      f"as {access_level_to_string(m.access_level)}")


class State:

    def __init__(self):
        self.dry_run = False
        self.update_only_one = False
        self._gitlab = None
        self.actions = Actions(self)
        self.helpers = Helpers(self)

    def __getattr__(self, item):
        # open gitlab connection on demand
        if item == "gl":
            if self._gitlab is None:
                self._gitlab = gitlab.Gitlab.from_config()  #per_page=100000000)
            return self._gitlab
        else:
            raise AttributeError("%s object has no attribute named %r" % (self.__class__.__name__, item))


pass_state = click.make_pass_decorator(State, ensure=True)


def set_state_option(*args, **kwargs):

    def decorator(f):

        def callback(ctx, param, value):
            state = ctx.ensure_object(State)
            setattr(state, param.name, value)
            return value

        return click.option(*args, expose_value=False, callback=callback, **kwargs)(f)

    return decorator


def dry_run_options(f):
    f = set_state_option('--dry-run', "-n", is_flag=True, help="Don't actually modify anything in gitlab.")(f)
    f = set_state_option('--update-only-one', is_flag=True,
                         help='Update only one row; then like dry-run for the rest.')(f)
    return f


@click.group(help="Manage gitlab repos and users for the visnav practical course.")
def cli():
    pass


@cli.group(name="users", help="inspect / manipulate gitlab practical course users")
def users_grp():
    pass


@users_grp.command('list', help='list users')
@click.option("--archived/--no-archived", show_default=True)
@click.option("--only-practical/--no-only-practical", default=True, show_default=True)
@click.option("--only-with-warnings/--no-only-with-warnings", default=False, show_default=True)
@click.option("--username", help="List only specified user.")
@pass_state
def list_users(state, archived, only_practical, only_with_warnings, username):

    if username:
        users = filter_none([state.helpers.get_user_with_username(username)])
    else:
        users = state.helpers.get_all_users(lambda u: (not only_practical or u.is_practical_course_account) and (
            archived or not u.is_archived) and (not only_with_warnings or len(u.warnings) > 0))

    AttributeTable([
        "id",
        "username",
        "name",
        "semester",
        "state",
        ("archived", "is_archived"),
        ("visnav", "is_visnav"),
        #"has_username_base_namespace",
        "ldap_uid",
        "email",
        #"has_ldap_temp_email",
        "created_on",
        "last_activity_on",
        ("unused", "is_unused"),
        ("#push", "push_count"),
        ("#membership", "membership_count"),
        "warnings",
    ]).print(users)


@users_grp.command('test', help='test users for email migration')
@click.option("--username", help="List only specified user.")
@click.option("--max-users", help="Limit output of users to specified number. 0 means no limit.", default=0)
@pass_state
def test_users(state, username, max_users):

    if username:
        users = filter_none([state.helpers.get_user_with_username(username)])
    else:
        users = state.helpers.get_all_users(max_results=max_users if max_users > 0 else None)

    # TODO: confirmed status
    AttributeTable([
        "id",
        "username",
        "name",
        "state",
        "ldap_uid",
        ("emails", "all_emails_verbose"),
        "created_on",
        "last_activity_on",
        "warnings",
    ]).print(users)


@users_grp.command(help='fix archived users')
@dry_run_options
@pass_state
def fix_archived(state):
    users = state.helpers.get_all_users(lambda u: u.is_practical_course_account and u.is_archived)
    state.helpers.apply_actions_users(users, state.actions.fix_archived_user)


@users_grp.command(help='archive old users')
@dry_run_options
@click.option("--semester", help="Semester up to which to archive users. Default: Current semester -2.")
@click.option("--username", help="Archive single user by username.")
@pass_state
def archive(state, semester, username):
    if semester and username:
        abort_cli("ERROR: cannot specify both semester and username")

    if username:
        users = [state.helpers.get_user_with_username(username)]
    else:
        if semester:
            newest_to_archive = Semester(semester)
        else:
            newest_to_archive = Semester.current().next_semester(-2)
        users = state.helpers.get_all_users(
            lambda u: u.is_practical_course_account and not u.is_archived and u.semester <= newest_to_archive)

    state.helpers.apply_actions_users(users, state.actions.archive_user)


@users_grp.command(help='create users for practical course')
@dry_run_options
@click.option("--usernames",
              required=True,
              help='comma-separated list of usersnames, or username ranges, e.g. s0060-s0079')
@click.option("--participants", type=click.File('r'), help='participants export from TUMonline')
@click.option("--delete-unused/--no-delete-unused", show_default=True)
@pass_state
def setup_visnav(state, usernames, participants, delete_unused):

    # read names from tumonline participants export
    participant_names = []
    if participants is not None:
        reader = csv.DictReader(participants, delimiter=',', quotechar='"')
        for row in reader:
            participant_names.append("{} {}".format(row['Vorname'], row['Familien- oder Nachname']))

    # parse username range
    usernames = state.helpers.username_spec_to_list(usernames)

    if delete_unused:
        users = state.helpers.get_all_users(lambda u: u.username in usernames)

        AttributeTable([
            "id",
            "username",
            "name",
            "semester",
            "state",
            ("archived", "is_archived"),
            ("visnav", "is_visnav"),
            "ldap_uid",
            "email",
            "created_on",
            "last_activity_on",
            ("unused", "is_unused"),
            ("#push", "push_count"),
            ("#membership", "membership_count"),
        ]).print(users)

        unused = [u for u in users if u.is_unused]

        if click.confirm("Delete unused users {}?".format(", ".join(u.username for u in unused))):
            for u in unused:
                print(f"Deleting {u.username}")
                if not state.dry_run:
                    u.delete()
    else:
        for username, name in itertools.zip_longest(usernames, participant_names):
            if username is None:
                break
            if name is None:
                name = "visnav " + username
            actions = state.actions.create_ldap_user(username=username, name=name)
            print(", ".join(actions))


@cli.group(name="projects", help='inspect / manipulate visnav projects and permissions')
def projects_grp():
    pass


@projects_grp.command("list", help='list visnav projects')
@pass_state
def list_projects(state):
    projects = state.helpers.get_all_visnav_projects()
    AttributeTable([
        "id",
        "semester",
        "name",
        "path_with_namespace",
        "username_base",
        ("created", "created_on"),
        ("last_activity", "last_activity_on"),
        "visibility",
        "is_old",
        "archived",
        "empty_repo",
        ("issues", "issues_enabled"),
        #"merge_requests_enabled",
        #"wiki_enabled",
        #"jobs_enabled",
        #"snippets_enabled",
        ("main", "is_visnav_main_repo"),
        ("prac", "is_visnav_practical_account_repo"),
        ("regular", "is_visnav_regular_account_repo"),
    ]).print(projects)


@cli.group(name="groups", help='inspect / manipulate visnav groups and permissions')
def groups_grp():
    pass


@groups_grp.command("list", help='list visnav groups')
@pass_state
def list_groups(state):
    groups = state.helpers.get_all_visnav_groups(with_managers=False)
    AttributeTable([
        "id",
        "semester",
        # "full_name",
        "name",
        "full_path",
        "username_base",
        "visibility",
        "is_old",
        ("main", "is_visnav_main_group"),
        ("prac", "is_visnav_practical_account_group"),
        ("regular", "is_visnav_regular_account_group"),
        "warnings"
    ]).print(groups)


@groups_grp.command(name='fix', help='fix visnav groups')
@dry_run_options
@pass_state
def fix_visnav_groups(state):
    groups = state.helpers.get_all_visnav_groups(with_managers=True)
    state.helpers.apply_actions_groups(groups, state.actions.fix_visnav_group)


@groups_grp.command(name="setup", help='setup visnav groups / projects for semester')
@dry_run_options
@click.option("--semester", help='semester to configure, e.g. ss20; default is current semester')
@click.option("--add-owner", help='usernames to add as owner (comma-separated list)')
@click.option("--protect-branches/--unprotect-branches",
              default=True,
              show_default=True,
              help='instead of protecting main branches, make them unprotected')
@click.option("--setup-users",
              help='usernames to setup as user with subgroup ' +
              '(comma-separated list of usernames, or username ranges, e.g. s0060-s0079)')
@pass_state
def setup_visnav_groups(state, semester, add_owner, protect_branches, setup_users):
    if semester is None:
        semester = Semester.current()
    else:
        semester = Semester(semester)

    # get main group
    group_name = f"visnav_{semester}"
    main_group = state.helpers.get_group_with_full_path(group_name)
    while main_group is None:
        if click.confirm(f"Main visnav group {group_name} not found. Do you want to create it?"):
            main_group = state.helpers.create_visnav_main_group(semester)
        else:
            abort_cli()
    # get group details
    main_group = state.gl.groups.get(main_group.id)
    print(f"Found group {group_name} (id: {main_group.id}, runners_token: {main_group.runners_token})")

    # check main group members
    new_owners = state.helpers.username_spec_to_list(add_owner) if add_owner else []
    for new_owner in new_owners:
        while True:
            members = main_group.members.all(all=True)
            print(f"Main group {main_group.full_path} members: " +
                  ", ".join([f"{m.username} ({access_level_to_string(m.access_level)})" for m in members]))
            if new_owner and new_owner not in [m.username for m in members
                                              ] and click.confirm(f"Add {new_owner} as owner?"):
                user = state.helpers.get_user_with_username(new_owner)
                if user is None:
                    print(f"ERROR: User {new_owner} not found.")
                    break
                print(f"Adding owner {new_owner}")
                if not state.dry_run:
                    main_group.members.create({'user_id': user.id, 'access_level': gitlab.OWNER_ACCESS})
                else:
                    break
            else:
                break

    # check runners
    runners = list(sorted(state.helpers.get_group_runners(main_group.id), key=lambda x: x.description))
    if runners:
        print("Group runners:")
        AttributeTable(["id", "description", "active", "status"]).print(runners)
    else:
        print("No group runners found.")

    # check main project
    main_project = state.helpers.get_project_with_path(group_name, parent=main_group)
    while main_project is None:
        if click.confirm(f"Main visnav project {group_name}/{group_name} not found. Do you want to create it?"):
            main_project = state.helpers.create_visnav_main_project(main_group)
        else:
            abort_cli()
    main_project = state.gl.projects.get(main_project.id)
    print(f"Found project {main_project.path_with_namespace} (id: {main_project.id})")

    # check main project members
    project_members = sorted(main_project.members.all(all=True), key=lambda m: m.access_level, reverse=True)
    print(f"Main project members: " +
          ", ".join([f"{m.username} ({access_level_to_string(m.access_level)})" for m in project_members]))

    # check current direct subgroups
    subgroups = main_group.subgroups.list(all=True)
    print("Main group subgroups: " + ", ".join([s.path for s in subgroups]))

    # check repo and branches
    state.helpers.check_visnav_repo_and_branches(main_project, protect_branches, confirm=True)

    # setup users
    confirm_user_creation = False
    if setup_users:
        setup_usernames = state.helpers.username_spec_to_list(setup_users)
        print("Setting up users: " + ", ".join(setup_usernames))
        for username in setup_usernames:
            user = state.helpers.get_user_with_username(username)
            if user is None:
                abort_cli(f"ERROR: User {username} not found.")

            print(f"Setting up for user {username} (id: {user.id})")

            # get / create group
            existing_subgroups = [s for s in subgroups if s.path == username]
            usergroup = None
            if existing_subgroups:
                usergroup = state.gl.groups.get(existing_subgroups[0].id)
                print(f"Subgroup {username} exists (id: {usergroup.id})")
            else:
                if (not confirm_user_creation) or click.confirm(f"Create subgroup {username}?", default=True):
                    print("Creating user subgroup")
                    if not state.dry_run:
                        props = dict(name=username, path=username, visibility="private", parent_id=main_group.id)
                        usergroup = state.gl.groups.create(props)
                        print(f"Created {usergroup.full_path} (id: {usergroup.id})")
            if usergroup is None:
                print("ERROR: Couldn't create or access user's group.")
                continue

            # get / create project
            userproject = state.helpers.get_project_with_path("visnav", parent=usergroup)
            if userproject:
                print(f"Project {userproject.path_with_namespace} exists (id: {userproject.id})")
            else:
                if (not confirm_user_creation) or click.confirm(f"Create project {username}/visnav?", default=True):
                    print("Creating user project")
                    userproject = state.helpers.create_visnav_student_project(main_project, usergroup, protect_branches)
            if userproject is None:
                print("ERROR: Couldn't create or access user's project.")
                continue

            # check repo and branches
            state.helpers.check_visnav_repo_and_branches(userproject, protect_branches, confirm=False)

            # check permissions
            state.helpers.ensure_project_permission(userproject, user, gitlab.DEVELOPER_ACCESS)
            state.helpers.ensure_project_permission(main_project, user, gitlab.GUEST_ACCESS)

            #pprint(userproject.attributes)

    # TODO: check subgroups repos; branch protection; access rights; outdated upstream; ...


if __name__ == "__main__":
    cli()
