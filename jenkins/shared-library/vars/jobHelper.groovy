import java.util.regex.Matcher

Map<String, String> readJenkinsFileParameters(String jenkinsfilePath) {
	String content = readFile "${jenkinsfilePath}/Jenkinsfile"
	Matcher matcher = content =~ '(?ms)defaultCiPipeline \\{(.*)}'
	String matchValue = matcher[0][1]?.toString().trim()

	// set matcher to null to avoid serialization exception
	matcher = null

	Map<String, String> parameters = readProperties text: matchValue

	println "Found parameters: ${parameters} in ${jenkinsfilePath}"
	return parameters
}

List<String> readArrayParameterValue(String parameterValue) {
	// Disable due to https://issues.jenkins.io/browse/JENKINS-33051
	// And don't want to bypass CPS
	// groovylint-disable-next-line UnnecessaryCollectCall
	List<String> values = parameterValue.replaceAll("(\\[|\\]|')", '').split(',').collect { String value -> value.trim() }
	println "${parameterValue} -> ${values}"
	return values
}

Map<String, String> siblingJobNames(Map<String, String> displayNameJenkinsfileMap, String jobFolder) {
	Item project = Jenkins.get().getItemByFullName(jobFolder)
	List<Item> siblingItems = project.parent.items

	Map<String, String> targets = [:]
	for (Item item in siblingItems) {
		if (org.jenkinsci.plugins.workflow.multibranch.WorkflowMultiBranchProject != item.getClass()
				|| item.fullName == project.fullName
				// skip jobs that are not in the build configuration file for this branch
				|| !displayNameJenkinsfileMap.containsKey(item.displayName)) {
			echo "Skipping job - ${item.fullName}"
			continue
		}

		targets.put(item.fullName, item.displayName)
	}

	println "targets: ${targets}"
	return targets
}

String resolveJobName(String jobFolder, String branchName) {
	final String separator = '/'
	String encodeJobName = branchName.replaceAll(separator, '%2F')
	String fullJobName = jobFolder + separator + encodeJobName
	Item job = Jenkins.get().getItemByFullName(fullJobName)
	if (job?.isBuildable()) {
		return encodeJobName
	}

	// find the PR that matches the branch
	List<Item> jobs = Jenkins.get().getItemByFullName(jobFolder).items
	for (Item item in jobs) {
		if (item.name.startsWith('PR-') && item.isBuildable()) {
			if (env.CHANGE_BRANCH == branchName) {
				echo "found PR: ${item.name}"
				return item.name
			}
		}
	}

	throw new IllegalStateException("Multibranch job folder ${jobFolder} does not contain a job with branch ${branchName}")
}

Object loadBuildConfigurationfile() {
	String buildConfiguration = readTrusted(helper.resolveBuildConfigurationFile())
	return yamlHelper.readYamlFromText(buildConfiguration)
}

Map <String, String> loadJenkinsfileMap(Object buildConfiguration = loadBuildConfigurationfile()) {
	Map<String, String> displayNameJenkinsfileMap = [:]

	buildConfiguration.builds.each { build -> displayNameJenkinsfileMap.put(build.name, build.path) }
	return displayNameJenkinsfileMap
}

String resolveOperatingSystem(String osValue) {
	List<String> osValues = readArrayParameterValue(osValue)
	return osValues.isEmpty() ? 'ubuntu' : osValues[0]
}

Object loadBuildEnvironment(String environmentFile) {
	String data = libraryResource resource: environmentFile
	return yamlHelper.readYamlFromText(data)
}

Object loadBuildBaseImages() {
	return loadBuildEnvironment('buildEnvironment/baseImages.yaml')
}

String resolveBuildImageName(String environment) {
	return environment.contains(':') ? environment : "symbolplatform/build-ci:${environment}"
}

String resolveCiEnvironmentName(Map params) {
	if (params.environment?.trim()) {
		return params.environment.replaceAll("'", '')
	}

	String[] parts = params.ciBuildDockerfile.split('\\.').collect { String part -> part.replaceAll("'", '').trim() }
	println("environment: ${parts[0]}")
	return parts[0]
}
