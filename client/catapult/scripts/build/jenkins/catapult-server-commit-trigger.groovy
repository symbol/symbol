pipeline {
    agent any

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
    }

    options {
        ansiColor('css')
    }

    stages {
        stage('print env') {
            steps {
                script {
                    echo "MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}"
                }
            }
        }
        stage('build') {
            steps {
                script {
                    git_branch = env.GIT_BRANCH ?: MANUAL_GIT_BRANCH
                    build_job = build job: 'server-pipelines/catapult-server-build-catapult-project', parameters: [
                        string(name: 'COMPILER_CONFIGURATION', value: 'gcc-latest'),
                        string(name: 'BUILD_CONFIGURATION', value: 'tests-metal'),
                        gitParameter(name: 'MANUAL_GIT_BRANCH', value: "${git_branch}")
                    ]

                    echo "BUILD JOB ${build_job.getNumber()} COMPLETED [${build_job.getTimeInMillis()} ms] ${build_job.getResult()}"
                }
            }
        }
        stage('test') {
            steps {
                script {
                    build_image_name = "catapult-server-${normalize_git_branch_name(git_branch)}-${build_job.getNumber()}"
                    test_job = build job: 'server-pipelines/catapult-server-run-tests', parameters: [
                        string(name: 'TEST_IMAGE_NAME', value: "${build_image_name}"),
                        string(name: 'TEST_MODE', value: 'test'),
                        string(name: 'VERBOSITY', value: 'test'),
                        gitParameter(name: 'MANUAL_GIT_BRANCH', value: "${git_branch}")
                    ]

                    echo "BUILD JOB ${test_job.getNumber()} COMPLETED [${test_job.getTimeInMillis()} ms] ${test_job.getResult()}"
                }
            }
        }
    }
}

def normalize_git_branch_name(git_branch) {
    friendly_branch_name = git_branch
    if (0 == friendly_branch_name.indexOf('origin/'))
        friendly_branch_name = friendly_branch_name.substring(7)

    return friendly_branch_name.replaceAll('/', '-')
}
