import os

from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy


class CatapultConan(ConanFile):
	settings = "os", "compiler", "build_type", "arch"  # pylint: disable=invalid-name

	def requirements(self):
		self.requires("boost/1.83.0", run=True)
		self.requires("openssl/3.2.1", run=True)
		self.requires("cppzmq/4.10.0@nemtech/stable", run=True)
		self.requires("mongo-cxx-driver/3.9.0@nemtech/stable", run=True)
		self.requires("rocksdb/8.9.1@nemtech/stable", run=True)

	def build_requirements(self):
		# pylint: disable=not-callable
		self.test_requires("gtest/1.14.0")
		self.test_requires("benchmark/1.8.3@nemtech/stable")

	def layout(self):
		cmake_layout(self)

	def configure(self):
		# release dependencies - boost
		self.options["boost*"].shared = True
		self.options["boost*"].bzip2 = False
		self.options["boost*"].zlib = False

		self.options["boost*"].without_atomic = False
		self.options["boost*"].without_chrono = False
		self.options["boost*"].without_container = False
		self.options["boost*"].without_context = True
		self.options["boost*"].without_contract = True
		self.options["boost*"].without_coroutine = True
		self.options["boost*"].without_date_time = False
		self.options["boost*"].without_exception = False
		self.options["boost*"].without_fiber = True
		self.options["boost*"].without_filesystem = False
		self.options["boost*"].without_graph = True
		self.options["boost*"].without_graph_parallel = True
		self.options["boost*"].without_iostreams = True
		self.options["boost*"].without_json = True
		self.options["boost*"].without_locale = False
		self.options["boost*"].without_log = False
		self.options["boost*"].without_math = False
		self.options["boost*"].without_mpi = True
		self.options["boost*"].without_nowide = True
		self.options["boost*"].without_program_options = False
		self.options["boost*"].without_python = True
		self.options["boost*"].without_random = False
		self.options["boost*"].without_regex = False
		self.options["boost*"].without_serialization = True
		self.options["boost*"].without_stacktrace = True
		self.options["boost*"].without_system = False
		self.options["boost*"].without_test = True
		self.options["boost*"].without_thread = False
		self.options["boost*"].without_timer = True
		self.options["boost*"].without_type_erasure = True
		self.options["boost*"].without_wave = True

		# release dependencies - openssl
		self.options["openssl*"].shared = True
		self.options["openssl*"].no_zlib = True
		self.options["openssl*"].no_legacy = False
		self.options["openssl*"].no_module = True

		# release dependencies - other
		self.options["mongo-cxx-driver*"].shared = True
		self.options["rocksdb*"].shared = "Windows" != self.settings.os  # pylint: disable=no-member
		self.options["zeromq*"].shared = True

		# test dependencies
		self.options["benchmark*"].shared = False
		self.options["gtest*"].shared = False
		self.options["gtest*"].build_gmock = False
		self.options["gtest*"].no_main = True

	def generate(self):
		dependency_path = os.path.join(self.build_folder, "deps")
		for require, dep in self.dependencies.items():
			if dep.cpp_info.libdirs and require.libs and require.run:
				self.output.info(f"Copying shared libraries into deps directory: {dep}")
				if "Windows" == self.settings.os:  # pylint: disable=no-member
					# on windows, copy shared libraries into deps directory
					copy(self, "*.dll", dep.cpp_info.bindirs[0], dependency_path)
					copy(self, "*.dll", dep.cpp_info.libdirs[0], dependency_path)
				elif "Linux" == self.settings.os:  # pylint: disable=no-member
					# on linux,  copy shared libraries into deps directory
					copy(self, "*.so*", dep.cpp_info.libdirs[0], dependency_path)
				elif "Macos" == self.settings.os:  # pylint: disable=no-member
					# on macos, copy shared libraries into deps directory
					copy(self, "*.dylib", dep.cpp_info.libdirs[0], dependency_path)

		toolchain = CMakeToolchain(self)
		if "Macos" == self.settings.os:  # pylint: disable=no-member
			toolchain.blocks["rpath"].skip_rpath = False

		toolchain.generate()
		deps = CMakeDeps(self)
		deps.generate()
