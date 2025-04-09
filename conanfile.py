from conan import ConanFile
from conan.tools.cmake import cmake_layout


class TacviewAnalyser(ConanFile):
    name = "TacviewAnalyser"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("boost/1.83.0", 
            options={
                "header_only": True,
                "without_test": True,
                "without_program_options": True,
                "without_graph": True,
                "without_serialization": True,
                "without_wave": True,
                "without_log": True
            }
        )
        self.requires("argparse/3.2")
        self.requires("drogon/1.9.10")
        self.requires("indicators/2.3")
        self.requires("libpqxx/7.10.0")
        self.requires("minizip/1.3.1")
        self.requires("nlohmann_json/3.11.3")
        self.requires("yaml-cpp/0.8.0")


    def layout(self):
        cmake_layout(self)
