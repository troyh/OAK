#!/bin/bash

. `dirname $0`/util.sh

URL="http://dev:81/api/createlogin"

# Try creating an account without an email address
TEST "password=foo&userid=test-user1" 501
TEST "email=&password=foo&userid=test-user1" 501

# Try creating an account without a userid
TEST "email=test-user1@foo.com&password=foo" 501
TEST "email=test-user1@foo.com&password=foo&userid=" 501

# Try creating a new account without a password
TEST "email=test-user1@foo.com&userid=test-user1" 501
TEST "email=test-user1@foo.com&password=&userid=test-user1" 501

# Try creating a legit account
TEST "email=test-user1@foo.com&password=foo&userid=test-user1" 200

# Try re-creating an existing account
TEST "email=test-user1@foo.com&password=foo&userid=test-user1" 501

# Try creating a new account with an existing userid
TEST "email=test-user2@foo.com&password=foo&userid=test-user1" 501

# Try creating a new account with an existing email
TEST "email=test-user1@foo.com&password=foo&userid=test-user2" 501

# Try very long values
TEST "email=test-user2@foofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoo.com&password=foofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoo&userid=test-verylongname-foofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoofoo" 501

