pipeline {
    agent {
        label 'ubuntu-20.04-8cores-16Gig'
    }

    options {
        ansiColor('css')
    }

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
        choice name: 'COMPILER_CONFIGURATION',
            choices: ['clang-latest', 'gcc-latest', 'clang-address-undefined', 'clang-thread'],
            description: 'compiler configuration'
        choice name: 'BUILD_CONFIGURATION',
            choices: ['tests-conan', 'tests-metal', 'tests-diagnostics'],
            description: 'build configuration'
    }

    stages {
        stage('build') {
            steps {
                script {
                    sh 'echo "$(id -u):$(id -g)" > user.txt'
                    fully_qualified_user = readFile(file: 'user.txt').trim()
                    destination_image_tag = get_destination_image_tag()

                    run_docker_build_command = """
                        python3 scripts/build/runDockerBuild.py \
                            --compiler-configuration scripts/build/configurations/${COMPILER_CONFIGURATION}.yaml \
                            --build-configuration scripts/build/configurations/${BUILD_CONFIGURATION}.yaml \
                            --user ${fully_qualified_user} \
                            --destination-image-tag ${destination_image_tag} \
                    """

                    sh "${run_docker_build_command} --base-image-names-only > base_image_names.txt"

                    base_image_names = readFile(file: 'base_image_names.txt').split('\n')
                    docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-token-symbolserverbot') {
                        for (base_image_name in base_image_names)
                            docker.image(base_image_name).pull()

                        sh "${run_docker_build_command}"

                        dest_image = docker.image("symbolplatform/symbol-server-test:catapult-server-${destination_image_tag}")
                        dest_image.push()
                    }
                }
            }
        }
    }
}

def get_destination_image_tag() {
    friendly_branch_name = MANUAL_GIT_BRANCH
    if (0 == friendly_branch_name.indexOf('origin/'))
        friendly_branch_name = friendly_branch_name.substring(7)

    friendly_branch_name = friendly_branch_name.replaceAll('/', '-')
    return "${friendly_branch_name}-${env.BUILD_NUMBER}"
}
