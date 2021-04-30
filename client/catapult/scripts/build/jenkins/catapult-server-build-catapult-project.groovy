pipeline {
    agent {
        label 'cat-server-01'
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

                    sh """
                        python3 scripts/build/runDockerBuild.py \
                            --compiler-configuration scripts/build/configurations/${COMPILER_CONFIGURATION}.yaml \
                            --build-configuration scripts/build/configurations/${BUILD_CONFIGURATION}.yaml \
                            --user ${fully_qualified_user} \
                            --destination-image-tag ${destination_image_tag}
                    """
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
