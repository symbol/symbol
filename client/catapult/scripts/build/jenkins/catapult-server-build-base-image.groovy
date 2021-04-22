pipeline {
    agent {
        label 'cat-server-01'
    }

    parameters {
        gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
        choice name: 'COMPILER_CONFIGURATION',
            choices: ['clang-latest', 'gcc-latest', 'clang-address-undefined', 'clang-thread'],
            description: 'compiler configuration'
    }

    stages {
        stage('prepare base image') {
            steps {
                script {
                    dest_image_name = "symbolplatform/symbol-server-build-base:${COMPILER_CONFIGURATION}"

                    basic_python_command = """
                        python3 ./scripts/build/baseImageDockerfileGenerator.py \
                            --compiler-configuration scripts/build/configurations/${COMPILER_CONFIGURATION}.yaml \
                            --versions ./scripts/build/versions.properties \
                    """

                    for (layer in ['os', 'boost', 'deps', 'test'])
                        base_image = base_image_build_layer("${layer}", "${basic_python_command}")

                    base_image.push()

                    // create conan base images
                    if ("${COMPILER_CONFIGURATION}" == "clang-latest" || "${COMPILER_CONFIGURATION}" == "gcc-latest") {
                        base_image = base_image_build_layer('conan', "${basic_python_command}")
                        base_image.push()
                    }
                }
            }
        }
    }
}

def base_image_build_layer(layer, basic_python_command) {
    sh """
        ${basic_python_command} --layer ${layer} > Dockerfile
        ${basic_python_command} --layer ${layer} --name-only > dest_image_name.txt

        echo "*** LAYER ${layer} ***"
        cat Dockerfile
        cat dest_image_name.txt
    """

    dest_image_name = readFile(file: 'dest_image_name.txt').trim()
    docker_image = docker.build "${dest_image_name}"
    return docker_image
}
