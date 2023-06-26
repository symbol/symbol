import java.nio.file.Paths

void call(Object buildConfiguration, String gitUrl, String rootFolder, String credentialsId) {
	println("Build configuration: ${buildConfiguration}")
	generateMonorepoPipelineJobs(buildConfiguration, gitUrl, rootFolder, credentialsId)
}

void generateJobFolder(String jobFullName) {
	final String pathSeparator = '/'
	String jobFolder = Paths.get(jobFullName).parent
	String fullFolderName = pathSeparator

	jobFolder.tokenize(pathSeparator).each { folderName ->
		fullFolderName += folderName
		createJenkinsFolder(fullFolderName, "${folderName}")
		fullFolderName += pathSeparator
	}
}

// Create pipeline jobs for the monorepo
void generateMonorepoPipelineJobs(Object buildConfiguration, String gitUrl, String rootFolder, String credentialsId) {
	final String pathSeparator = '/'
	Object[] tokens = gitUrl.tokenize(pathSeparator)
	String repositoryOwner = tokens[2]
	String repositoryName = tokens.last().split('\\.')[0]
	String ownerAndProject = "${repositoryOwner}/${repositoryName}"
	String fullBranchFolder = Paths.get(rootFolder).resolve(repositoryName)

	buildConfiguration.customBuilds.each { build ->
		Map jobConfiguration = [:]
		jobConfiguration.gitUrl = gitUrl
		jobConfiguration.credentialsId = credentialsId
		jobConfiguration.ownerAndProject = ownerAndProject
		jobConfiguration.jobName = build.jobName.startsWith(pathSeparator)
				? build.jobName
				: Paths.get(fullBranchFolder.toString()).resolve(build.jobName).toString()
		generateJobFolder(jobConfiguration.jobName)
		jobConfiguration.displayName = build.name
		jobConfiguration.buildsToKeep = build.logRotation
		addTriggers(build.triggers, jobConfiguration)
		jobConfiguration.jenkinsfilePath = build.targetDirectory
				? Paths.get(build.targetDirectory.toString()).resolve(build.scriptPath.toString()).toString()
				: build.scriptPath
		jobConfiguration.targetDirectory = build.targetDirectory
		createPipelineJob(jobConfiguration)
	}
}

void addTriggers(Object triggers, Map jobConfiguration) {
	Map addTrigger = [
			'cron': { Map job, Object trigger ->
				job.cronTrigger = trigger.schedule
			}
	]

	triggers.each { trigger -> addTrigger[trigger.type](jobConfiguration, trigger) }
}
