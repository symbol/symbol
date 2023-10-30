// This will return list of files Jenkins thinks have changed, except for new branches where it will return an empty list.
List<String> jenkinsChangedFiles() {
	List<String> files = currentBuild.changeSets*.items*.affectedPaths
	return files.flatten().unique()
}

// This will return all the files in the pull request
List<String> pullRequestChangedFiles() {
	// CHANGE_ID is set only for pull requests
	return env.CHANGE_ID ? pullRequest.files*.filename : []
}

// Jenkins native interface to retrieve changes, i.e. `currentBuild.changeSets`, returns an empty list for newly
// created branches (see https://issues.jenkins.io/browse/JENKINS-14138), so let's use `git` instead.
List<String> lastCommitChangedFiles() {
	return runScript('git diff-tree --no-commit-id --name-only -r HEAD', true).trim().split('\n')
}

List<String> resolveChangedList() {
	List<Closure> changedFileHandler = [
		this.&jenkinsChangedFiles,
		this.&pullRequestChangedFiles,
		this.&lastCommitChangedFiles
	]

	for (Closure handler in changedFileHandler) {
		List<String> changedFiles = handler()
		if (!changedFiles.isEmpty()) {
			return changedFiles
		}
	}

	return []
}

boolean isFileInChangedSet(String fileName) {
	return resolveChangedList().find { changedFilePath -> changedFilePath == fileName }
}

Map<String, String> findRelevantMultibranchPipelines(List<String> changedFilesPath, Map<String, String> jenkinsBuilds) {
	return jenkinsBuilds.findAll { build ->
		changedFilesPath.find { changedFilePath -> changedFilePath.startsWith(build.value) }
	}
}

Map<String, String> findMultibranchPipelinesToRun(Map<String, String> jenkinsBuilds) {
	return findRelevantMultibranchPipelines(resolveChangedList(), jenkinsBuilds)
}
