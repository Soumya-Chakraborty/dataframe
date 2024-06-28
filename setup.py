from setuptools import setup, Extension

# Define the extension module
dataframes_module = Extension('dataframes',
                              sources=['dataframes.c'])

setup(
    name='dataframes',
    version='1.0.0',
    description='A basic DataFrame implementation in C with Python bindings',
    author='Soumya Chakraborty',
    author_email='soumya@example.com',
    url='https://github.com/yourusername/dataframes',
    ext_modules=[dataframes_module],
    py_modules=['dataframes'],
    classifiers=[
        'Programming Language :: Python :: 3',
        'License :: OSI Approved :: GNU General Public License v3 (GPLv3)',
        'Operating System :: OS Independent',
    ],
)