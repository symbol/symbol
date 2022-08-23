pipeline {
	agent any

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
	}

	options {
		ansiColor('css')
		timestamps()
	}

	stages {
		stage('print env') {
			steps {
				echo """
							env.GIT_BRANCH: ${env.GIT_BRANCH}
						 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}
				"""
			}
		}

		stage('build servers') {
			parallel {
				stage('gcc latest (conan)') {
					steps {
						script {
							dispatchUbuntuBuildJob('gcc-latest', 'tests-conan')
						}
					}
				}
				stage('gcc latest (metal)') {
					steps {
						script {
							dispatchUbuntuBuildJob('gcc-latest', 'tests-metal')
						}
					}
				}

				stage('clang latest (conan)') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-latest', 'tests-conan')
						}
					}
				}
				stage('clang latest (metal)') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-latest', 'tests-metal')
						}
					}
				}

				stage('clang ausan') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-ausan', 'tests-metal')
						}
					}
				}
				stage('clang tsan') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-tsan', 'tests-metal')
						}
					}
				}
				stage('clang diagnostics') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-latest', 'tests-diagnostics')
						}
					}
				}
				stage('code coverage (gcc latest)') {
					steps {
						script {
							dispatchUbuntuBuildJob('gcc-code-coverage', 'tests-metal')
						}
					}
				}
				stage('msvc latest (conan)') {
					steps {
						script {
							dispatchBuildJob('msvc-latest', 'tests-conan', 'windows')
						}
					}
				}
			}
		}
	}
}

def dispatchBuildJob(String compilerConfiguration, String buildConfiguration, String operatingSystem) {
	build job: 'Symbol/server-pipelines/catapult-client-build-catapult-project', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compilerConfiguration}"),
		string(name: 'BUILD_CONFIGURATION', value: "${buildConfiguration}"),
		string(name: 'OPERATING_SYSTEM', value: "${operatingSystem}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
	]
}

def dispatchUbuntuBuildJob(String compilerConfiguration, String buildConfiguration) {
	dispatchBuildJob(compilerConfiguration, buildConfiguration, 'ubuntu')
}
