from distutils.core import setup, Extension

setup(
    name = 'rpi_hcsr04',
    version = '0.1.0',
    description = 'Control module hc-sr04 which connected to raspberry pi GPIO.',
    author = 'aozk',
    author_email = 'rm.hyphen.rf.space.slash@gmail.com',
    url = 'https://github.com/aozk/rpi_hcsr04',
    ext_modules = [Extension('rpi_hcsr04', ['hcsr04/hcsr04.c'])],
    classifiers = [
        'Development Status :: 4 - Beta',
        'Environment :: Console',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: POSIX',
        'Operating System :: POSIX :: Linux',
        'Topic :: Software Development',
        'Topic :: System :: Hardware',
        'Programming Language :: Python :: 3',
        'Programming Language :: C',
        'Natural Language :: Japanese',
    ],
)
