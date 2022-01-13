#!/usr/bin/env bash
set -x

export REMOTE_NAME="origin"
export FUNCTIONS_VERSION="0.1.2"
COMMIT_MESSAGE="$(git show -s --format=%B)"

test_functions() {
	echo "Functions Loaded"
}

increment_version() {
	declare -a part=( "${1//\./ }" )
	declare new
	declare -i carry=1

	for (( COUNTER=${#part[@]}-1; COUNTER>=0; COUNTER-=1 )); do
		len=${#part[COUNTER]}
		new=$((part[COUNTER]+carry))
		[ "${#new}" -gt "$len" ] && carry=1 || carry=0
		[ "$COUNTER" -gt 0 ] && part[COUNTER]=${new: -len} || part[COUNTER]=${new}
	done
	new="${part[*]}"
	echo -e "${new// /.}"
}

log_env_variables() {
	echo "DEV_BRANCH = $DEV_BRANCH"
	echo "POST_RELEASE_BRANCH = $POST_RELEASE_BRANCH"
	echo "RELEASE_BRANCH = $RELEASE_BRANCH"
	echo "REMOTE_NAME = $REMOTE_NAME"
	echo "DOCKER_IMAGE_NAME = $DOCKER_IMAGE_NAME"
	echo "COMMIT_MESSAGE = $COMMIT_MESSAGE"
	echo "GIT_URL = $GIT_URL"
	echo "FUNCTIONS_VERSION = $FUNCTIONS_VERSION"
	echo "VERSION = $VERSION"
}

validate_env_variables() {
	log_env_variables
	validate_env_variable "RELEASE_BRANCH"
	validate_env_variable "POST_RELEASE_BRANCH"
	validate_env_variable "DEV_BRANCH"
	validate_env_variable "COMMIT_MESSAGE"
}

validate_env_variable() {
	var="$1"
	if [ "${!var}" = "" ]
	then
		echo "Env $var has not been provided"
		exit 128
	fi
}

checkout_branch() {
	CHECKOUT_BRANCH="$1"
	validate_env_variable "GIT_URL"
	validate_env_variable "CHECKOUT_BRANCH"
	validate_env_variable "GITHUB_TOKEN"
	validate_env_variable "REMOTE_NAME"

	git remote rm $REMOTE_NAME
	echo "Setting remote url $GIT_URL"
	git remote add $REMOTE_NAME "$GIT_URL" >/dev/null 2>&1
	echo "Checking out $CHECKOUT_BRANCH as head detached."
	git checkout "$CHECKOUT_BRANCH"
}

load_version_from_file() {
	VERSION="$(head -n 1 version.txt)"
	echo -e "$VERSION"
}

post_release_version_file() {
	validate_env_variable "RELEASE_BRANCH"
	validate_env_variable "REMOTE_NAME"
	validate_env_variable "POST_RELEASE_BRANCH"
	checkout_branch "${RELEASE_BRANCH}"
	VERSION="$(load_version_from_file)"

	NEW_VERSION=$(increment_version "$VERSION")

	echo "Version: $VERSION"
	echo "New Version: $NEW_VERSION"

	echo "Creating tag version v$VERSION"
	git tag -fa "v$VERSION" -m "Releasing version $VERSION"

	echo "Creating new version $NEW_VERSION"
	echo "$NEW_VERSION" > 'version.txt'
	git add version.txt
	git commit -m "Creating new version $NEW_VERSION"

	echo "Pushing code to $REMOTE_NAME $POST_RELEASE_BRANCH"
	git push "$REMOTE_NAME" "$RELEASE_BRANCH:$POST_RELEASE_BRANCH"
	echo "Pushing tags to $REMOTE_NAME"
	git push --tags "$REMOTE_NAME"
}

if [ "$1" == "post_release_version_file" ]; then
	post_release_version_file
fi
