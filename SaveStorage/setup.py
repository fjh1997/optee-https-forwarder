from setuptools import setup, Extension

module = Extension(
        'Tsecurestorage',
        sources = ['host/main.c'],
        libraries=['teec'],
        include_dirs=['ta/include','host/include','/root/usr/include'],
        library_dirs=['/root/usr/lib']
        )

setup(name = 'Tsecurestorage',
          version = '1.0',
          description = 'A Python module to interact with OPTEE Secure Storage',
          ext_modules = [module])
