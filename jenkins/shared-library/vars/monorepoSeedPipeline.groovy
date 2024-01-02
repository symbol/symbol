import java.nio.file.Paths

// groovylint-disable-next-line MethodSize
void call(Closure body) {
	Map jenkinsfileParams = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = jenkinsfileParams
	body()

	pipeline {
		parameters {
			choice name: 'OPERATING_SYSTEM',
				choices: jenkinsfileParams.operatingSystem ?: ['ubuntu'],
				description: 'Run on specific OS'
			choice name: 'ARCHITECTURE',
				choices: ['arm64', 'amd64'],
				description: 'Computer architecture'
		}

		agent {
			label """${
				env.OPERATING_SYSTEM = env.OPERATING_SYSTEM ?: "${jenkinsfileParams.operatingSystem[0]}"
				env.ARCHITECTURE = env.ARCHITECTURE ?: 'arm64'
				return helper.resolveAgentName(env.OPERATING_SYSTEM, env.ARCHITECTURE, jenkinsfileParams.instanceSize ?: 'small')
			}"""
		}

		options {
			ansiColor('css')
			timestamps()
		}

		environment {
			JENKINS_ROOT_FOLDER = "${jenkinsfileParams.organizationName}/generated"
			GITHUB_CREDENTIALS_ID = "${jenkinsfileParams.gitHubId}"
		}

		stages {
			stage('create controller jobs') {
				when {
					beforeAgent true
					anyOf {
						expression { changedSetHelper.isFileInChangedSet('.github/jenkinsfile/controller.groovy') }
						triggeredBy 'UserIdCause'
					}
				}
				stages {
					stage('display environment') {
						steps {
							sh 'printenv'
						}
					}
					stage('checkout') {
						when {
							triggeredBy 'UserIdCause'
						}
						steps {
							script {
								runScript("git reset --hard origin/${env.BRANCH_NAME}")
							}
						}
					}
					stage('Controller multibranch job') {
						steps {
							script {
								generateControllerMultibranchJob(env.GIT_URL, env.JENKINS_ROOT_FOLDER, env.GITHUB_CREDENTIALS_ID)
							}
						}
					}
				}
			}
		}
	}
}

void generateControllerMultibranchJob(String gitUrl, String rootFolder, String credentialsId) {
	Map jobConfiguration = [:]

	jobConfiguration.gitUrl = gitUrl
	jobConfiguration.rootFolder = rootFolder
	jobConfiguration.credentialsId = credentialsId
	jobConfiguration.repositoryOwner = helper.resolveOrganizationName()
	jobConfiguration.repositoryName = helper.resolveRepositoryName()
	jobConfiguration.packageExcludePaths = ''
	jobConfiguration.fullBranchFolder = Paths.get(rootFolder).resolve(jobConfiguration.repositoryName).toString()
	jobConfiguration.jobName = Paths.get(jobConfiguration.fullBranchFolder.toString()).resolve('controller').toString()
	jobConfiguration.jenkinsfilePath = '.github/jenkinsfile/controller.groovy'
	jobConfiguration.packageIncludePaths = ''
	jobConfiguration.displayName = 'Controller'
	jobConfiguration.packageFolder = "${jobConfiguration.repositoryName}/controller"
	jobConfiguration.daysToKeep = 30
	generateJobFolder(jobConfiguration.jobName)
	createMultibranchJob(jobConfiguration, false)
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
