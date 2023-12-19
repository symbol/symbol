gitPullRequestPipeline {
	dockerImageName = 'symbolplatform/build-ci:cpp-ubuntu-lts'
	branchName = 'fix/update_catapult_dependency'
	scriptSetupCommand = 'python3 -m pip install -r jenkins/catapult/requirements.txt'
	srcriptCommand = 'python3 jenkins/catapult/catapultDependencyUpdater.py --versions-file jenkins/catapult/versions.properties --source-path client/catapult'
	reviewers = []
}
