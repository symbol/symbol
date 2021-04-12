import sha3

NAMESPACE_FLAG = 1 << 63


def generate_mosaic_id(owner_address, nonce):
    """Generates a mosaic id from an owner address and a nonce."""
    hasher = sha3.sha3_256()
    hasher.update(nonce.to_bytes(4, 'little'))
    hasher.update(owner_address.bytes)
    digest = hasher.digest()

    result = int.from_bytes(digest[0:8], 'little')
    if result & NAMESPACE_FLAG:
        result -= NAMESPACE_FLAG

    return result


def generate_namespace_id(name, parent_namespace_id=0):
    """Generates a namespace id from a name and an optional parent namespace id."""
    hasher = sha3.sha3_256()
    hasher.update(parent_namespace_id.to_bytes(8, 'little'))
    hasher.update(name.encode('utf8'))
    digest = hasher.digest()

    result = int.from_bytes(digest[0:8], 'little')
    return result | NAMESPACE_FLAG


def generate_mosaic_alias_id(fully_qualified_name):
    """Generates a mosaic id from a fully qualified mosaic alias name."""
    return generate_namespace_path(fully_qualified_name)[-1]


def is_valid_namespace_name(name):
    """Returns true if a name is a valid namespace name."""
    def is_alphanum(character):
        return 'a' <= character <= 'z' or '0' <= character <= '9'

    return name and is_alphanum(name[0]) and all(is_alphanum(ch) or ch in ['_', '-'] for ch in name)


def generate_namespace_path(fully_qualified_name):
    """Parses a fully qualified namespace name into a path."""
    path = []
    parent_namespace_id = 0
    for name in fully_qualified_name.split('.'):
        if not is_valid_namespace_name(name):
            raise ValueError('fully qualified name is invalid due to invalid part name ({})'.format(fully_qualified_name))

        path.append(generate_namespace_id(name, parent_namespace_id))
        parent_namespace_id = path[-1]

    return path
