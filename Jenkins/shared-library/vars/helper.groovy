Boolean is_manual_build(String manual_branch) {
    return null != manual_branch && '' != manual_branch
}

String resolve_branch_name(String manual_branch) {
    return is_manual_build(manual_branch) ? manual_branch : env.GIT_BRANCH
}