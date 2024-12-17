from setuptools import setup, Extension

module = Extension(
        'TEErandom',
        sources = ['host/main.c'],
        libraries=['teec'],
        include_dirs=['ta/include','host/include','/root/usr/include'],
        library_dirs=['/root/usr/lib']
        )

setup(name = 'TEErandom',
          version = '1.0',
          description = 'This is a demo package',
          ext_modules = [module])
