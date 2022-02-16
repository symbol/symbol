import java.nio.file.Paths

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
			JENKINS_ROOT_FOLDER	  = "${params.organizationName}/generated"
			BUILD_CONFIGURATION_FILE = 'buildConfiguration.yaml'
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
						return fileExists(resolveBuildConfigurationFile(env.BUILD_CONFIGURATION_FILE))
					}
				}
				steps {
					script {
						buildConfiguration = yamlHelper.readYamlFromFile(resolveBuildConfigurationFile(env.BUILD_CONFIGURATION_FILE))
						createMonorepoMultibranchJobs(buildConfiguration, env.GIT_URL, env.JENKINS_ROOT_FOLDER, env.GITHUB_CREDENTIALS_ID)
					}
				}
			}
		}
	}
}

String resolveBuildConfigurationFile(String configurationFile)  {
	repositoryName = helper.resolveRepoName().tokenize('.')[0]
	String filepath = Paths.get('jenkins/seed-job').resolve(repositoryName).resolve(configurationFile)
	logger.logInfo("build configuration file: ${filepath}")
	return filepath
}
