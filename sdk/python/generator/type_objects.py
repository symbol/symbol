class BaseObject:
	def __init__(self, ast_model):
		self.ast_model = ast_model


class StructObject(BaseObject):
	def __init__(self, ast_model):
		super().__init__(ast_model)
		self.layout = []
		self.name_to_index = {}

	def add_field(self, field):
		self.name_to_index[field.ast_model.name] = len(self.layout)
		self.layout.append(field)

	def get_field_by_name(self, field_name):
		index = self.name_to_index[field_name]
		return self.layout[index]
