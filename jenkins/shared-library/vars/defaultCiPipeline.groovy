/* groovylint-disable NestedBlockDepth */
import java.nio.file.Paths
import org.jenkinsci.plugins.badge.EmbeddableBadgeConfig

// groovylint-disable-next-line MethodSize
void call(Closure body) {
	Map params = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = params
	body()

	String packageRootPath = findJenkinsfilePath()

	pipeline {
		parameters {
			gitParameter branchFilter: 'origin/(.*)',
				defaultValue: "${env.GIT_BRANCH}",
				name: 'MANUAL_GIT_BRANCH',
				type: 'PT_BRANCH'
			choice name: 'PLATFORM',
				choices: params.platform ?: 'ubuntu',
				description: 'Run on specific platform'
			choice name: 'BUILD_CONFIGURATION',
				choices: ['release-private', 'release-public'],
				description: 'build configuration'
			booleanParam name: 'SHOULD_PUBLISH_IMAGE', description: 'true to publish image', defaultValue: false
		}

		agent {
			label "${PLATFORM}-agent"
		}

		options {
			ansiColor('css')
			timestamps()
		}

		environment {
			GITHUB_CREDENTIALS_ID = "${params.gitHubId}"
			DOCKERHUB_CREDENTIALS_ID = 'docker-hub-token-symbolserverbot'
			NPM_CREDENTIALS_ID = 'NPM_TOKEN_ID'
			PYTHON_CREDENTIALS_ID = 'PYPI_TOKEN_ID'
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
		}

		stages {
			stage('CI pipeline') {
				stages {
					stage('display environment') {
						steps {
							println("Parameters: ${params}")
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
							verifyCommitMessage()
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
							runStepRelativeToPackageRootWithBadge packageRootPath, "${params.packageId}", 'lint', {
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
							runStepRelativeToPackageRootWithBadge packageRootPath, "${params.packageId}", 'build', {
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
							runStepRelativeToPackageRootWithBadge packageRootPath, "${params.packageId}", 'test', {
								runTests(env.TEST_SCRIPT_FILEPATH)
							}
						}
					}
					stage('run tests (examples)') {
						when {
							expression {
								return fileExists(resolvePath(packageRootPath, env.TEST_EXAMPLES_SCRIPT_FILEPATH))
							}
						}
						steps {
							runStepRelativeToPackageRootWithBadge packageRootPath, "${params.packageId}", 'examples', {
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
							runStepRelativeToPackageRootWithBadge packageRootPath, "${params.packageId}", 'vectors', {
								runTests(env.TEST_VECTORS_SCRIPT_FILEPATH)
							}
						}
					}
				}
			}
			stage('CD pipeline') {
				stages {
					stage('publish RC') {
						when {
							anyOf {
								branch env.DEV_BRANCH
								allOf {
									expression {
										return env.SHOULD_PUBLISH_IMAGE.toBoolean()
									}
									not {
										expression {
											return helper.isPublicBuild(env.BUILD_CONFIGURATION)
										}
									}
								}
							}
						}
						steps {
							runStepRelativeToPackageRoot packageRootPath, {
								publish(params, 'alpha')
							}
						}
					}
					stage('publish Release') {
						when {
							allOf {
								branch env.RELEASE_BRANCH
								expression {
									return env.SHOULD_PUBLISH_IMAGE.toBoolean()
								}
								expression {
									return helper.isPublicBuild(env.BUILD_CONFIGURATION)
								}
							}
						}
						steps {
							runStepRelativeToPackageRoot packageRootPath, {
								publish(params, 'release')
							}
						}
					}
				}
			}
		}
		post {
			//TODO: add notification
			success {
				echo "Build Success - ${env.JOB_BASE_NAME} - ${env.BUILD_ID} on ${env.BUILD_URL}"
			}
			failure {
				echo "Build Failed - ${env.JOB_BASE_NAME} - ${env.BUILD_ID} on ${env.BUILD_URL}"
			}
			aborted {
				echo " ${env.JOB_BASE_NAME} Build - ${env.BUILD_ID} Aborted!"
			}
		}
	}
}

void runStepRelativeToPackageRoot(String rootPath, Closure body) {
	dir(rootPath) {
		body()
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
