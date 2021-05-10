pipeline {
    agent {
        label 'ubuntu-20.04-8cores-16Gig'
    }

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: "${env.GIT_BRANCH}", name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
        choice name: 'COMPILER_CONFIGURATION',
            choices: ['gcc-latest', 'clang-latest', 'clang-address-undefined', 'clang-thread'],
            description: 'compiler configuration'
        choice name: 'BUILD_CONFIGURATION',
            choices: ['tests-metal', 'tests-conan', 'tests-diagnostics', 'none'],
            description: 'build configuration'

        string name: 'TEST_IMAGE_LABEL', description: 'docker test image label', defaultValue: ''
        choice name: 'TEST_MODE',
            choices: ['test', 'bench', 'none'],
            description: 'test mode'
        choice name: 'TEST_VERBOSITY',
            choices: ['suite', 'test', 'max'],
            description: 'output verbosity level'

        booleanParam name: 'SHOULD_PUBLISH_BUILD_IMAGE', description: 'true to publish build image', defaultValue: false
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
                            fully_qualified_user = sh(
                                script: 'echo "$(id -u):$(id -g)"',
                                returnStdout: true
                            ).trim()

                            build_image_label = get_build_image_label()
                            build_image_full_name = "symbolplatform/symbol-server-test:${build_image_label}"
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

                                  TEST_IMAGE_LABEL: ${TEST_IMAGE_LABEL}
                                         TEST_MODE: ${TEST_MODE}
                                    TEST_VERBOSITY: ${TEST_VERBOSITY}

                        SHOULD_PUBLISH_BUILD_IMAGE: ${SHOULD_PUBLISH_BUILD_IMAGE}

                              fully_qualified_user: ${fully_qualified_user}
                                 build_image_label: ${build_image_label}
                             build_image_full_name: ${build_image_full_name}
                        """
                    }
                }
            }
        }
        stage('build') {
            when {
                expression { 'none' != BUILD_CONFIGURATION }
            }
            stages {
                stage('prepare variables') {
                    steps {
                        script {
                            run_docker_build_command = """
                                python3 catapult-src/scripts/build/runDockerBuild.py \
                                    --compiler-configuration catapult-src/scripts/build/configurations/${COMPILER_CONFIGURATION}.yaml \
                                    --build-configuration catapult-src/scripts/build/configurations/${BUILD_CONFIGURATION}.yaml \
                                    --user ${fully_qualified_user} \
                                    --destination-image-label ${build_image_label} \
                            """
                        }
                    }
                }
                stage('pull dependency images') {
                    steps {
                        script {
                            base_image_names = sh(
                                script: "${run_docker_build_command} --base-image-names-only",
                                returnStdout: true
                            ).split('\n')

                            docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
                                for (base_image_name in base_image_names)
                                    docker.image(base_image_name).pull()
                            }
                        }
                    }
                }
                stage('build') {
                    steps {
                        sh "${run_docker_build_command}"
                    }
                }
                stage('push built image') {
                    when {
                        expression { SHOULD_PUBLISH_BUILD_IMAGE.toBoolean() }
                    }
                    steps {
                        script {
                            docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
                                docker.image(build_image_full_name).push()
                            }
                        }
                    }
                }
            }
        }
        stage('test') {
            when {
                expression { 'none' != TEST_MODE }
            }
            stages {
                stage('pull dependency images') {
                    when {
                        expression { '' != TEST_IMAGE_LABEL }
                    }
                    steps {
                        script {
                            docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
                                docker.image(build_image_full_name).pull()
                            }
                        }
                    }
                }
                stage('run tests') {
                    steps {
                        script {
                            if ('' != TEST_IMAGE_LABEL)
                                test_image_name = "registry.hub.docker.com/symbolplatform/symbol-server-test:${build_image_label}"
                            else
                                test_image_name = "symbolplatform/symbol-server-test:${build_image_label}"

                            sh """
                                python3 catapult-src/scripts/build/runDockerTests.py \
                                    --image ${test_image_name} \
                                    --user ${fully_qualified_user} \
                                    --mode ${TEST_MODE} \
                                    --verbosity ${TEST_VERBOSITY}
                            """
                        }
                    }
                }
            }
        }
    }
}

def get_build_image_label() {
    friendly_branch_name = 'null' != MANUAL_GIT_BRANCH ? MANUAL_GIT_BRANCH : env.GIT_BRANCH
    if (0 == friendly_branch_name.indexOf('origin/'))
        friendly_branch_name = friendly_branch_name.substring(7)

    friendly_branch_name = friendly_branch_name.replaceAll('/', '-')
    return "catapult-server-${friendly_branch_name}-${env.BUILD_NUMBER}"
}
