#!/usr/bin/env bash
set -x

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
# shellcheck source=./shared-library/resources/artifacts/scripts/jenkins-functions.sh
. "$SCRIPT_DIR/jenkins-functions.sh"

node_load_version() {
	VERSION="$(npm run version --silent)"
	validate_env_variable "VERSION"
	echo -e "$VERSION"
}

node_publish_alpha() {
	VERSION="$(node_load_version)"
	validate_env_variable "VERSION"
	validate_env_variable "NPM_TOKEN"
	npm pack
	HASH=$(checksum -a sha256 "$(ls *.tgz)" | cut --d ' ' -f 1)
	echo "Uploading npm package version $NEW_VERSION with ${HASH}"
	npm version "$NEW_VERSION" --commit-hooks false --git-tag-version false
	npm publish --tag alpha
}

node_publish_release() {
	VERSION="$(node_load_version)"
	validate_env_variable "VERSION"
	if [ "$SKIP_RELEASE_PUBLISH" = "true" ]; then
		echo "Skipping publishing of sdk artifacts"
		echo ""
	else
		validate_env_variable "NPM_TOKEN"
		echo "Publishing $GIT_URL artifacts"
		npm publish
		echo ""
	fi
}

node_post_release() {
	VERSION="$(node_load_version)"
	validate_env_variable "VERSION"
	validate_env_variable "RELEASE_BRANCH"
	validate_env_variable "REMOTE_NAME"
	validate_env_variable "POST_RELEASE_BRANCH"
	checkout_branch "${RELEASE_BRANCH}"

	git tag -fa "v$VERSION" -m "Releasing version $VERSION"

	echo "Increasing artifact version"
	npm version patch -m "Increasing version to %s" --git-tag-version false

	NEW_VERSION=$(npm run version --silent)

	echo "New Version"
	echo "$NEW_VERSION"
	echo ""

	git add .
	git commit -m "Creating new version $NEW_VERSION"

	echo "Pushing code to $REMOTE_NAME $POST_RELEASE_BRANCH"
	git push --set-upstream "$REMOTE_NAME" "$RELEASE_BRANCH:$POST_RELEASE_BRANCH"
	echo "Pushing tags to $REMOTE_NAME"
	git push --tags "$REMOTE_NAME"
}

if [ "$1" == "alpha" ]; then
	node_publish_alpha
fi

if [ "$1" == "release" ]; then
	node_publish_release
fi

if [ "$1" == "post_release" ]; then
	node_post_release
fi
