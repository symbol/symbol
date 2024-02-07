import java.nio.file.Paths
import org.jenkinsci.plugins.badge.EmbeddableBadgeConfig

// groovylint-disable-next-line MethodSize
void call(Closure body) {
	Map jenkinsfileParams = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = jenkinsfileParams
	body()

	final String packageRootPath = findJenkinsfilePath()

	pipeline {
		parameters {
			choice name: 'OPERATING_SYSTEM',
				choices: jenkinsfileParams.operatingSystem ?: ['ubuntu'],
				description: 'Operating System'
			choice name: 'BUILD_CONFIGURATION',
				choices: ['release-private', 'release-public'],
				description: 'build configuration'
			choice name: 'ARCHITECTURE',
				choices: ['arm64', 'amd64'],
				description: 'Computer architecture'
			choice name: 'TEST_MODE',
				choices: ['code-coverage', 'test'],
				description: 'test mode'
			choice name: 'CI_ENVIRONMENT',
				choices: resolveCiEnvironment(jenkinsfileParams),
				description: 'ci environment'
			booleanParam name: 'SHOULD_PUBLISH_IMAGE', description: 'true to publish image', defaultValue: false
			booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
			booleanParam name: 'SHOULD_RUN_ALL_TEST', description: 'true to run all the test stage', defaultValue: false
		}

		agent {
			node {
				// ARCHITECTURE can be null on first job due to https://issues.jenkins.io/browse/JENKINS-41929
				label """${
					env.OPERATING_SYSTEM = env.OPERATING_SYSTEM ?: "${jenkinsfileParams.operatingSystem[0]}"
					env.ARCHITECTURE = env.ARCHITECTURE ?: 'arm64'
					return helper.resolveAgentName(env.OPERATING_SYSTEM, env.ARCHITECTURE, jenkinsfileParams.instanceSize ?: 'medium')
				}"""
				customWorkspace "${helper.resolveWorkspacePath(env.OPERATING_SYSTEM)}"
			}
		}

		options {
			ansiColor('css')
			timestamps()
			timeout(time: 3, unit: 'HOURS')
		}

		environment {
			DOCKER_CREDENTIALS_ID = 'docker-hub-token-symbolserverbot'
			NPM_CREDENTIALS_ID = 'NPM_TOKEN_ID'
			PYTHON_CREDENTIALS_ID = 'PYPI_TOKEN_ID'
			TEST_PYTHON_CREDENTIALS_ID = 'TEST_PYPI_TOKEN_ID'
			DEV_BRANCH = 'dev'
			RELEASE_BRANCH = 'main'

			LINT_SETUP_SCRIPT_FILEPATH = 'scripts/ci/setup_lint.sh'
			LINT_SCRIPT_FILEPATH = 'scripts/ci/lint.sh'

			BUILD_SETUP_SCRIPT_FILEPATH = 'scripts/ci/setup_build.sh'
			BUILD_SCRIPT_FILEPATH = 'scripts/ci/build.sh'

			TEST_SETUP_SCRIPT_FILEPATH = 'scripts/ci/setup_test.sh'
			TEST_SCRIPT_FILEPATH = 'scripts/ci/test.sh'

			TEST_EXAMPLES_SCRIPT_FILEPATH = 'scripts/ci/test_examples.sh'
			TEST_VECTORS_SCRIPT_FILEPATH = 'scripts/ci/test_vectors.sh'

			GITHUB_PAGES_PUBLISH_SCRIPT_FILEPATH = 'scripts/ci/gh_pages_publish.sh'
		}

		stages {
			stage('setup environment') {
				steps {
					script {
						runScript('git submodule update --remote')
						author = sh(script: 'git log -1 --pretty=format:\'%an\'', returnStdout: true).trim()
					}
				}
			}
			stage('CI pipeline') {
				agent {
					docker {
						image jobHelper.resolveBuildImageName(params.CI_ENVIRONMENT ?: resolveCiEnvironment(jenkinsfileParams)[0])
						args jenkinsfileParams.dockerArgs ?: ''

						// using the same node and the same workspace mounted to the container
						reuseNode true
					}
				}
				stages {
					stage('display environment') {
						steps {
							println("Jenkinsfile parameters: ${jenkinsfileParams}")
							runScript(isUnix() ? 'printenv' : 'set')
						}
					}
					stage('setup docker environment') {
						steps {
							script {
								helper.runInitializeScriptIfPresent()
							}

							runStepRelativeToPackageRoot packageRootPath, {
								configureArtifactRepository(jobHelper.resolveCiEnvironmentName(jenkinsfileParams))
							}
						}
					}
					stage('checkout') {
						when {
							triggeredBy 'UserIdCause'
						}
						steps {
							script {
								runScript("git reset --hard origin/${env.BRANCH_NAME}")
								helper.runInitializeScriptIfPresent()
							}
						}
					}
					stage('verify conventional commit message') {
						when {
							anyOf {
								branch env.DEV_BRANCH
								branch env.RELEASE_BRANCH
							}
						}
						steps {
							script {
								runStepRelativeToPackageRoot '.', {
									final String[] exemptAuthor = ['github-actions[bot]', 'dependabot[bot]']

									println("Last commit author: ${author}")
									if (exemptAuthor.contains(author)) {
										println("Disabling max body line length rule for ${author}")
										env.GITLINT_IGNORE = 'body-max-line-length'
									}

									verifyCommitMessage()
								}
							}
						}
					}
					stage('setup lint') {
						when {
							expression {
								return fileExists(resolvePath(packageRootPath, env.LINT_SETUP_SCRIPT_FILEPATH))
							}
						}
						steps {
							runStepRelativeToPackageRoot packageRootPath, {
								setupBuild(env.LINT_SETUP_SCRIPT_FILEPATH)
							}
						}
					}
					stage('run lint') {
						when { expression { return fileExists(resolvePath(packageRootPath, env.LINT_SCRIPT_FILEPATH)) } }
						steps {
							runStepRelativeToPackageRootWithBadge packageRootPath, "${jenkinsfileParams.packageId}", 'lint', {
								linter(env.LINT_SCRIPT_FILEPATH)
							}
						}
					}
					stage('setup build') {
						when {
							expression {
								return fileExists(resolvePath(packageRootPath, env.BUILD_SETUP_SCRIPT_FILEPATH))
							}
						}
						steps {
							runStepRelativeToPackageRoot packageRootPath, {
								setupBuild(env.BUILD_SETUP_SCRIPT_FILEPATH)
							}
						}
					}
					stage('run build') {
						when {
							expression {
								return fileExists(resolvePath(packageRootPath, env.BUILD_SCRIPT_FILEPATH))
							}
						}
						steps {
							runStepRelativeToPackageRootWithBadge packageRootPath, "${jenkinsfileParams.packageId}", 'build', {
								buildCode(env.BUILD_SCRIPT_FILEPATH)
							}
						}
					}
					stage('setup tests') {
						when {
							expression {
								return fileExists(resolvePath(packageRootPath, env.TEST_SETUP_SCRIPT_FILEPATH))
							}
						}
						steps {
							runStepRelativeToPackageRoot packageRootPath, {
								setupTests(env.TEST_SETUP_SCRIPT_FILEPATH)
							}
						}
					}
					stage('run tests') {
						steps {
							runStepRelativeToPackageRootWithBadge packageRootPath, "${jenkinsfileParams.packageId}", 'test', {
								runTests(env.TEST_SCRIPT_FILEPATH)
							}
						}
					}
					stage('run tests (examples)') {
						when {
							allOf {
								expression {
									return params.SHOULD_RUN_ALL_TEST?.toBoolean()
								}
								expression {
									return fileExists(resolvePath(packageRootPath, env.TEST_EXAMPLES_SCRIPT_FILEPATH))
								}
							}
						}
						steps {
							runStepRelativeToPackageRootWithBadge packageRootPath, "${jenkinsfileParams.packageId}", 'examples', {
								runTests(env.TEST_EXAMPLES_SCRIPT_FILEPATH)
							}
						}
					}
					stage('run tests (vectors)') {
						when {
							expression {
								return fileExists(resolvePath(packageRootPath, env.TEST_VECTORS_SCRIPT_FILEPATH))
							}
						}
						steps {
							runStepRelativeToPackageRootWithBadge packageRootPath, "${jenkinsfileParams.packageId}", 'vectors', {
								runTests(env.TEST_VECTORS_SCRIPT_FILEPATH)
							}
						}
					}
					stage('code coverage') {
						when {
							allOf {
								expression {
									// The branch indexing build TEST_MODE = null
									return null == params.TEST_MODE || 'code-coverage' == params.TEST_MODE
								}
								expression {
									return  null != jenkinsfileParams.codeCoverageTool
								}
								expression {
									// If all the tests are not run then code coverage will fail to meet the required minimum
									// Nightly builds will run all tests.
									return (params.SHOULD_RUN_ALL_TEST?.toBoolean()
										|| !fileExists(resolvePath(packageRootPath, env.TEST_EXAMPLES_SCRIPT_FILEPATH)))
								}
							}
						}
						steps {
							runStepRelativeToPackageRoot packageRootPath, {
								codeCoverage(jenkinsfileParams)
							}
						}
					}
				}
			}
			stage('CD pipeline') {
				stages {
					stage('publish RC') {
						when {
							allOf {
								triggeredBy 'UserIdCause'
								expression {
									return shouldPublishImage(env.SHOULD_PUBLISH_IMAGE)
								}
								not {
									expression {
										return helper.isPublicBuild(env.BUILD_CONFIGURATION)
									}
								}
							}
						}
						steps {
							runStepRelativeToPackageRoot packageRootPath, {
								publish(jenkinsfileParams, 'alpha')
							}
						}
					}
					stage('publish Release') {
						when {
							allOf {
								branch env.RELEASE_BRANCH
								triggeredBy 'UserIdCause'
								expression {
									return shouldPublishImage(env.SHOULD_PUBLISH_IMAGE)
								}
								expression {
									return helper.isPublicBuild(env.BUILD_CONFIGURATION)
								}
							}
						}
						steps {
							runStepRelativeToPackageRoot packageRootPath, {
								publish(jenkinsfileParams, 'release')
							}
						}
					}
				}
			}
		}
		post {
			success {
				echo "Build Success - ${env.JOB_BASE_NAME} - ${env.BUILD_ID} on ${env.BUILD_URL}"
			}
			failure {
				echo "Build Failed - ${env.JOB_BASE_NAME} - ${env.BUILD_ID} on ${env.BUILD_URL}"
			}
			aborted {
				echo " ${env.JOB_BASE_NAME} Build - ${env.BUILD_ID} Aborted!"
			}
			unsuccessful {
				script {
					if (null != env.SHOULD_PUBLISH_FAIL_JOB_STATUS && env.SHOULD_PUBLISH_FAIL_JOB_STATUS.toBoolean()) {
						helper.sendDiscordNotification(
							"Jenkins Job Failed for ${currentBuild.fullDisplayName}",
							"Build#${env.BUILD_NUMBER} has result of ${currentBuild.currentResult} in stage **${env.FAILED_STAGE_NAME}** " +
									"with message: **${env.FAILURE_MESSAGE}**.",
							env.BUILD_URL,
							currentBuild.currentResult
						)
					}
				}
			}
		}
	}
}

