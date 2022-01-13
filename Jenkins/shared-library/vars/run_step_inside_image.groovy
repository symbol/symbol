
def call(String stepName, String imageName, String image_registry, String image_registry_credential, String args, Closure body) {
    logger.log_info("Running docker step ${stepName}")
    if (image_registry == null){
        run_inside_public_image(stepName, imageName, args, body)
    } else  {
        run_inside_private_image(stepName, imageName, image_registry, image_registry_credential, args, body)
    }
}

def run_inside_private_image(String stepName, String imageName, String image_registry, String image_registry_credential, String args, Closure body) {
    logger.log_info("Login into docker ${image_registry} with ${image_registry_credential}")
    docker.withRegistry(image_registry, image_registry_credential){
        run_inside_public_image(stepName, imageName, args, body)
    }
}

def run_inside_public_image(String stepName, String imageName, String args, Closure body) {
    logger.log_info("Running docker step ${stepName} in ${imageName}")
    docker.image(imageName).inside(args) {
        body()
    }
}