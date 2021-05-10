pipeline {
    agent any

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
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
                stage('gcc-latest (conan)') {
                    steps {
                        script {
                            dispatch_build_job('gcc-latest', 'tests-conan')
                        }
                    }
                }
                stage('clang-latest (conan)') {
                    steps {
                        script {
                            dispatch_build_job('clang-latest', 'tests-conan')
                        }
                    }
                }

                stage('gcc-latest (metal)') {
                    steps {
                        script {
                            dispatch_build_job('gcc-latest', 'tests-metal')
                        }
                    }
                }
                stage('clang-latest (metal)') {
                    steps {
                        script {
                            dispatch_build_job('clang-latest', 'tests-metal')
                        }
                    }
                }

                stage('clang-address-undefined') {
                    steps {
                        script {
                            dispatch_build_job('clang-address-undefined', 'tests-metal')
                        }
                    }
                }
                stage('clang-thread') {
                    steps {
                        script {
                            dispatch_build_job('clang-thread', 'tests-metal')
                        }
                    }
                }
                stage('clang-diagnostics') {
                    steps {
                        script {
                            dispatch_build_job('clang-latest', 'tests-diagnostics')
                        }
                    }
                }
            }
        }
    }
}

def dispatch_build_job(compiler_configuration, build_configuration) {
    build job: 'server-pipelines/catapult-server-build-catapult-project', parameters: [
        string(name: 'COMPILER_CONFIGURATION', value: "${compiler_configuration}"),
        string(name: 'BUILD_CONFIGURATION', value: "${build_configuration}"),
        string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
    ]
}