void runStepRelativeToPackageRoot(String rootPath, Closure body) {
	try {
		dir(rootPath) {
			body()
		}
		// groovylint-disable-next-line CatchException
	} catch (Exception exception) {
		echo "Caught: ${exception}"
		env.FAILURE_MESSAGE = exception.message ?: exception
		env.FAILED_STAGE_NAME = env.STAGE_NAME
		throw exception
	}
}

void runStepRelativeToPackageRootWithBadge(String rootPath, String packageId, String badgeName, Closure body) {
	EmbeddableBadgeConfig badge = addEmbeddableBadgeConfiguration(id: "${packageId}-${badgeName}", subject: badgeName)
	badge.status = 'running'

	Boolean isSuccess = false
	try {
		runStepRelativeToPackageRoot rootPath, body
		badge.status = 'passing'
		isSuccess = true
	} finally {
		if (!isSuccess) {
			badge.status = 'failing'
		}
	}
}

String resolvePath(String rootPath, String path) {
	return Paths.get(rootPath).resolve(path).toString()
}

Boolean shouldPublishImage(String shouldPublish) {
	return shouldPublish == null ? false : shouldPublish.toBoolean()
}

List<String> resolveCiEnvironment(Map params) {
	String environmentName = jobHelper.resolveCiEnvironmentName(params)
	List<String> environmentTags = params.otherEnvironments?.clone() ?: []

	// default environment is LTS
	environmentTags.add(0, "${environmentName}-${params.operatingSystem[0]}-lts")
	return environmentTags
}
