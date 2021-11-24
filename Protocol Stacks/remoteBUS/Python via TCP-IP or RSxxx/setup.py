from distutils.core import setup
setup(
  name = 'emBRICK',         # How you named your package folder (MyLib)
  packages = ['emBRICK'],   # Chose the same as "name"
  version = '0.07',      # Start with a small number and increase it with every change you make
  license='MIT',        # Chose a license from here: https://help.github.com/articles/licensing-a-repository
  description = 'Driver to build a connection RemoteMaster <-> LWCS over Ethernet, Modbus TCP & RTU',   # Give a short description about your library
  author = 'IMACS',                   # Type in your name
  author_email = 'serkan.sen@imacs-gmbh.de',      # Type in your E-Mail
  url = 'https://github.com/IMACS-GmbH/emBRICK/tree/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx%20(using%20Windows-PC%20or%20RaspberryPi)',   # Provide either the link to your github or to your website
  download_url = 'https://github.com/IMACS-GmbH/emBRICK/blob/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx%20(using%20Windows-PC%20or%20RaspberryPi)/emBRICK-0.06.tar.gz',    # I explain this later on
  keywords = ['IMACS', 'EMBRICK', 'PYTHON','TCP/IP','Modbus', 'RTU','TCP'],   # Keywords that define your package best
  install_requires=[            
          'threaded',
          'pyserial',
          'pymodbus',
          'sockets',
          'tabulate',
          'DateTime',
          'numpy'
      ],
  classifiers=[
    'Development Status :: 4 - Beta',      # Chose either "3 - Alpha", "4 - Beta" or "5 - Production/Stable" as the current state of your package
    'Intended Audience :: Developers',      # Define that your audience are developers
    'Topic :: Software Development :: Build Tools',
    'License :: OSI Approved :: MIT License',   # Again, pick a license
    'Programming Language :: Python :: 3',      #Specify which pyhton versions that you want to support
    'Programming Language :: Python :: 3.4',
    'Programming Language :: Python :: 3.5',
    'Programming Language :: Python :: 3.6',
  ],
)
