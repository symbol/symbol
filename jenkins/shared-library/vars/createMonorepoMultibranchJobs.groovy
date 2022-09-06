import java.nio.file.Paths

void call(Object buildConfiguration, String gitUrl, String rootFolder, String credentialsId) {
	println("Build configuration: ${buildConfiguration}")
	generateMultibranchJobs(buildConfiguration, gitUrl, rootFolder, credentialsId)
}

String appendFilter(String path) {
	return "${path}/**/*"
}

// Create multibranch jobs for the monorepo
void generateMultibranchJobs(Object buildConfiguration, String gitUrl, String rootFolder, String credentialsId) {
	final String pathSeparator = '/'
	Map jobConfiguration = [:]

	jobConfiguration.gitUrl = gitUrl
	jobConfiguration.rootFolder = rootFolder
	jobConfiguration.credentialsId = credentialsId

	Object[] tokens = gitUrl.tokenize(pathSeparator)
	jobConfiguration.repositoryOwner = tokens[2]
	jobConfiguration.repositoryName = tokens.last().split('\\.')[0]

	String fullFolderName = pathSeparator
	rootFolder.tokenize(pathSeparator).each { folderName ->
		fullFolderName += folderName
		createJenkinsFolder(fullFolderName, "${folderName}")
		fullFolderName += pathSeparator
	}
	createJenkinsFolder("${rootFolder}/${jobConfiguration.repositoryName}", "Branch ${jobConfiguration.repositoryName}")

	jobConfiguration.fullBranchFolder = Paths.get(rootFolder).resolve(jobConfiguration.repositoryName).toString()
	generatePackageMultibranchJobs(buildConfiguration, jobConfiguration)
}

// Create a multibranch job for each package in the monorepo
void generatePackageMultibranchJobs(Object buildConfiguration, Map jobConfiguration) {
	final String pathSeparator = '/'

	buildConfiguration.builds.each { build ->
		jobConfiguration.packageExcludePaths = '**/*'
		String pipelineName = build.path.tokenize(pathSeparator).last()
		jobConfiguration.jobName = Paths.get(jobConfiguration.fullBranchFolder.toString()).resolve(pipelineName).toString()
		jobConfiguration.jenkinsfilePath = Paths.get(build.path).resolve('Jenkinsfile').toString()
		pathList = addPathAndDependsOnFolder(build)
		pathList.add(helper.resolveBuildConfigurationFile())
		jobConfiguration.packageIncludePaths = pathList.join('\n')
		jobConfiguration.displayName = build.name
		jobConfiguration.packageFolder = Paths.get(jobConfiguration.repositoryName.toString()).resolve(build.path).toString()
		createMultibranchJob(jobConfiguration)
	}
}

List<String> addPathAndDependsOnFolder(Object build) {
	List<String> packagePaths = [appendFilter(build.path)]
	if (null != build.dependsOn) {
		build.dependsOn.each { dependsOnPath ->
			packagePaths += appendFilter(dependsOnPath)
		}
	}
	return packagePaths
}
