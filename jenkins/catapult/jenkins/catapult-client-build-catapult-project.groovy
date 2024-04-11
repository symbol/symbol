pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: "${env.GIT_BRANCH}", name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'COMPILER_CONFIGURATION',
			choices: [
				'gcc-debian',
				'gcc-latest',
				'gcc-prior',
				'gcc-westmere',
				'clang-latest',
				'clang-prior',
				'clang-ausan',
				'clang-tsan',
				'gcc-code-coverage',
				'msvc-latest',
				'msvc-prior'
			],
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
		choice name: 'ARCHITECTURE',
			choices: ['amd64', 'arm64'],
			description: 'platform'

		booleanParam name: 'SHOULD_PUBLISH_BUILD_IMAGE', description: 'true to publish build image', defaultValue: false
		booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
	}

	agent {
		label "${helper.resolveAgentName("${OPERATING_SYSTEM}", "${ARCHITECTURE}", 'xlarge')}"
	}

	environment {
		DOCKER_URL = 'https://registry.hub.docker.com'
		DOCKER_CREDENTIALS_ID = 'docker-hub-token-symbolserverbot'
	}

	options {
		ansiColor('css')
		timestamps()
		timeout(time: 3, unit: 'HOURS')
	}

	stages {
		stage('prepare') {
			stages {
				stage('git checkout') {
					when {
						expression { isManualBuild() }
					}
					steps {
						script {
							helper.runStepAndRecordFailure {
								dir('catapult-src') {
									sh 'git config -l'
									sh "git checkout ${resolveBranchName()}"
									sh "git reset --hard origin/${resolveBranchName()}"
								}
							}
						}
					}
				}
				stage('prepare variables') {
					steps {
						script {
							helper.runStepAndRecordFailure {
								fullyQualifiedUser = sh(
									script: 'echo "$(id -u):$(id -g)"',
									returnStdout: true
								).trim()

								buildImageLabel = TEST_IMAGE_LABEL?.trim() ? TEST_IMAGE_LABEL : resolveBuildImageLabel()
								buildImageFullName = "symbolplatform/symbol-server-test:${buildImageLabel}"

								compilerConfiguratonFilePath = "catapult-src/jenkins/catapult/configurations/${ARCHITECTURE}/${COMPILER_CONFIGURATION}.yaml"
								buildConfigurationFilePath = "catapult-src/jenkins/catapult/configurations/${BUILD_CONFIGURATION}.yaml"
							}
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
						  			  ARCHITECTURE: ${ARCHITECTURE}

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
									--compiler-configuration ${compilerConfiguratonFilePath} \
									--build-configuration ${buildConfigurationFilePath} \
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
							helper.runStepAndRecordFailure {
								baseImageNames = sh(
									script: "${runDockerBuildCommand} --base-image-names-only",
									returnStdout: true
								).split('\n')

								docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
									for (baseImageName in baseImageNames) {
										docker.image(baseImageName.trim()).pull()
									}
								}
							}
						}
					}
				}
				stage('lint') {
					steps {
						script {
							helper.runStepAndRecordFailure {
								sh """
									python3 catapult-src/jenkins/catapult/runDockerTests.py \
										--image registry.hub.docker.com/symbolplatform/symbol-server-test-base:${OPERATING_SYSTEM} \
										--compiler-configuration ${compilerConfiguratonFilePath} \
										--user ${fullyQualifiedUser} \
										--mode lint \
										--source-path catapult-src \
										--linter-path catapult-src/linters
								"""
							}
						}
					}
				}
				stage('build') {
					steps {
						script {
							helper.runStepAndRecordFailure {
								sh "${runDockerBuildCommand}"
							}
						}
					}
				}
				stage('push built image') {
					when {
						expression { SHOULD_PUBLISH_BUILD_IMAGE.toBoolean() }
					}
					steps {
						script {
							helper.runStepAndRecordFailure {
								docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
									docker.image(buildImageFullName).push()
								}
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
							helper.runStepAndRecordFailure {
								docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
									docker.image(buildImageFullName).pull()
								}
							}
						}
					}
				}
				stage('run tests') {
					steps {
						script {
							helper.runStepAndRecordFailure {
								testImageName = isCustomTestImage()
										? "registry.hub.docker.com/symbolplatform/symbol-server-test:${buildImageLabel}"
										: "symbolplatform/symbol-server-test:${buildImageLabel}"

								sh """
									python3 catapult-src/jenkins/catapult/runDockerTests.py \
										--image ${testImageName} \
										--compiler-configuration ${compilerConfiguratonFilePath} \
										--user ${fullyQualifiedUser} \
										--mode ${TEST_MODE} \
										--verbosity ${TEST_VERBOSITY} \
										--source-path catapult-src
								"""
							}
						}
					}
				}
				stage('code coverage') {
					when {
						expression { isCodeCoverageBuild() }
					}
					steps {
						script {
							helper.runStepAndRecordFailure {
								baseImageNames = sh(
									script: "${runDockerBuildCommand} --base-image-names-only",
									returnStdout: true
								).split('\n')

								docker.image(baseImageNames[0]).inside("--volume=${pwd()}/catapult-src:/catapult-src") {
									sh '''
										cd /catapult-src
										lcov --directory client/catapult/_build --capture --output-file coverage_all.info --ignore-errors mismatch
										lcov --remove coverage_all.info '/usr/*' '/mybuild/*' '/*tests/*' '/*external/*' --output-file client_coverage.info
										lcov --list client_coverage.info
									'''

									withCredentials([string(credentialsId: 'SYMBOL_CODECOV_ID', variable: 'CODECOV_TOKEN')]) {
										String platform = 'arm64' == params.ARCHITECTURE ? 'aarch64' : 'linux'
										sh """
											cd /catapult-src
											curl -Os https://uploader.codecov.io/latest/${platform}/codecov
											chmod +x codecov
											./codecov --verbose --nonZero --rootDir . --flags client-catapult --file client_coverage.info
										"""
									}
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
		unsuccessful {
			script {
				if (env.SHOULD_PUBLISH_FAIL_JOB_STATUS?.toBoolean()) {
					helper.sendDiscordNotification(
						"Catapult Client Job Failed for ${currentBuild.fullDisplayName}",
						"Job configuration ${COMPILER_CONFIGURATION} with ${BUILD_CONFIGURATION} on ${OPERATING_SYSTEM} has result of"
						+ " ${currentBuild.currentResult} in stage **${env.FAILED_STAGE_NAME}** with message: **${env.FAILURE_MESSAGE}**.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
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
	return null != env.MANUAL_GIT_BRANCH && '' != env.MANUAL_GIT_BRANCH && 'null' != env.MANUAL_GIT_BRANCH
}

Boolean isCustomTestImage() {
	return '' != env.TEST_IMAGE_LABEL
}

String resolveBranchName() {
	return isManualBuild() ? env.MANUAL_GIT_BRANCH : env.GIT_BRANCH
}

String resolveBuildImageLabel() {
	friendlyBranchName = resolveBranchName()
	if (0 == friendlyBranchName.indexOf('origin/')) {
		friendlyBranchName = friendlyBranchName.substring(7)
	}

	friendlyBranchName = friendlyBranchName.replaceAll('/', '-')
	return "catapult-client-${friendlyBranchName}-${env.BUILD_NUMBER}"
}

Boolean isCodeCoverageBuild() {
	return 'gcc-code-coverage' == env.COMPILER_CONFIGURATION
}
