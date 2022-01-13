void call(String branch, String credentialsId, String url) {
    cleanWs()
    git branch: branch, credentialsId: credentialsId, poll: false, url: url
}