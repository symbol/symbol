// groovylint-disable-next-line MethodSize
void call(Closure body) {
	Map params = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = params
	body()

	pipeline {
		parameters {
			gitParameter branchFilter: 'origin/(.*)',
				defaultValue: "${env.GIT_BRANCH}",
				name: 'MANUAL_GIT_BRANCH',
				type: 'PT_BRANCH',
				selectedValue: 'TOP',
				sortMode: 'ASCENDING',
				useRepository: "${helper.resolveRepoName()}"
			choice name: 'OPERATING_SYSTEM',
				choices: params.operatingSystem ?: 'ubuntu',
				description: 'Run on specific OS'
			choice name: 'ARCHITECTURE',
				choices: ['amd64', 'arm64'],
				description: 'Computer architecture'
		}

		agent {
			label """${helper.resolveAgentName(
					env.OPERATING_SYSTEM ?: "${params.operatingSystem[0]}",
					env.ARCHITECTURE ?: 'amd64',
					params.instanceSize ?: 'small'
			)}"""
		}

		options {
			ansiColor('css')
			timestamps()
		}

		environment {
			JENKINS_ROOT_FOLDER = "${params.organizationName}/generated"
			GITHUB_CREDENTIALS_ID = "${params.gitHubId}"
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
						gitCheckout(helper.resolveBranchName(env.MANUAL_GIT_BRANCH), env.GITHUB_CREDENTIALS_ID, env.GIT_URL)
						helper.runInitializeScriptIfPresent()
					}
				}
			}
			stage('create pipeline jobs') {
				when {
					expression {
						return fileExists(helper.resolveBuildConfigurationFile())
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
		}
	}
}
