pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: "${env.GIT_BRANCH}", name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'COMPILER_CONFIGURATION',
			choices: ['gcc-latest', 'gcc-prior', 'gcc-westmere', 'clang-latest', 'clang-prior', 'clang-ausan', 'clang-tsan', 'gcc-code-coverage', 'msvc-latest', 'msvc-prior'],
			description: 'compiler configuration'
		choice name: 'BUILD_CONFIGURATION',
			choices: ['tests-metal', 'tests-conan', 'tests-diagnostics', 'none'],
			description: 'build configuration'
		choice name: 'OPERATING_SYSTEM',
			choices: ['ubuntu', 'fedora', 'debian', 'windows'],
			description: 'operating system'

		string name: 'TEST_IMAGE_LABEL', description: 'docker test image label', defaultValue: ''
		choice name: 'TEST_MODE',
			choices: ['test', 'bench', 'none'],
			description: 'test mode'
		choice name: 'TEST_VERBOSITY',
			choices: ['suite', 'test', 'max'],
			description: 'output verbosity level'

		booleanParam name: 'SHOULD_PUBLISH_BUILD_IMAGE', description: 'true to publish build image', defaultValue: false
	}

	agent {
		label "${helper.resolveAgentName("${OPERATING_SYSTEM}")}"
	}

	environment {
		DOCKER_URL = 'https://registry.hub.docker.com'
		DOCKER_CREDENTIALS_ID = 'docker-hub-token-symbolserverbot'
	}

	options {
		ansiColor('css')
		timestamps()
	}

	stages {
		stage('prepare') {
			stages {
				stage('prepare variables') {
					steps {
						script {
							fullyQualifiedUser = sh(
								script: 'echo "$(id -u):$(id -g)"',
								returnStdout: true
							).trim()

							buildImageLabel = '' != TEST_IMAGE_LABEL ? TEST_IMAGE_LABEL : getBuildImageLabel()
							buildImageFullName = "symbolplatform/symbol-server-test:${buildImageLabel}"
						}
					}
				}
				stage('print env') {
					steps {
						echo """
									env.GIT_BRANCH: ${env.GIT_BRANCH}
								 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}

							COMPILER_CONFIGURATION: ${COMPILER_CONFIGURATION}
							   BUILD_CONFIGURATION: ${BUILD_CONFIGURATION}
								  OPERATING_SYSTEM: ${OPERATING_SYSTEM}

								  TEST_IMAGE_LABEL: ${TEST_IMAGE_LABEL}
										 TEST_MODE: ${TEST_MODE}
									TEST_VERBOSITY: ${TEST_VERBOSITY}

						SHOULD_PUBLISH_BUILD_IMAGE: ${SHOULD_PUBLISH_BUILD_IMAGE}

								fullyQualifiedUser: ${fullyQualifiedUser}
								   buildImageLabel: ${buildImageLabel}
								buildImageFullName: ${buildImageFullName}
						"""
					}
				}
				stage('git checkout') {
					when {
						expression { isManualBuild() }
					}
					steps {
						cleanWs()
						dir('catapult-src') {
							sh 'git config -l'
							git branch: "${getBranchName()}",
									url: 'https://github.com/symbol/symbol.git'
						}
					}
				}
			}
		}
		stage('build') {
			when {
				expression { isBuildEnabled() }
			}
			stages {
				stage('prepare variables') {
					steps {
						script {
							runDockerBuildCommand = """
								python3 catapult-src/jenkins/catapult/runDockerBuild.py \
									--compiler-configuration catapult-src/jenkins/catapult/configurations/${COMPILER_CONFIGURATION}.yaml \
									--build-configuration catapult-src/jenkins/catapult/configurations/${BUILD_CONFIGURATION}.yaml \
									--operating-system ${OPERATING_SYSTEM} \
									--user ${fullyQualifiedUser} \
									--destination-image-label ${buildImageLabel} \
									--source-path catapult-src \
							"""
						}
					}
				}
				stage('pull dependency images') {
					steps {
						script {
							baseImageNames = sh(
								script: "${runDockerBuildCommand} --base-image-names-only",
								returnStdout: true
							).split('\n')

							docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
								for (baseImageName in baseImageNames)
									docker.image(baseImageName.trim()).pull()
							}
						}
					}
				}
				stage('lint') {
					steps {
						sh """
							python3 catapult-src/jenkins/catapult/runDockerTests.py \
								--image registry.hub.docker.com/symbolplatform/symbol-server-test-base:${OPERATING_SYSTEM} \
								--compiler-configuration catapult-src/jenkins/catapult/configurations/${COMPILER_CONFIGURATION}.yaml \
								--user ${fullyQualifiedUser} \
								--mode lint \
								--source-path catapult-src \
								--linter-path catapult-src/linters
						"""
					}
				}
				stage('build') {
					steps {
						sh "${runDockerBuildCommand}"
					}
				}
				stage('push built image') {
					when {
						expression { SHOULD_PUBLISH_BUILD_IMAGE.toBoolean() }
					}
					steps {
						script {
							docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
								docker.image(buildImageFullName).push()
							}
						}
					}
				}
			}
			post {
				always {
					recordIssues enabledForFailure: true, tool: pyLint(pattern: 'catapult-data/logs/pylint.log')
					recordIssues enabledForFailure: true, tool: pep8(pattern: 'catapult-data/logs/pycodestyle.log')
					recordIssues enabledForFailure: true, tool: gcc(pattern: 'catapult-data/logs/isort.log', name: 'isort', id: 'isort')

					recordIssues enabledForFailure: true,
						tool: gcc(pattern: 'catapult-data/logs/shellcheck.log', name: 'shellcheck', id: 'shellcheck')
				}
			}
		}
		stage('test') {
			when {
				expression { isTestEnabled() }
			}
			stages {
				stage('pull dependency images') {
					when {
						expression { isCustomTestImage() }
					}
					steps {
						script {
							docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
								docker.image(buildImageFullName).pull()
							}
						}
					}
				}
				stage('run tests') {
					steps {
						script {
							if (isCustomTestImage())
								testImageName = "registry.hub.docker.com/symbolplatform/symbol-server-test:${buildImageLabel}"
							else
								testImageName = "symbolplatform/symbol-server-test:${buildImageLabel}"

							sh """
								python3 catapult-src/jenkins/catapult/runDockerTests.py \
									--image ${testImageName} \
									--compiler-configuration catapult-src/jenkins/catapult/configurations/${COMPILER_CONFIGURATION}.yaml \
									--user ${fullyQualifiedUser} \
									--mode ${TEST_MODE} \
									--verbosity ${TEST_VERBOSITY} \
									--source-path catapult-src
							"""
						}
					}
				}
				stage('code coverage') {
					when {
						expression { isCodeCoverageBuild() }
					}
					steps {
						script {
							dir('catapult-src') {
								sh """
									sudo ln -s "${pwd()}" /catapult-src
									lcov --directory client/catapult/_build --capture --output-file coverage_all.info
									lcov --remove coverage_all.info '/usr/*' '/mybuild/*' '/*tests/*' '/*external/*' --output-file client_coverage.info 
									lcov --list client_coverage.info
								"""

								withCredentials([string(credentialsId: 'SYMBOL_CODECOV_ID', variable: 'CODECOV_TOKEN')]) {
									sh """
										curl -Os https://uploader.codecov.io/latest/linux/codecov
										chmod +x codecov
										./codecov --required --root . --flags client-catapult -X gcov --file client_coverage.info
									"""
								}
							}
						}
					}
				}
			}
		}
	}
	post {
		always {
			junit '**/catapult-data/logs/*.xml'

			dir('catapult-data') {
				deleteDir()
			}
			dir('mongo') {
				deleteDir()
			}
		}
	}
}

Boolean isBuildEnabled() {
	return 'none' != BUILD_CONFIGURATION
}

Boolean isTestEnabled() {
	return 'none' != TEST_MODE
}

Boolean isManualBuild() {
	return null != MANUAL_GIT_BRANCH && '' != MANUAL_GIT_BRANCH && 'null' != MANUAL_GIT_BRANCH
}

Boolean isCustomTestImage() {
	return '' != TEST_IMAGE_LABEL
}

String getBranchName() {
	return isManualBuild() ? MANUAL_GIT_BRANCH : env.GIT_BRANCH
}

String getBuildImageLabel() {
	friendlyBranchName = getBranchName()
	if (0 == friendlyBranchName.indexOf('origin/'))
		friendlyBranchName = friendlyBranchName.substring(7)

	friendlyBranchName = friendlyBranchName.replaceAll('/', '-')
	return "catapult-client-${friendlyBranchName}-${env.BUILD_NUMBER}"
}

def isCodeCoverageBuild() {
	return 'gcc-code-coverage' == COMPILER_CONFIGURATION
}
