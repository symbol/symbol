import java.nio.file.Paths

def call(body) {
    def params= [:]
    body.resolveStrategy = Closure.DELEGATE_FIRST
    body.delegate = params
    body()

    String package_root_path = get_jenkinsfile_path()

    pipeline {
        parameters {
            gitParameter branchFilter: 'origin/(.*)',
                    defaultValue: "${env.GIT_BRANCH}",
                    name: 'MANUAL_GIT_BRANCH',
                    type: 'PT_BRANCH'
            choice name: 'PLATFORM',
                    choices: params.platform ?: 'ubuntu',
                    description: 'Run on specific platform'
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
            GITHUB_CREDENTIALS_ID = 'Symbol-Github-app'
            DOCKERHUB_CREDENTIALS_ID = 'docker-hub-token-symbolserverbot'
            NPM_CREDENTIALS_ID = 'NPM_TOKEN_ID'
            PYTHON_CREDENTIALS_ID = '' //TODO: need an account
            DEV_BRANCH = 'dev'
            RELEASE_BRANCH = 'main'
            BUILD_BRANCH = 'main'
            BUILD_SETUP_SCRIPT_FILEPATH = 'scripts/ci/setup_build.sh'
            BUILD_SCRIPT_FILEPATH = 'scripts/ci/build.sh'
            TEST_SETUP_SCRIPT_FILEPATH = 'scripts/ci/setup_tests.sh'
            TEST_SCRIPT_FILEPATH = 'scripts/ci/test.sh'
            LINTER_SCRIPT_FILEPATH = 'scripts/ci/linter.sh'
        }

        stages {
            stage('CI pipeline') {
                stages {
                    stage("Display environment") {
                        steps {
                            println("Parameters: ${params}")
                            sh 'printenv'
                        }
                    }
                    stage("checkout") {
                        when {
                            expression { helper.is_manual_build() }
                        }
                        steps {
                            git_checkout(env.BUILD_BRANCH, env.GITHUB_CREDENTIALS_ID, env.GIT_URL)
                        }
                    }
                    stage("verify conventional commit message") {
                        steps {
                            verify_commit_message()
                        }
                    }
                    stage("lint") {
                        when { expression { return fileExists (resolve_path(package_root_path, env.LINTER_SCRIPT_FILEPATH)) } }
                        steps {
                            run_step_relative_to_package_root package_root_path, {
                                linter(env.LINTER_SCRIPT_FILEPATH)
                            }
                        }
                    }
                    stage("setup build") {
                        when { expression { return fileExists (resolve_path(package_root_path, env.BUILD_SETUP_SCRIPT_FILEPATH)) } }
                        steps {
                            run_step_relative_to_package_root package_root_path, {
                                setup_build(env.BUILD_SETUP_SCRIPT_FILEPATH)
                            }
                        }
                    }
                    stage("build code") {
                        steps {
                            run_step_relative_to_package_root package_root_path, {
                                build_code(env.BUILD_SCRIPT_FILEPATH)
                            }
                        }
                    }
                    stage("setup tests") {
                        when { expression { return fileExists (resolve_path(package_root_path, env.TEST_SETUP_SCRIPT_FILEPATH)) } }
                        steps {
                            run_step_relative_to_package_root package_root_path, {
                                setup_tests(env.TEST_SETUP_SCRIPT_FILEPATH)
                            }
                        }
                    }
                    stage("run tests") {
                        steps {
                            run_step_relative_to_package_root package_root_path, {
                                run_tests(env.TEST_SCRIPT_FILEPATH)
                            }
                        }
                    }
                }
            }
            stage('CD pipeline') {
                stages {
                    stage("publish RC") {
                        when {
                            anyOf {
                                branch pattern: env.DEV_BRANCH, comparator: "EQUALS"
                                expression { env.SHOULD_PUBLISH_IMAGE == 'true' }
                            }
                        }
                        steps {
                            run_step_relative_to_package_root package_root_path, {
                                publish(params, 'alpha')
                            }
                        }
                    }
                }
            }
        }
        post {
            //TODO: add notification
            success {
                echo "Build Success"
                echo "Successfully built ${env.JOB_BASE_NAME} - ${env.BUILD_ID} on ${env.BUILD_URL}"
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

def run_step_relative_to_package_root(String root_path, Closure body) {
    dir(root_path){
        body()
    }
}

def resolve_path(String root_path, String path) {
    Paths.get(root_path).resolve(path).toString()
}