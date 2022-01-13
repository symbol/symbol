Object readYamlFromText(String data) {
	return readYaml(text: data)
}

Object readYamlFromFile(String fileName) {
	return readYaml(file: fileName)
}

void writeYamlToFile(Map text, String fileName, boolean overwriteFile=true) {
	writeYaml(file: fileName, data: text, overwrite: overwriteFile)
}
