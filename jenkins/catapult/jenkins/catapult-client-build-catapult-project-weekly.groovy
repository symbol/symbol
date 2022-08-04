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
				stage('gcc (metal) [debian]') {
					steps {
						script {
							dispatch_build_job('gcc-10', 'tests-metal', 'debian')
						}
					}
				}

				stage('gcc (westmere)') {
					steps {
						script {
							dispatch_build_job('gcc-westmere', 'tests-metal', 'ubuntu')
						}
					}
				}

				stage('gcc (metal) [fedora]') {
					steps {
						script {
							dispatch_build_job('gcc-latest', 'tests-metal', 'fedora')
						}
					}
				}

				stage('clang prior (metal)') {
					steps {
						script {
							dispatch_build_job('clang-prior', 'tests-metal', 'ubuntu')
						}
					}
				}

				stage('clang prior (conan)') {
					steps {
						script {
							dispatch_build_job('clang-prior', 'tests-conan', 'ubuntu')
						}
					}
				}

				stage('gcc prior (metal)') {
					steps {
						script {
							dispatch_build_job('gcc-prior', 'tests-metal', 'ubuntu')
						}
					}
				}

				stage('gcc prior (conan)') {
					steps {
						script {
							dispatch_build_job('gcc-prior', 'tests-conan', 'ubuntu')
						}
					}
				}
			}
		}
	}
}

def dispatch_build_job(compiler_configuration, build_configuration, operating_system) {
	build job: 'Symbol/server-pipelines/catapult-client-build-catapult-project', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compiler_configuration}"),
		string(name: 'BUILD_CONFIGURATION', value: "${build_configuration}"),
		string(name: 'OPERATING_SYSTEM', value: "${operating_system}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
	]
}
