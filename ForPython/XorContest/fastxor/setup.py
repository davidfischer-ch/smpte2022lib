from distutils.core import setup, Extension

fastxor = Extension('fastxor',
                    define_macros=[('MAJOR_VERSION', '1'),
                                   ('MINOR_VERSION', '0')],
                    include_dirs=['/usr/local/include'],
                    # libraries=[''],
                    library_dirs=['/usr/local/lib'],
                    extra_compile_args=['-fpermissive'],
                    sources=['fastxor.cpp'])

setup(name='python-fastxor',
      version='1.0',
      description='This is a C++ fast xor implemented by eryksun <http://stackoverflow.com/users/205580/eryksun>',
      author='David Fischer',
      author_email='david.fischer.ch@gmail.com',
      url='http://stackoverflow.com/questions/15459684/transmission-bytearray-from-python-to-c-and-return-it',
      ext_modules=[fastxor],
      long_description='''
This is really just a demo package.
''')
