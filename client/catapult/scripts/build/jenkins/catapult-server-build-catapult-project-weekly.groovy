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
                stage('gcc 10 (conan)') {
                    steps {
                        script {
                            dispatch_build_job('gcc-10', 'tests-conan', 'ubuntu')
                        }
                    }
                }
                stage('gcc 10 (metal)') {
                    steps {
                        script {
                            dispatch_build_job('gcc-10', 'tests-metal', 'ubuntu')
                        }
                    }
                }
                stage('gcc 11 (metal) [fedora]') {
                    steps {
                        script {
                            dispatch_build_job('gcc-11', 'tests-metal', 'fedora')
                        }
                    }
                }

                stage('clang 11 (metal)') {
                    steps {
                        script {
                            dispatch_build_job('clang-11', 'tests-metal', 'ubuntu')
                        }
                    }
                }
                stage('clang 12 (conan)') {
                    steps {
                        script {
                            dispatch_build_job('clang-12', 'tests-conan', 'ubuntu')
                        }
                    }
                }
                stage('clang 12 (metal)') {
                    steps {
                        script {
                            dispatch_build_job('clang-12', 'tests-metal', 'ubuntu')
                        }
                    }
                }

                stage('clang ausan') {
                    steps {
                        script {
                            dispatch_build_job('clang-ausan', 'tests-metal', 'ubuntu')
                        }
                    }
                }
                stage('clang tsan') {
                    steps {
                        script {
                            dispatch_build_job('clang-tsan', 'tests-metal', 'ubuntu')
                        }
                    }
                }
                stage('clang diagnostics') {
                    steps {
                        script {
                            dispatch_build_job('clang-12', 'tests-diagnostics', 'ubuntu')
                        }
                    }
                }
            }
        }
    }
}

def dispatch_build_job(compiler_configuration, build_configuration, operating_system) {
    build job: 'server-pipelines/catapult-server-build-catapult-project', parameters: [
        string(name: 'COMPILER_CONFIGURATION', value: "${compiler_configuration}"),
        string(name: 'BUILD_CONFIGURATION', value: "${build_configuration}"),
        string(name: 'OPERATING_SYSTEM', value: "${operating_system}"),
        string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}")
    ]
}
