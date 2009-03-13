#!/bin/bash

. `dirname $0`/util.sh

URL="http://dev:81/api/login"

TEST "email=test-user1@foo.com&password=foo" 200
TEST "email=test-user1@foo.com&password=foobar" 401
TEST "email=test-noexistentuser1@foo.com&password=foo" 401
TEST "email=test-user1@foo.com&password=" 400
TEST "email=&password=" 400
TEST "email=" 400
TEST "password=" 400
TEST "" 400
