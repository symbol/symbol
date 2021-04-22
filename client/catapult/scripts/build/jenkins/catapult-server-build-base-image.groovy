pipeline {
    agent {
        label 'cat-server-01'
    }

    stages {
        stage('setup parameters') {
            steps {
                script {
                    properties([
                        parameters([
                            choice(name: 'COMPILER', choices: ['gcc-10', 'clang-11'], description: 'compiler'),
                            text(name: 'SANITIZERS', defaultValue: '', description: 'comma delimited sanitizers (clang only)'),
                            choice(name: 'ARCH', choices: ['skylake', 'broadwell', 'westmere'], description: 'architecture')
                        ])
                    ])
                }
            }
        }

        stage('prepare base image') {
            steps {
                script {
                    dest_image_name = "symbolplatform/symbol-server-build-base:${COMPILER}"
                    if (params.SANITIZERS)
                        dest_image_name += '-' + params.SANITIZERS.replaceAll(',', '-')
                    dest_image_name += "-${ARCH}"

                    base_image_build_layer(0, "${dest_image_name}-preimage1")
                    base_image_build_layer(1, "${dest_image_name}-preimage2")
                    base_image = base_image_build_layer(2, "${dest_image_name}")
                    base_image.push()
                }
            }
        }
    }
}

def base_image_build_layer(layer, dest_image_name) {
    sh """
        python3 ./scripts/build/baseImageDockerfileGenerator.py \
            --layer=${layer} \
            --compiler="${params.COMPILER}" \
            --sanitizers="${params.SANITIZERS ?: ''}" \
            --architecture="${params.ARCH}" \
            --versions="./scripts/build/versions.properties" > Dockerfile

        echo "*** LAYER ${layer} ***"
        cat Dockerfile
    """

    script {
        docker_image = docker.build "${dest_image_name}"
        return docker_image
    }
}
