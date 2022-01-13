def read_yaml_from_text(String data) {
    return readYaml(text: data)
}

def read_yaml_from_file(String file_name) {
    return readYaml(file: file_name)
}

def write_yaml_to_file(Map text, String file_name, boolean overwrite_file=true) {
    writeYaml(file: file_name, data: text, overwrite: overwrite_file)
}

