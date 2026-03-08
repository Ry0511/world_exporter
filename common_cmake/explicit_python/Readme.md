# Explicitly Versioned Python
This folder sets up a `explicit_python` target, which lets you link against an explicitly specified
(Windows) Python version. `FindPython` is generally not suitable for what we do, since it picks an
arbitrary version on your system, which may not even be the right architecture, and it picks a host
version if cross compiling. This target also adds install rules for the `.dll`s, `.pyd`s, and
`.zip`s which are needed at runtime when running an embedded interpreter.

To use this, you must define the variables `EXPLICIT_PYTHON_VERSION` and `EXPLICIT_PYTHON_ARCH`, as
used by the [python ftp](https://www.python.org/ftp/python) (i.e. `win32`/`amd64`).

It's also highly recommended to set the variables for the `URL_HASH` for each file it downloads:
- `EXPLICIT_PYTHON_HASH_BASE`, from `<version>/python-<version>-<arch>.zip`
- `EXPLICIT_PYTHON_HASH_TEST`, from `<version>/python-<version>-test-<arch>.zip`
- `EXPLICIT_PYTHON_HASH_EMBEDDABLE`, from `<version>/python-<version>-embeddable-<arch>.zip`

