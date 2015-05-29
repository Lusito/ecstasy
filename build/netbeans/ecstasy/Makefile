# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin

# build
build: .build-post

.build-pre:

.build-post: .build-impl

# clean
clean: .clean-post

.clean-pre:

.clean-post: .clean-impl

# clobber
clobber: .clobber-post

.clobber-pre:

.clobber-post: .clobber-impl

# all
all: .all-post

.all-pre:

.all-post: .all-impl

# build tests
build-tests: .build-tests-post

.build-tests-pre:

.build-tests-post: .build-tests-impl

# run tests
test: .test-post

.test-pre: build-tests

.test-post: .test-impl

# help
help: .help-post

.help-pre:

.help-post: .help-impl

# include project implementation makefile
include nbproject/Makefile-impl.mk

# include project make variables
include nbproject/Makefile-variables.mk
