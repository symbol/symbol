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
			choice name: 'PLATFORM',
				choices: params.platform ?: 'ubuntu',
				description: 'Run on specific platform'
		}

		agent {
			label 'ubuntu-agent'
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
					stage('Multibranch job') {
						steps {
							script {
								buildConfiguration = yamlHelper.readYamlFromFile(helper.resolveBuildConfigurationFile())
								createMonorepoMultibranchJobs(buildConfiguration, env.GIT_URL, env.JENKINS_ROOT_FOLDER, env.GITHUB_CREDENTIALS_ID)
							}
						}
					}
					stage('Nightly job') {
						steps {
							script {
								String repositoryName = env.GIT_URL.tokenize('/').last().split('\\.')[0]
								Map jobConfiguration = [:]
								jobConfiguration.jobName = "${env.JENKINS_ROOT_FOLDER}/${repositoryName}/nightlyJob"
								jobConfiguration.displayName = 'Nightly Job'
								jobConfiguration.jenkinsfilePath = 'jenkins/jenkinsfiles/Jenkinsfile-nightly.groovy'
								jobConfiguration.repositoryUrl = 'https://github.com/symbol/symbol.git'
								jobConfiguration.credentialsId = ''
								createPipelineJob(jobConfiguration)
							}
						}
					}
				}
			}
		}
	}
}
