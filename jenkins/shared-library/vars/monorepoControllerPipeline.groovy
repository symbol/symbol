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
			booleanParam name: 'SHOULD_CREATE_PIPELINE', description: 'true re-create pipeline and multibranch jobs', defaultValue: false
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
			disableConcurrentBuilds(abortPrevious: true)
			timestamps()
		}

		environment {
			GITHUB_CREDENTIALS_ID = helper.resolveGitHubCredentialsId()
			JENKINS_ROOT_FOLDER = Paths.get(currentBuild.fullProjectName).parent.parent.parent.toString()
		}

		stages {
			stage('display environment') {
				steps {
					sh 'printenv'
				}
			}
			stage('checkout') {
				when {
					expression { helper.isManualBuild(env.MANUAL_GIT_BRANCH) }
				}
				steps {
					script {
						sh "git checkout ${helper.resolveBranchName(env.MANUAL_GIT_BRANCH)}"
						sh "git reset --hard origin/${helper.resolveBranchName(env.MANUAL_GIT_BRANCH)}"
					}
				}
			}
			stage('create pipeline jobs') {
				when {
					anyOf {
						expression { changedSetHelper.isFileInChangedSet(helper.resolveBuildConfigurationFile()) }
						expression { env.SHOULD_CREATE_PIPELINE?.toBoolean() }
					}
				}
				stages {
					stage('read build configuration') {
						steps {
							script {
								buildConfiguration = yamlHelper.readYamlFromFile(helper.resolveBuildConfigurationFile())
							}
						}
					}
					stage('Multibranch job') {
						when {
							expression {
								return null != buildConfiguration.builds
							}
						}
						steps {
							script {
								createMonorepoMultibranchJobs(buildConfiguration, env.GIT_URL, env.JENKINS_ROOT_FOLDER, env.GITHUB_CREDENTIALS_ID)
							}
						}
					}
					stage('Pipeline job') {
						when {
							expression {
								return null != buildConfiguration.customBuilds
							}
						}
						steps {
							script {
								createMonorepoPipelineJobs(buildConfiguration, env.GIT_URL, env.JENKINS_ROOT_FOLDER, env.GITHUB_CREDENTIALS_ID)
							}
						}
					}
				}
			}
			stage('build jobs') {
				steps {
					script {
						triggerJobs(helper.resolveBranchName(env.MANUAL_GIT_BRANCH))
					}
				}
			}
		}
	}
}

void triggerJobs(String branchName) {
	final Object buildConfiguration = jobHelper.loadBuildConfigurationfile()
	final Map<String, String> allJenkinsfiles = jobHelper.loadJenkinsfileMap(buildConfiguration)
	final Map<String, String> triggeredJenkinsfile = changedSetHelper.findMultibranchPipelinesToRun(allJenkinsfiles)
	final Map<String, String> dependencyJenkinsfile = findDependencyMultibranchPipelinesToRun(triggeredJenkinsfile, buildConfiguration)
	final String currentJobName = Paths.get(currentBuild.fullProjectName).parent
	final Map<String, String> jenkinsfilesJobToRun = triggeredJenkinsfile + dependencyJenkinsfile
	Map<String, String> siblingNameMap = jobHelper.siblingJobNames(jenkinsfilesJobToRun, currentJobName)

	if (siblingNameMap.size() == 0) {
		return
	}

	Map<String, Closure> buildJobs = [:]
	final String jobName = jobHelper.resolveJobName(siblingNameMap.keySet().toArray()[0], branchName)

	siblingNameMap.each { String jobPath, String displayName ->
		buildJobs["${displayName}"] = {
			stage("${displayName}") {
				final String fullJobName = jobPath + '/' + jobName
				final String jenkinsfilePath = jenkinsfilesJobToRun.get(displayName)
				final Map<String, String> jenkinsfileParameters = jobHelper.readJenkinsFileParameters(jenkinsfilePath)
				final String osValue = jobHelper.resolveOperatingSystem(jenkinsfileParameters.operatingSystem)

				// For new branches, Jenkins will receive an event from the version control system to provision the
				// corresponding Pipeline under the Multibranch Pipeline item. We have to wait for Jenkins to process the
				// event so a build can be triggered.
				timeout(time: 5, unit: 'MINUTES') {
					waitUntil(initialRecurrencePeriod: 1e3) {
						Item pipeline = Jenkins.instance.getItemByFullName(fullJobName)
						pipeline && !pipeline.isDisabled()
					}
				}

				build job: "${fullJobName}", parameters: [
					gitParameter(name: 'MANUAL_GIT_BRANCH', value: branchName),
					string(name: 'OPERATING_SYSTEM', value: osValue ?: 'ubuntu'),
					string(name: 'BUILD_CONFIGURATION', value: 'release-private'),
					string(name: 'TEST_MODE', value: 'code-coverage'),
					string(name: 'ARCHITECTURE', value: params.ARCHITECTURE),
					booleanParam(name: 'SHOULD_PUBLISH_IMAGE', value: false),
					booleanParam(name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', value: false)],
					wait: true,
					propagate: true
			}
		}
	}

	parallel buildJobs
}

Map<String, String> findDependencyMultibranchPipelinesToRun(Map<String, String> jenkinsfileMap, Object buildConfiguration) {
	List<Object> dependencyBuilds =  jenkinsfileMap.collectMany { String displayName, String jenkinsfilePath ->
		buildConfiguration.builds.findAll { build -> build.dependsOn.find { dependsOnPath -> dependsOnPath == jenkinsfilePath } }
	}.unique()

	println "triggered builds: ${jenkinsfileMap}"
	println "dependency builds required: ${dependencyBuilds}"

	Map<String, String> dependencyJobMap = [:]
	dependencyBuilds.each { build -> dependencyJobMap.put(build.name, build.path) }
	return dependencyJobMap
}
