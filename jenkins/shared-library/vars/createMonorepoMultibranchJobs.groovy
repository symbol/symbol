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
	createJenkinsFolder("${rootFolder}/${jobConfiguration.repositoryName}", jobConfiguration.repositoryName)

	jobConfiguration.fullBranchFolder = Paths.get(rootFolder).resolve(jobConfiguration.repositoryName).toString()
	generatePackageMultibranchJobs(buildConfiguration, jobConfiguration)
}

// Create a multibranch job for each package in the monorepo
void generatePackageMultibranchJobs(Object buildConfiguration, Map jobConfiguration) {
	final String pathSeparator = '/'
	List<String> jobPathList = []

	buildConfiguration.builds.each { build ->
		jobConfiguration.packageExcludePaths = '**/*'
		String pipelineName = build.path.replaceAll(pathSeparator, '-')
		jobConfiguration.jobName = Paths.get(jobConfiguration.fullBranchFolder.toString()).resolve(pipelineName).toString()
		jobConfiguration.jenkinsfilePath = Paths.get(build.path).resolve('Jenkinsfile').toString()
		pathList = addPathAndDependsOnFolder(build)
		pathList.add(helper.resolveBuildConfigurationFile())
		jobConfiguration.packageIncludePaths = pathList.join('\n')
		jobConfiguration.displayName = build.name
		jobConfiguration.packageFolder = Paths.get(jobConfiguration.repositoryName.toString()).resolve(build.path).toString()
		jobConfiguration.daysToKeep = build.logRotation
		renameJobIfDisplayNamePresent(jobConfiguration.fullBranchFolder, jobConfiguration.displayName, jobConfiguration.jobName)
		createMultibranchJob(jobConfiguration)
		jobPathList.add(jobConfiguration.jobName)
	}

	removeOldJobs(jobConfiguration.fullBranchFolder, jobPathList)
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

void renameJobIfDisplayNamePresent(String jobFolder, String jobDisplayName, String fullJobName) {
	Item job = Jenkins.get().getItemByFullName(fullJobName)
	if (null != job) {
		return
	}

	List<Item> jobs = Jenkins.get().getItemByFullName(jobFolder).items
	for (Item item in jobs) {
		println("Checking Item: ${item.displayName}, ${item.getClass()} ${item.fullName}")
		if (org.jenkinsci.plugins.workflow.multibranch.WorkflowMultiBranchProject == item.getClass()
				&& item.displayName == jobDisplayName
				&& item.fullName != fullJobName) {
			String newJobName = fullJobName.tokenize('/').last()
			println("Renaming job: ${item.fullName} to ${newJobName}")
			item.renameTo(newJobName)
			break
		}
	}
}

void removeOldJobs(String jobFolder, List<String> currentJobList) {
	List<Item> jobs = Jenkins.get().getItemByFullName(jobFolder).items
	println("Checking jobs: ${currentJobList}")
	for (Item item in jobs) {
		println("Checking Item remove: ${item.name}, ${item.getClass()} ${item.fullName}")
		if (org.jenkinsci.plugins.workflow.multibranch.WorkflowMultiBranchProject == item.getClass()
				&& currentJobList.indexOf(item.fullName) == -1
				&& !isJobActive(item)) {
			println("Removing old job: ${item.fullName}")
			item.delete()
		}
	}
}

boolean isJobActive(Item job) {
	if (!job.isBuildable()) {
		return false
	}

	return job.allJobs.any { item -> item.isBuildable() }
}
