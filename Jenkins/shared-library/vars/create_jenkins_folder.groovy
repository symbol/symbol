// Create a folder in Jenkins.
void call(String folder, String display_name) {
    jobDsl scriptText: '''
            folder(folder) {
                displayName(display_name)
                }
        ''', additionalParameters: [
            folder       : folder,
            display_name : display_name
        ]
}