from ply import lex  # pylint: disable=unused-import

tokens = [  # pylint: disable=invalid-name
	'NUMBER',
	'FLOAT_NUMBER',
	'TEMPLATE_NAME',
	'NAME',
	'OPEN_PAREN',
	'CLOSE_PAREN',
	'OPEN_BRACKET',
	'CLOSE_BRACKET',
	'OPEN_BRACE',
	'CLOSE_BRACE',
	'OPEN_SQUARE_BRACKET',
	'CLOSE_SQUARE_BRACKET',
	'COLON',
	'SEMI_COLON',
	'COMMA',
	'BACKSLASH',
	'PIPE',
	'PERCENT',
	'EXCLAMATION',
	'CARET',
	'COMMENT_SINGLELINE',
	'COMMENT_MULTILINE',
	'PRECOMP_MACRO',
	'ASTERISK',
	'AMPERSTAND',
	'EQUALS',
	'MINUS',
	'PLUS',
	'DIVIDE',
	'CHAR_LITERAL',
	'STRING_LITERAL',
	'NEWLINE',
	'SQUOTE'
]


t_ignore = ' \t\r.?@\f'  # pylint: disable=invalid-name
t_NUMBER = r'[0-9][0-9XxA-Fa-f]*'
t_FLOAT_NUMBER = r'[-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?'
t_NAME = r'[A-Za-z_~][A-Za-z0-9_]*'
t_OPEN_PAREN = r'\('
t_CLOSE_PAREN = r'\)'
t_OPEN_BRACKET = r'\<'
t_CLOSE_BRACKET = r'\>'
t_OPEN_BRACE = r'{'
t_CLOSE_BRACE = r'}'
t_OPEN_SQUARE_BRACKET = r'\['
t_CLOSE_SQUARE_BRACKET = r'\]'
t_SEMI_COLON = r';'
t_COLON = r':'
t_COMMA = r','
t_BACKSLASH = r'\\'
t_PIPE = r'\|'
t_PERCENT = r'%'
t_CARET = r'\^'
t_EXCLAMATION = r'!'
t_PRECOMP_MACRO = r'\#(.*?\\\n|.*)*'  # gimre: regex to handle multiline #defines


def t_COMMENT_SINGLELINE(tok):  # pylint: disable=invalid-name
	r'\/\/.*\n'
	tok.lexer.lineno += len([a for a in tok.value if a == '\n'])


t_ASTERISK = r'\*'
t_MINUS = r'\-'
t_PLUS = r'\+'
t_DIVIDE = r'/(?!/)'
t_AMPERSTAND = r'&'
t_EQUALS = r'='
t_CHAR_LITERAL = '\'.\''
t_SQUOTE = r'\''
t_STRING_LITERAL = r'"([^"\\]|\\.)*"'


def t_COMMENT_MULTILINE(tok):  # pylint: disable=invalid-name
	r'/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/'
	tok.lexer.lineno += len([a for a in tok.value if a == '\n'])


def t__n_e_w_l_i_n_e(tok):
	r'\n+'
	tok.lexer.lineno += len(tok.value)


def t_error(err):
	print(('Lex error: ', err))
